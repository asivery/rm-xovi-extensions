#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include "types.h"
#include "indexfile.h"
#include "qmldiff.h"
#include "../../util.h"
#include "../xovi.h"
#include "systemversion.h"

/*
    ModificationDefinition are raw structures that come from the modification files
    They are basically a condition of whether or not a modification will be applied.
    During the first traversal of the structure, these objects will be copied into
    REPLACEMENT_ENTRIES or INJECTION_ENTRIES tables.
    If any of these fields contains a value, the structure will be traverse again,
    to stat() it. Then in the third and final traversal, the modifications will be applied.
*/
static struct ReplacementEntry *REPLACEMENT_ENTRIES = NULL;
static struct InjectionEntry *INJECTION_ENTRIES = NULL;

struct ModificationDefinition *DEFINITIONS = NULL;

struct ResourceRoot {
    uint8_t *tree;
    uint8_t *name;
    uint8_t *data;

    uint32_t treeSize;
    uint32_t nameSize;
    uint32_t dataSize;
    uint32_t originalDataSize;

    uint32_t entriesAffected;
};

struct TempFileReference {
    struct ResourceRoot root;
    int fd;
    void *rootAddress;
    int size;
};

uint32_t readUInt32(uint8_t *addr, int offset){
    return (addr[offset + 0] << 24) |
           (addr[offset + 1] << 16) |
           (addr[offset + 2] << 8) |
           (addr[offset + 3] << 0);
}

void writeUint32(uint8_t *addr, int offset, uint32_t value){
    addr[offset + 0] = (value >> 24);
    addr[offset + 1] = (value >> 16);
    addr[offset + 2] = (value >> 8);
    addr[offset + 3] = (value);
}

void writeUint16(uint8_t *addr, int offset, uint16_t value){
    addr[offset + 0] = (value >> 8);
    addr[offset + 1] = (value);
}

uint16_t readUInt16(uint8_t *addr, int offset){
    return (addr[offset + 0] << 8) |
           (addr[offset + 1] << 0);
}

static inline int findOffset(int node) {
    return node * TREE_ENTRY_SIZE;
}

void nameOfChild(struct ResourceRoot *root, int node, int *size, char *buffer, int max) {
    if (!node){
        if(size) *size = 0;
        *buffer = 0;
        return;
    }
    const int offset = findOffset(node);

    uint32_t name_offset = readUInt32(root->tree, offset);
    uint16_t name_length = readUInt16(root->name, name_offset);
    if(name_length > max - 1) name_length = max - 1;
    name_offset += 2;
    name_offset += 4; // jump past hash

    if(size) *size = name_length;
    for(int i = 1; i<name_length * 2; i += 2) {
        buffer[i / 2] = ((char *)root->name)[name_offset + i];
    }
    buffer[name_length] = 0;
}

void testWriteToFile(const char *fname, struct ResourceRoot *root) {
    LOG("Writing to file %s\n", fname);
    int actualDataOffset = 4 + 16 + 4;
    int fd = open(fname, O_RDWR | O_CREAT);
    write(fd, "qres", 4);
    write(fd, "\x00\x00\x00\x03", 4);
    uint8_t itable[4];
    writeUint32(itable, 0, actualDataOffset + root->dataSize + root->nameSize); // Offset to tree
    write(fd, itable, 4);
    writeUint32(itable, 0, actualDataOffset); // Offset to tree
    write(fd, itable, 4);
    writeUint32(itable, 0, actualDataOffset + root->dataSize); // Offset to tree
    write(fd, itable, 4);
    write(fd, "\x00\x00\x00\x05", 4); // actual flags
    write(fd, root->data, root->dataSize);
    write(fd, root->name, root->nameSize);
    write(fd, root->tree, root->treeSize);
    close(fd);
}

void statArchive(struct ResourceRoot *root, int node) {
    int offset = findOffset(node);
    int thisMaxLength = offset + TREE_ENTRY_SIZE;
    if(thisMaxLength > root->treeSize) root->treeSize = thisMaxLength;
    uint32_t nameOffset = readUInt32(root->tree, offset);
    uint32_t thisMaxNameLength = nameOffset + readUInt16(root->name, nameOffset);
    if(thisMaxNameLength > root->nameSize) root->nameSize = thisMaxNameLength;
    int flags = readUInt16(root->tree, offset + 4);
    if(!(flags & DIRECTORY)) {
        // If it's not a file, stat the payloads too
        uint32_t dataOffset = readUInt32(root->tree, offset + 4 + 2 + 4);
        uint32_t dataSize = readUInt32(root->data, dataOffset);
        uint32_t thisMaxDataLength = dataOffset + dataSize + 4;
        if(thisMaxDataLength > root->dataSize) root->dataSize = thisMaxDataLength;
    } else {
        // Recurse.
        uint32_t childCount = readUInt32(root->tree, offset + 4 + 2);
        offset += 4 + 4 + 2;
        uint32_t childOffset = readUInt32(root->tree, offset);
        for(int child = childOffset; child < childOffset + childCount; child++){
            statArchive(root, child);
        }
    }
    root->originalDataSize = root->dataSize;
}


char qmldiffApplyChanges(const char *filename, void **data, uint32_t *size, uint16_t *flags, bool *freeAfterwards) {
    char *temporary;
    uint32_t dataSize;
    if (*flags == 4) {
        // ZSTD.
        size_t decompressedSize = $ZSTD_getFrameContentSize(*data, *size);
        temporary = malloc(decompressedSize + 1);
        $ZSTD_decompress(temporary, decompressedSize, *data, *size);
        temporary[decompressedSize] = 0;
        dataSize = decompressedSize;
    } else if(*flags == 0) {
        temporary = malloc(*size + 1);
        memcpy(temporary, *data, *size);
        temporary[*size] = 0;
        dataSize = *size;
    } else {
        fprintf(stderr, "Cannot understand the format of %s - bailing.\n", filename);
        return false;
    }

    char *processed = qmldiff_process_file(filename, temporary, dataSize);
    free(temporary);
    if(processed == NULL) return false;

    *data = processed;
    *size = strlen(processed);
    *freeAfterwards = true;
    *flags = 0;

    return true;
}

void processNode(struct ResourceRoot *root, int node, const char *rootName) {
    int offset = findOffset(node) + 4; // Skip name
    uint16_t flags = readUInt16(root->tree, offset);
    offset += 2;
    int stringLength;
    char nameBuffer[256];
    nameOfChild(root, node, &stringLength, nameBuffer, 256);
    if(flags & DIRECTORY) {
        uint32_t childCount = readUInt32(root->tree, offset);
        offset += 4;
        uint32_t childOffset = readUInt32(root->tree, offset);
        // Form the new root
        int rootLength = strlen(rootName);
        char *tempRoot = malloc(rootLength + stringLength + 2); // /, and \0
        memcpy(tempRoot, rootName, rootLength);
        memcpy(tempRoot + rootLength, nameBuffer, stringLength);
        tempRoot[rootLength + stringLength] = '/';
        tempRoot[rootLength + stringLength + 1] = 0;
        hash_t fileHash = hashString(tempRoot);
        struct ModificationDefinition *definitionToApply = NULL;
        HASH_FIND_HT(DEFINITIONS, &fileHash, definitionToApply);
        if(definitionToApply) {
            LOG("[%s]: NOT YET IMPLEMENTED\n", NAME);
        }
        for(int child = childOffset; child < childOffset + childCount; child++){
            processNode(root, child, tempRoot);
        }
        free(tempRoot);
    } else {
        hash_t fileHash = hashStringS(nameBuffer, hashString((char*) rootName)); // Equal to hash on concat
        LOG("[%s]: Processing node %d: %s%s [%llu]\n", NAME, node, rootName, nameBuffer, fileHash);
        struct ModificationDefinition *definitionToApply = NULL;
        HASH_FIND_HT(DEFINITIONS, &fileHash, definitionToApply);
        if(definitionToApply) {
            // Found! Update the replacement table
            // Create a new entry
            if(definitionToApply->modificationType == MODIF_REPLACE) {
                struct ReplacementEntry *newEntry = malloc(sizeof(struct ReplacementEntry));
                newEntry->node = node;
                newEntry->data = definitionToApply->action.toReplace->data;
                newEntry->size = definitionToApply->action.toReplace->size;
                newEntry->freeAfterwards = false;
                newEntry->copyToOffset = root->dataSize;
                writeUint16(root->tree, offset - 2, 0);
                writeUint32(root->tree, offset + 4, newEntry->copyToOffset);

                root->entriesAffected++;
                root->dataSize += newEntry->size + 4;
                HASH_ADD_INT(REPLACEMENT_ENTRIES, node, newEntry);
                LOG("[%s]: Marked for replacement - %s%s\n", NAME, rootName, nameBuffer);
            } else {
                LOG("[%s]: Invalid mode - should be set to REPLACE\n", NAME);
            }
        } else {
            // Check if any of the overrides has anything to say about this file.
            int rootLen = strlen(rootName);
            int nameLen = strlen(nameBuffer);
            char *tempNameBuffer = malloc(rootLen + nameLen + 1);
            memcpy(tempNameBuffer, rootName, rootLen);
            memcpy(tempNameBuffer + rootLen, nameBuffer, nameLen);
            tempNameBuffer[rootLen + nameLen] = 0;

            if(qmldiff_is_modified(tempNameBuffer)) {
                uint16_t flags = readUInt16(root->tree, offset - 2);
                uint32_t offsetOfData = readUInt32(root->tree, offset + 4);
                uint32_t dataSize = readUInt32(root->data, offsetOfData);
                void *dataAddress = &root->data[offsetOfData + 4];
                bool freeAfterwards = false;

                if(qmldiffApplyChanges(tempNameBuffer, &dataAddress, &dataSize, &flags, &freeAfterwards)) {
                    // The value's getting remapped. Create an update record and swap the values
                    struct ReplacementEntry *newEntry = malloc(sizeof(struct ReplacementEntry));
                    newEntry->node = node;
                    newEntry->data = dataAddress;
                    newEntry->size = dataSize;
                    newEntry->copyToOffset = root->dataSize;
                    newEntry->freeAfterwards = freeAfterwards;
                    writeUint16(root->tree, offset - 2, flags);
                    writeUint32(root->tree, offset + 4, newEntry->copyToOffset);

                    root->entriesAffected++;
                    root->dataSize += newEntry->size + 4;
                    HASH_ADD_INT(REPLACEMENT_ENTRIES, node, newEntry);
                    LOG("[%s]: Marked for replacement by external override - %s%s\n", NAME, rootName, nameBuffer);
                }
            }

            free(tempNameBuffer);
        }
    }
}

void applyDataTableChanges(struct ResourceRoot *root){
        // If so, reallocate the data table.
    char *oldData = root->data;
    root->data = malloc(root->dataSize);
    memcpy(root->data, oldData, root->originalDataSize);
    struct ReplacementEntry *s, *next;

    for (s = REPLACEMENT_ENTRIES; s != NULL; s = next) {
        uint8_t itable[4];
        writeUint32(itable, 0, s->size);
        memcpy(&root->data[s->copyToOffset], itable, 4);
        memcpy(&root->data[s->copyToOffset + 4], s->data, s->size);
        if(s->freeAfterwards) {
            free(s->data);
        }
        next = s->hh.next;
        free(s);
    }
    REPLACEMENT_ENTRIES = NULL;

}
struct TempFileReference testGetFromFile(const char *filename){
    int fd = open(filename, O_RDONLY);
    struct stat stat_temp;
    fstat(fd, &stat_temp);
    char *addr = mmap(NULL, stat_temp.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
    LOG("MMAP'd to address %p (FD %d)\n", addr, fd);
    uint32_t treeOffset = readUInt32(addr, 8),
             dataOffset = readUInt32(addr, 12),
             nameOffset = readUInt32(addr, 16);
    struct TempFileReference root = {
        .root = {
            .tree = &addr[treeOffset],
            .data = &addr[dataOffset],
            .name = &addr[nameOffset],
            .treeSize = 0,
            .dataSize = 0,
            .originalDataSize = 0,
            .nameSize = 0,
            .entriesAffected = 0,
        },
        .fd = fd,
        .rootAddress = addr,
        .size = stat_temp.st_size,
    };
    return root;
}

int override$_Z21qRegisterResourceDataiPKhS0_S0_(int version, uint8_t *tree, uint8_t *name, uint8_t *data) {
    pthread_mutex_lock(&mainMutex);
    LOG("[%s]: Asked to add %d %p %p %p to QT resource root.\n", NAME, version, tree, name, data);
    struct ResourceRoot resource = {
        .data = data,
        .name = name,
        .tree = tree,

        .treeSize = 0,
        .dataSize = 0,
        .originalDataSize = 0,
        .nameSize = 0,

        .entriesAffected = 0,
    };

    statArchive(&resource, 0);
    resource.tree = malloc(resource.treeSize);
    memcpy(resource.tree, tree, resource.treeSize);

    processNode(&resource, 0, "");
    LOG("[%s]: Processing done!\n", NAME);

    // Did we alter anything?
    if(resource.entriesAffected) {
        LOG("[%s]: Rebuilding data tables...\n", NAME);
        // Yes - good. Rebuild the data table, and use the affected variables instead.
        applyDataTableChanges(&resource);
        tree = resource.tree;
        name = resource.name;
        data = resource.data;
    } else {
        // No. Free the used memory
        free(resource.tree);
    }

    // Invoke the original code
    LOG("[%s]: Invoking with %d %p %p %p.\n", NAME, version, tree, name, data);
    int status = $_Z21qRegisterResourceDataiPKhS0_S0_(version, tree, name, data);
    pthread_mutex_unlock(&mainMutex);
    return status;
}

void fatalErrorAbortEverything(const char *error) {
    fprintf(stderr, "[qt-resource-rebuilder]: Something terrible has happened. Please verify your setup. Error: %s\n", error);
    fflush(stderr);
    abort();
}

void _xovi_construct(){
    const char *env = getenv("QML_DISABLE_DISK_CACHE");
    if(env == NULL) {
        fatalErrorAbortEverything("QML_DISABLE_DISK_CACHE is not set correctly");
    }
    if(strcmp("1", env) != 0) {
        fatalErrorAbortEverything("QML_DISABLE_DISK_CACHE is not set correctly");
    }

    setupSystemVersion();

    pthread_mutex_init(&mainMutex, NULL);
    loadAllModifications(&DEFINITIONS);
    char *temp = Environment->getExtensionDirectory(NAME);
    qmldiff_build_change_files(temp);
    free(temp);
    // The function itself will decide if the thread needs to be started
    qmldiff_load_rules(r$hashtabRules);
    qmldiff_start_saving_thread();
}

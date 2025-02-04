#include "indexfile.h"
#include "types.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include "../../util.h"
#include "rccload.h"
#include "../xovi.h"

#define MAX_LINE_LENGTH     1024
#define MAX_FILENAME_LENGTH 1024

#define CONCAT_STRINGS_WITH_DELIM(result, str1, str2, delim) do {           \
    size_t len1 = strlen(str1);                                             \
    size_t len2 = strlen(str2);                                             \
    size_t delim_len = 1;                                                   \
    if (len1 + len2 + delim_len >= (MAX_FILENAME_LENGTH)) {                 \
        LOG("[%s]: Error: Concatenated string exceeds max length\n", NAME); \
        result[0] = '\0';                                                   \
    } else {                                                                \
        snprintf(result, MAX_FILENAME_LENGTH, "%s%c%s", str1, delim, str2); \
    }                                                                       \
} while (0)

static inline int endsWith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix);
}

static inline void defineReplacement(struct ModificationDefinition **definitions, char *qmlPath, char *_fileName, const char *extensionRootDir) {
    hash_t hash = hashString(qmlPath);

    struct ModificationDefinition *existing = NULL;
    HASH_FIND_HT(*definitions, &hash, existing);
    if(existing) {
        LOG("[%s]: Error: %s has already been overwritten! Cannot overlay again!\n", NAME, qmlPath);
        return;
    }

    char fileName[MAX_FILENAME_LENGTH];
    if(_fileName[0] != '/') {
        // Relative
        CONCAT_STRINGS_WITH_DELIM(fileName, extensionRootDir, _fileName, '/');
    } else {
        strncpy(fileName, _fileName, MAX_FILENAME_LENGTH);
    }

    struct stat statStruct;
    if(stat(fileName, &statStruct) == -1){
        LOG("[%s]: Cannot stat file %s\n", NAME, fileName);
        return;
    }
    size_t size = statStruct.st_size;
    if(size > UINT32_MAX) {
        // I doubt anyone will ever see this message.
        LOG("[%s]: File %s is too large. Will be truncated.\n", NAME, fileName);
        size = UINT32_MAX;
    }
    LOG("[%s]: Loading file %s (%lld bytes)\n", NAME, fileName, size);

    char *fileContents = malloc(size);
    if(!fileContents) {
        LOG("[%s]: Failed to malloc()!\n", NAME);
        return;
    }

    FILE *file = $fopen(fileName, "r");
    if (file == NULL) {
        LOG("[%s]: Error opening file %s\n", NAME, fileName);
        return;
    }

    fread(fileContents, size, 1, file);
    fclose(file);

    struct ModificationDefinition *mod = malloc(sizeof(struct ModificationDefinition));
    struct ReplacementEntry *repl = malloc(sizeof(struct ReplacementEntry));
    mod->modificationType = MODIF_REPLACE;
    mod->entryName = strdup(qmlPath);
    mod->entryNameHash = hash;
    mod->action.toReplace = repl;
    repl->data = fileContents;
    repl->node = 0;
    repl->size = size;
    HASH_ADD_HT(*definitions, entryNameHash, mod);
    LOG("[%s]: Defined a replacement for %s [%llu] (%s) successfully!\n", NAME, qmlPath, hash, fileName);
}

static void loadAllFromFile(char *fileName, struct ModificationDefinition **definitions, const char *extensionRootDir){
    FILE *file = $fopen(fileName, "r");
    if (file == NULL) {
        LOG("[%s]: Error opening file %s\n", NAME, fileName);
        return;
    }
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if(strlen(line) == 0) continue;

        char type;
        char qmlPath[MAX_LINE_LENGTH];
        char realPath[MAX_LINE_LENGTH];

        if (sscanf(line, "%c %s %s", &type, qmlPath, realPath) == 3) {
            switch(type){
                case 'R':
                    // Replacement
                    defineReplacement(definitions, qmlPath, realPath, extensionRootDir);
                    break;
                default:
                    LOG("[%s]: Unimplemented action: %c! - skipping line %s\n", NAME, type, line);
                    break;
            }
        } else {
            LOG("[%s]: Line format invalid: %s\n", NAME, line);
        }
    }

    fclose(file);
}

void loadAllModifications(struct ModificationDefinition **definitions) {
    char *extensionRootDir = Environment->getExtensionDirectory(NAME);
    DIR *extHome = opendir(extensionRootDir);
    struct dirent *entry;
    int n = 0;
    char fileName[MAX_FILENAME_LENGTH];
    LOG("[%s]: Locating resources to load...\n", NAME);
    if(!extHome) {
        LOG("[%s]: Extension home directory (%s) not found. Bailing.\n", NAME, extensionRootDir);
        return;
    }
    while((entry = readdir(extHome)) != NULL) {
        if(entry->d_type == DT_REG && endsWith(entry->d_name, ".qrr") == 0){
            CONCAT_STRINGS_WITH_DELIM(fileName, extensionRootDir, entry->d_name, '/');
            LOG("[%s]: Reading %s\n", NAME, fileName);
            loadAllFromFile(fileName, definitions, extensionRootDir);
            ++n;
        }
        if(entry->d_type == DT_REG && endsWith(entry->d_name, ".rcc") == 0){
            CONCAT_STRINGS_WITH_DELIM(fileName, extensionRootDir, entry->d_name, '/');
            registerRCCFile(fileName);
        }
    }
    closedir(extHome);
    free(extensionRootDir);
    LOG("[%s]: Loaded %d resource rebuild files.\n", NAME, n);
}

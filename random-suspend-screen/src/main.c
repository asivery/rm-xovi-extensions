#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../util.h"
#include "../../fileman/src/fileman.h"
#include "../xovi.h"

#define NAME "random-suspend-screen"


static const char *ENV_ROOT;
static char *filenameBuffer;
static int rootNameLength;

const char *nameRemap(const char *_name, const char *_mode, int _1, int _2){
    int fileCount = 0, currentIndex = 0, fileIndex = 0;
    struct dirent *entry;
    DIR *suspendRoot;

    suspendRoot = opendir(ENV_ROOT);
    LOG("[%s]: Counting loadable files...\n", NAME);
    while((entry = readdir(suspendRoot)) != NULL) {
        if(entry->d_type == DT_REG){
            fileCount++;
        }
    }
    closedir(suspendRoot);

    fileIndex = rand() % fileCount;
    if(fileCount == 0) {
        LOG("[%s]: Found no files, opening original file.\n", NAME);
        return _name;
    }
    LOG("[%s]: Found %d files, opening file #%d.\n", NAME, fileCount, fileIndex);

    suspendRoot = opendir(ENV_ROOT);
    while((entry = readdir(suspendRoot)) != NULL) {
        if(entry->d_type == DT_REG){
            if(currentIndex == fileIndex){
                strncpy(filenameBuffer + rootNameLength, entry->d_name, 255);
                break;
            }
            currentIndex++;
        }
    }
    closedir(suspendRoot);
    printf("[%s]: Selected file %s.\n", NAME, filenameBuffer);


    return filenameBuffer;
}

void _xovi_construct(){
    srand(time(NULL));
    ENV_ROOT = Environment->getExtensionDirectory(NAME);
    // If this directory does not exist, do not load anything.
    DIR *suspendRoot = opendir(ENV_ROOT);
    if(suspendRoot == NULL){
        LOG("[%s]: No %s enviroment directory. Bailing.\n", NAME, NAME);
        return;
    }
    closedir(suspendRoot);

    rootNameLength = strlen(ENV_ROOT);
    filenameBuffer = malloc(rootNameLength + 255);
    memcpy(filenameBuffer, ENV_ROOT, rootNameLength);

    LOG("[%s]: Loaded.\n", NAME);

    struct FilemanOverride *foverride = malloc(sizeof(struct FilemanOverride));
    foverride->handlerType = FILEMAN_HANDLE_NAME_MAP;
    foverride->handler.fileNameRemap = nameRemap;
    foverride->name = "/usr/share/remarkable/suspended.png";
    foverride->nameMatchType = FILEMAN_MATCH_WHOLE;
    fileman$registerOverride(foverride);
}

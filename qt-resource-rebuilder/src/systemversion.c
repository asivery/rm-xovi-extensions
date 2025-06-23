#include <stdio.h>
#include <string.h>
#include "qmldiff.h"
#include "systemversion.h"

void setupSystemVersion(){
    char name[256], value[256], line[256];
    memset(name, 0, sizeof(name));
    memset(value, 0, sizeof(name));
    FILE *file = fopen("/etc/os-release", "r");
    if(file == NULL) {
        fprintf(stderr, "[qt-resource-rebuilder] No version file found!!\n");
        return;
    }
    while(fgets(line, sizeof(line), file)) {
        const char *a = strstr(line, "=");
        int nameL = a - line;
        a += 1; // Skip the '='
        if(nameL >= 256) continue;
        memcpy(name, line, nameL);
        name[nameL] = 0;
        int valL = strlen(a);
        if(valL >= 256) continue;
        memcpy(value, a, valL);

        char *realValue = value;
        while(realValue[0] == '"' || realValue[0] == ' ') {
            realValue++;
            valL--;
        }
        while(realValue[valL - 1] == '\n' || realValue[valL - 1] == '"') {
            realValue[valL - 1] = 0;
            valL--;
        };
        if(strcmp(name, "IMG_VERSION") == 0) {
            qmldiff_set_version(realValue);
        }
    }
    fclose(file);
}

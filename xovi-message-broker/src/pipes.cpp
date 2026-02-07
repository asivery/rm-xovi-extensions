#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "pipes.h"
#include "messagebroker.h"

static inline void recreatePipe(const char *name) {
    if(access(name, F_OK) == 0) {
        unlink(name);
    }
    mkfifo(name, 0660);
}

static inline int openPipe(const char *name, bool write) {
    int fd = open(name, write ? O_WRONLY : O_RDONLY);
    if(fd == -1) {
        printf("[xovi-message-broker]: Failed to open pipe %s for %s\n", name, write ? "writing" : "reading");
    }
    return fd;
}

// Command format:
// [o][c][signalName]:[value]\n
// o: If output is requested, >
// c: u / e - UI / Extension
static void executeCommand(char *data) {
    bool hasOutput = false, toExtension;
    char *signal, *value;
    char *output, *locationOfSeparator;

    char initial = *data++;
    if(initial == '>') {
        hasOutput = true;
        initial = *data++;
    }

    switch(initial) {
        case 'u':
            toExtension = false;
            break;
        case 'e':
            toExtension = true;
            break;
        default:
            goto parseError;
    }
    locationOfSeparator = strchr(data, ':');
    if(locationOfSeparator == nullptr) goto parseError;
    *locationOfSeparator = 0; // Split into two strings;
    signal = data;
    value = locationOfSeparator + 1;

    int nativeHitCount;
    if(toExtension) {
        output = broadcastToNative(signal, value, &nativeHitCount);
        printf("[xovi-message-broker]: Signal %s : %s to extensions\n", signal, value);
    } else {
        // To QML:
        output = broadcast(signal, value);
        printf("[xovi-message-broker]: Signal %s : %s to qml\n", signal, value);
        nativeHitCount = 1;
    }

    if(hasOutput && nativeHitCount == 1) {
        int outputTextLength = 0;
        if(output != NULL) outputTextLength = strlen(output);
        int outPipeWriteFD = openPipe(OUT_PIPE, true);
        if(outPipeWriteFD != -1) {
            if(outputTextLength > 0) write(outPipeWriteFD, output, outputTextLength);
            close(outPipeWriteFD);
        }
    }

    if(output != NULL) {
        free(output);
    }

    return;
parseError:
    printf("[xovi-message-broker]: Cannot parse command `%s'\n", data);
    return;
}

static void *pipeThread(void *) {
    printf("[xovi-message-broker]: Starting pipe thread!\n");
    char *commandBuffer = new char[MAX_COMMAND_SIZE];
    int cursor = 0;
    mainLoop: for(;;) {
        printf("[xovi-message-broker]: Waiting for new commands...\n");
        int inPipeReadFD = openPipe(IN_CMD_PIPE, false);
        if(inPipeReadFD == -1) {
            continue;
        }
        cursor = 0;
        memset(commandBuffer, 0, MAX_COMMAND_SIZE);
        for(;;) {
            int readBytes = read(inPipeReadFD, commandBuffer + cursor, MAX_COMMAND_SIZE);
            if(readBytes < 1) {
                goto mainLoop;
            }
            for(int subcursor = 0; subcursor < readBytes; subcursor++) {
                if(commandBuffer[cursor + subcursor] == '\n') {
                    // Send command.
                    commandBuffer[cursor + subcursor] = 0;
                    executeCommand(commandBuffer);
                    goto mainLoop;
                }
            }
            cursor += readBytes;
        }
    }
    return nullptr;
}

void initializePipes() {
    recreatePipe(IN_CMD_PIPE);
    recreatePipe(OUT_PIPE);
    pthread_t thread;
    pthread_create(&thread, NULL, pipeThread, NULL);
    pthread_detach(thread);
}

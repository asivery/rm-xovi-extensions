#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <linux/input.h>
#include <time.h>
#include "files.h"
#include "../../util.h"

static FILE *penFile = NULL;

extern volatile time_t lastPenUpdateTime;
extern volatile bool running;
static pthread_t thread_id;

static void *_thread(void *_){
    LOG("[PalmRejection -> Pen]: Started pen monitoring thread\n");
    struct input_event readEvent;
    while(running) {
        fread(&readEvent, sizeof(readEvent), 1, penFile);
        if(readEvent.code && readEvent.type) {
            // Valid packet.
            lastPenUpdateTime = time(NULL);
        }
    }
}

bool startHalManager() {
    penFile = $fopen(PEN_FILE, "r");
    if(!penFile) {
        LOG("[PalmRejection -> Pen]: Failed to open " PEN_FILE " (Pen)\n");
        return false;
    }
    pthread_create(&thread_id, NULL, _thread, NULL);
    return true;
}

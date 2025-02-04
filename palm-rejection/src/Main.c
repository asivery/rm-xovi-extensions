#include <stdbool.h>

#include "../../fileman/src/fileman.h"
#include "../../util.h"
#include "../xovi.h"


volatile bool touchScreenDisabled = false;
// TODO:
volatile bool running = true;

extern bool startTouchPipe();
extern bool startHalManager();

void _xovi_construct(){
    Environment->requireExtension("fileman", 0, 1, 0);
    if(!startHalManager()) {
        LOG("[PalmRejection]: Bailing!\n");
        return;
    }
    startTouchPipe();
}

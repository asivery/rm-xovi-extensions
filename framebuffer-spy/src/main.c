#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "../xovi.h"
#include "../framebuffer-spy.h"

static struct FramebufferConfig config;
static const char *framebufferConfigString = "NULL";

static int sizeMap;

static void prepareConfigString() {
    framebufferConfigString = malloc(1024);
    snprintf((char *) framebufferConfigString, 1024,
            "%p,%d,%d,%d,%d,%d",
            config.framebufferAddress,
            config.width, config.height,
            config.type, config.bpl,
            config.requiresReload);
    fprintf(stderr, "Found framebuffer! Config string is %s\n", framebufferConfigString);
}

#ifdef __aarch64__
void override$_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, long long int bpl, int f, void *a, void *b){
#elif __arm__
void override$_ZN6QImageC1EPhiiiNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, int bpl, int f, void *a, void *b){
#endif
    fprintf(stderr, "Invoked image constructor for address %p, w = %d, h = %d, bpl = %d, f = %d\n", that, x, y, bpl, f);
    bool rmppmCondition = x == 960 && y == 1696 && bpl == 3840 && f == 4;
    bool rmppCondition = x == 1620 && y == 2160 && bpl == 6528 && f == 4;
    bool rm2Condition = x == 1404 && y == 1872 && ((bpl == 2808 && f == 7) || (bpl == 5616 && f == 4));
    if((rmppmCondition || rmppCondition || rm2Condition) && config.framebufferAddress == NULL) {
        config.width = x;
        config.height = y;
        config.type = f == 4 ? FBSPY_TYPE_RGBA : FBSPY_TYPE_RGB565;
        config.bpl = bpl;
        config.requiresReload = false;
        config.framebufferAddress = data;
        prepareConfigString();
    }
#ifdef __aarch64__
    $_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(that, data, x, y, bpl, f, a, b);
#elif __arm__
    $_ZN6QImageC1EPhiiiNS_6FormatEPFvPvES2_(that, data, x, y, bpl, f, a, b);
#endif
}

// export
struct FramebufferConfig getFramebufferConfig() {
    return config;
}

// export
char *getConfigString() {
    return strdup(framebufferConfigString);
}

// export
void refreshFramebuffer() {
    if(config.requiresReload) {
        msync(config.framebufferAddress, sizeMap, MS_SYNC);
    }
}

static void prepareRM1() {
    int fd = open("/dev/fb0", O_RDONLY);
    if(fd != -1) {
        sizeMap = 1872 * 1408 * 2;
        fprintf(stderr, "Framebuffer detected as a device file (fd=%d, size=%u) - mmaping...\n", fd, sizeMap);
        void *data = mmap(NULL, sizeMap, PROT_READ, MAP_SHARED, fd, 0);
        config.width = 1408;
        config.height = 1872;
        config.type = FBSPY_TYPE_RGB565;
        config.bpl = config.width * 2;
        config.requiresReload = true;
        config.framebufferAddress = data;
        prepareConfigString();
    }
}

void _xovi_construct() {
    // Check if this is an rM1:
    char *data = malloc(1024);
    config.framebufferAddress = NULL;
    int devFD = open("/sys/devices/soc0/machine", O_RDONLY);
    if(devFD > -1) {
        int r = read(devFD, data, 1023);
        if(r > 0) {
            data[r] = 0;
            if(strstr(data, "reMarkable 1.0") != NULL || strstr(data, "reMarkable Prototype 1") != NULL) {
                prepareRM1();
            }
        }
        close(devFD);
    }
    free(data);
}

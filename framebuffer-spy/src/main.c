#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../xovi.h"

static void *framebufferData = NULL;

void setFramebufferAddress(void *data) {
    fprintf(stderr, "Found framebuffer! Address is %p\n", data);
    framebufferData = data;
    char temp[50];
    snprintf(temp, 50, "%p", framebufferData);
    setenv("FRAMEBUFFER_SPY_EXTENSION_FBADDR", temp, 1);
}

#ifdef __aarch64__
void override$_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, long long int bpl, int f, void *a, void *b){
#elif __arm__
void override$_ZN6QImageC1EPhiiiNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, int bpl, int f, void *a, void *b){
#endif
    fprintf(stderr, "Invoked image constructor for address %p, w = %d, h = %d, bpl = %d, f = %d\n", that, x, y, bpl, f);
    bool rmppmCondition = x == 960 && y == 1696 && bpl == 3840 && f == 4;
    bool rmppCondition = x == 1620 && y == 2160 && bpl == 6528 && f == 4;
    bool rm2Condition = x == 1404 && y == 1872 && bpl == 2808 && f == 7;
    if((rmppmCondition || rmppCondition || rm2Condition) && framebufferData == NULL) {
        setFramebufferAddress(data);
    }
#ifdef __aarch64__
    $_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(that, data, x, y, bpl, f, a, b);
#elif __arm__
    $_ZN6QImageC1EPhiiiNS_6FormatEPFvPvES2_(that, data, x, y, bpl, f, a, b);
#endif
}

void *getFramebufferAddress(){
    return framebufferData;
}

void _xovi_construct() {
    // Check if this is an rM1:
    int fd = open("/dev/fb0", O_RDONLY);
    if(fd != -1) {
        int sizeMap = 1872 * 1408 * 2;
        fprintf(stderr, "Framebuffer detected as a device file (fd=%d, size=%u) - mmaping...\n", fd, sizeMap);
        void *data = mmap(NULL, sizeMap, PROT_READ, MAP_SHARED, fd, 0);
        setFramebufferAddress(data);
        setenv("FRAMEBUFFER_SPY_REQUIRE_MSYNC", "1", 1);
    }
}

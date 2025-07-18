#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../xovi.h"

static void *framebufferData = NULL;

#ifdef __aarch64__
void override$_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, long long int bpl, int f, void *a, void *b){
#elif __arm__
void override$_ZN6QImageC1EPhiiiNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, int bpl, int f, void *a, void *b){
#endif
    fprintf(stderr, "Invoked image constructor for address %p, w = %d, h = %d, bpl = %d, f = %d\n", that, x, y, bpl, f);
    bool rmppCondition = x == 1620 && y == 2160 && bpl == 6528 && f == 4;
    bool rm2Condition = x == 1404 && y == 1872 && bpl == 2808 && f == 7;
    if(rmppCondition || rm2Condition) {
        fprintf(stderr, "Found framebuffer! Address is %p\n", data);
        framebufferData = data;
        char temp[50];
        snprintf(temp, 50, "%p", framebufferData);
        setenv("FRAMEBUFFER_SPY_EXTENSION_FBADDR", temp, 1);
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

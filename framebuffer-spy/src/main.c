#include <stdio.h>
#include <stdlib.h>
#include "../xovi.h"

static void *framebufferData = NULL;

void override$_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(void *that, void *data, int x, int y, long long int bpl, int f, void *a, void *b){
    fprintf(stderr, "Invoked image constructor for address %p, w = %d, h = %d, bpl = %d, f = %d\n", that, x, y, bpl, f);
    if(x == 1620 && y == 2160 && bpl == 6496 && f == 4) {
        fprintf(stderr, "Found framebuffer! Address is %p\n", data);
        framebufferData = data;
        char temp[50];
        snprintf(temp, 50, "%p", framebufferData);
        setenv("FRAMEBUFFER_SPY_EXTENSION_FBADDR", temp, 1);
    }
    $_ZN6QImageC1EPhiixNS_6FormatEPFvPvES2_(that, data, x, y, bpl, f, a, b);
}

void *getFramebufferAddress(){
    return framebufferData;
}

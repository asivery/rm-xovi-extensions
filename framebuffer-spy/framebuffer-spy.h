#pragma once

#include <stdbool.h>
#define FBSPY_TYPE_RGB565 1
#define FBSPY_TYPE_RGBA 2

struct FramebufferConfig {
    int width, height, type, bpl;
    bool requiresReload;
};

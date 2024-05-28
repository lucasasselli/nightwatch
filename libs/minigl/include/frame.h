#pragma once

#include <stdint.h>

#ifndef SCREEN_SIZE_X
#define SCREEN_SIZE_X 240
#endif

#ifndef SCREEN_SIZE_Y
#define SCREEN_SIZE_Y 240
#endif

typedef struct {
    uint8_t c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
    float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
} minigl_frame_t;

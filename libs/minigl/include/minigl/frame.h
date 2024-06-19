#pragma once

#include <stdint.h>

#ifndef SCREEN_SIZE_X
#define SCREEN_SIZE_X 240
#endif

#ifndef SCREEN_SIZE_Y
#define SCREEN_SIZE_Y 240
#endif

#ifdef __arm__
#include <arm_fp16.h>
typedef __fp16 fp16_t;
#else
typedef float fp16_t;
#endif

typedef struct {
    fp16_t depth;
    uint8_t color;
} minigl_pixel_t;

typedef struct {
    minigl_pixel_t** data;
    int size_x;
    int size_y;
} minigl_frame_t;

minigl_frame_t* minigl_frame_new(int width, int height);

int minigl_frame_to_file(minigl_frame_t* frame, char* path);

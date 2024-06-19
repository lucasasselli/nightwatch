#pragma once

#include <spng.h>

typedef struct {
    uint8_t color;
    uint8_t alpha;
} minigl_pixel_ga_t;

// FIXME: Make opacity optional
typedef struct {
    minigl_pixel_ga_t** data;
    uint32_t size_x;
    uint32_t size_y;
} minigl_tex_t;

int minigl_tex_read_file(const char* path, minigl_tex_t* tex);

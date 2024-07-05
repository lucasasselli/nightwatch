#pragma once

#include <spng.h>
#include <stdbool.h>

typedef enum {
    MINIGL_COLOR_FMT_G8,
    MINIGL_COLOR_FMT_GA8
} minigl_color_fmt_t;

typedef struct {
    uint8_t color;
    uint8_t alpha;
} minigl_pixel_ga8_t;

typedef struct {
    uint8_t color;
} minigl_pixel_g8_t;

typedef struct {
    minigl_color_fmt_t format;
    union {
        minigl_pixel_ga8_t** data_ga8;
        minigl_pixel_g8_t** data_g8;
    };
    uint32_t size_x;
    uint32_t size_y;
} minigl_tex_t;

// TODO: Move flip y here?
typedef struct {
    bool force_g8;
    uint8_t alpha_color;
} minigl_tex_read_opts_t;

#define MINIGL_TEX_READ_OPTS_NONE ((minigl_tex_read_opts_t){0})

int minigl_tex_read_file(const char* path, minigl_tex_t* tex, minigl_tex_read_opts_t);

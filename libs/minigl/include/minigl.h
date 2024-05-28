#pragma once

#include "frame.h"
#include "minigl_types.h"
#include "object.h"
#include "texture.h"

#define MINIGL_INLINE static inline __attribute((always_inline))

// #define MINIGL_SCANLINE
// #define MINIGL_PERSP_CORRECT
#define MINIGL_NO_DITHERING

#define MINIGL_Z_FADE_THRESHOLD 0.90f

typedef enum {
    PERF_CLIP,
    PERF_CULL,
    PERF_POLY,
    PERF_FRAG
} minigl_perf_event_t;

typedef union {
    int array[4];
    struct {
        int clip;
        int cull;
        int poly;
        int frag;
    };
} minigl_perf_data_t;

// State objects
typedef struct {
    minigl_tex_t texture;
    minigl_tex_mode_t texture_mode;
    minigl_tex_t dither;
    minigl_face_cull_t face_cull;
    uint8_t draw_color;
} minigl_cfg_t;

void minigl_set_tex(minigl_tex_t t);

void minigl_set_dither(minigl_tex_t t);

void minigl_set_color(uint8_t color);

minigl_perf_data_t minigl_perf_get(void);

void minigl_perf_clear(void);

void minigl_clear(uint8_t color, int depth);

void minigl_draw(minigl_obj_buf_t obj);

minigl_frame_t* minigl_get_frame(void);

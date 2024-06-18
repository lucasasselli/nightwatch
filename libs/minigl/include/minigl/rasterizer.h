#pragma once

#include "minigl/frame.h"
#include "minigl/object.h"
#include "minigl/texture.h"

#define MINIGL_SCANLINE
// #define MINIGL_PERSP_CORRECT
#define MINIGL_NO_DITHERING

// TODO: Implement
typedef enum {
    MINIGL_NEVER,
    MINIGL_LESS,
    MINIGL_GREATER,
    MINIGL_EQUAL
} minigl_depth_funct_t;

// TODO: Implement
typedef enum {
    MINIGL_FCULL_NONE,
    MINIGL_FCULL_BACK,
    MINIGL_FCULL_FRONT
} minigl_face_cull_t;

typedef enum {
    MINIGL_SHADE_SOLID,    // Solid color
    MINIGL_SHADE_TEX_2D,   // 2D texture
    MINIGL_SHADE_MATERIAL  // Material
} minigl_shade_mode_t;

// State objects
typedef struct {
    minigl_tex_t dither;
    minigl_shade_mode_t texture_mode;
    minigl_tex_t texture;
    uint8_t color;
    minigl_matgroup_t* matgroup;
} minigl_cfg_t;

void minigl_set_tex(minigl_tex_t t);

void minigl_set_dither(minigl_tex_t t);

void minigl_set_color(uint8_t color);

void minigl_set_matgroup(minigl_matgroup_t* matgroup);

void minigl_clear(uint8_t color, int depth);

void minigl_draw(minigl_objbuf_t obj);

minigl_frame_t* minigl_get_frame(void);

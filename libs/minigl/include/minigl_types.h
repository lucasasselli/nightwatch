#pragma once

#include <cglm/cglm.h> /* for inline */

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

// TODO: Implement
typedef enum {
    MINIGL_NEVER,
    MINIGL_LESS,
    MINIGL_GREATER,
    MINIGL_EQUAL
} minigl_depth_funct_t;

typedef enum {
    MINIGL_TEX_0D,  // Solid color
    MINIGL_TEX_2D   // 2D texture
} minigl_tex_mode_t;

typedef enum {
    MINIGL_DITHER_OFF,
    MINIGL_DITHER_ON
} minigl_dither_mode_t;

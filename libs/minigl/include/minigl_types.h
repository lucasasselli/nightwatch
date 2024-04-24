#pragma once

#include <cglm/cglm.h> /* for inline */
#include <stdint.h>

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

// ---------------------------------------------------------------------------
// RENDERING
// ---------------------------------------------------------------------------

typedef struct {
    vec4* vcoord_ptr;
    vec2* tcoord_ptr;
    ivec3* vface_ptr;
    ivec3* tface_ptr;
    int vcoord_size;
    int tcoord_size;
    int face_size;
} minigl_obj_t;

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
    MINIGL_FCULL_NONE,
    MINIGL_FCULL_BACK,
    MINIGL_FCULL_FRONT
} minigl_face_cull_t;

typedef enum {
    MINIGL_TEX_0D,  // Solid color
    MINIGL_TEX_2D   // 2D texture
} minigl_tex_mode_t;

// ---------------------------------------------------------------------------
// OTHER
// ---------------------------------------------------------------------------

typedef struct {
    vec3 pos;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
} minigl_camera_t;

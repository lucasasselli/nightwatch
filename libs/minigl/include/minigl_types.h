#pragma once

// ---------------------------------------------------------------------------
// MATH
// ---------------------------------------------------------------------------

typedef union {
    struct {
        int x;
        int y;
        int z;
    } coord;
    int array[3];
} int3_t;

typedef union {
    struct {
        float x;
        float y;
    } coord;
    float array[2];
} vec2_t;

typedef union {
    struct {
        float x;
        float y;
        float z;
    } coord;
    float array[3];
} vec3_t;

typedef union {
    struct {
        float x;
        float y;
        float z;
        float w;
    } coord;
    float array[4];
} vec4_t;

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
    MINIGL_TEXTURE_0D,  // Solid color
    MINIGL_TEXTURE_1D,  // 1D Texture
    MINIGL_TEXTURE_2D   // 2D Texture
} minigl_texture_mode_t;

// ---------------------------------------------------------------------------
// RENDERING
// ---------------------------------------------------------------------------

typedef struct {
    vec4_t* vcoord_ptr;
    vec2_t* tcoord_ptr;
    int3_t* vface_ptr;
    int3_t* tface_ptr;
    int vcoord_size;
    int tcoord_size;
    int face_size;
} minigl_obj_t;

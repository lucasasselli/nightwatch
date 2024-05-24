#pragma once

#include <cglm/cglm.h> /* for inline */

#include "pd_api.h"

#define OBJ_LINE_BUFFER_SIZE 50

typedef struct {
    vec4* vcoord_ptr;
    vec2* tcoord_ptr;
    ivec3* vface_ptr;
    ivec3* tface_ptr;
    int vcoord_size;
    int tcoord_size;
    int face_size;
} minigl_obj_t;

// NOTE: This is identical to minigl_obj_t, but used for
typedef struct {
    vec4* vcoord_ptr;
    vec2* tcoord_ptr;
    ivec3* vface_ptr;
    ivec3* tface_ptr;
    int vcoord_size;
    int tcoord_size;
    int face_size;
} minigl_obj_buf_t;

extern PlaydateAPI* pd;

int minigl_obj_read_file(char* path, minigl_obj_t* out);

void minigl_obj_copy(minigl_obj_t in, minigl_obj_t* out);

void minigl_obj_copy_trans(minigl_obj_t in, mat4 trans, minigl_obj_t* out);

void minigl_obj_trans(minigl_obj_t* in, mat4 trans);

minigl_obj_buf_t minigl_obj_buf_init(size_t size);

void minigl_obj_buf_free(minigl_obj_buf_t buf);

void minigl_obj_to_obj_buf_trans(minigl_obj_t in, mat4 trans, minigl_obj_buf_t* out);

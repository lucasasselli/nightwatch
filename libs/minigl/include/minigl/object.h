#pragma once

#include <cglm/cglm.h>

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

// NOTE: This is identical to minigl_obj_t, but used to buffer geometry
// Potentially can be used interchangeably.
typedef struct {
    vec4* vcoord_ptr;
    vec2* tcoord_ptr;
    ivec3* vface_ptr;
    ivec3* tface_ptr;
    int vcoord_size;
    int tcoord_size;
    int face_size;
} minigl_objbuf_t;

int minigl_obj_read_file(char* path, minigl_obj_t* out);

void minigl_obj_copy(minigl_obj_t in, minigl_obj_t* out);

void minigl_obj_copy_trans(minigl_obj_t in, mat4 trans, minigl_obj_t* out);

void minigl_obj_trans(minigl_obj_t* in, mat4 trans);

minigl_objbuf_t minigl_objbuf_init(size_t size);

void minigl_objbuf_free(minigl_objbuf_t buf);

void minigl_obj_to_objbuf_trans(minigl_obj_t in, mat4 trans, minigl_objbuf_t* out);

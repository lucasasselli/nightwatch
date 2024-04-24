#pragma once

#include <spng.h>

#include "pd_api.h"

#define OBJ_LINE_BUFFER_SIZE 50

typedef struct {
    uint8_t** ptr;
    uint32_t size_x;
    uint32_t size_y;
} minigl_tex_t;

extern PlaydateAPI* pd;

int minigl_tex_read_file(char* path, minigl_tex_t* tex);

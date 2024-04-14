#pragma once

#include <stdio.h>

#include "minigl_types.h"
#include "pd_api.h"

#define OBJ_FILE_BUFFER_SIZE 1000

extern PlaydateAPI* pd;

typedef struct {
    vec4_t* vptr;
    int3_t* fptr;
    int vptr_size;
    int fptr_size;
} obj_data_t;

obj_data_t obj_file_read(char* path);

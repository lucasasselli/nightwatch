#pragma once

#include <stdio.h>

#include "minigl_types.h"
#include "pd_api.h"

#define OBJ_FILE_BUFFER_SIZE 1000

extern PlaydateAPI* pd;

minigl_obj_t obj_file_read(char* path);

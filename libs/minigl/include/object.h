#pragma once

#include "minigl_types.h"
#include "pd_api.h"

#define OBJ_LINE_BUFFER_SIZE 50

extern PlaydateAPI* pd;

minigl_obj_t obj_file_read(char* path);

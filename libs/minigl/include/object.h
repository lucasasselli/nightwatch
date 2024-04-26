#pragma once

#include "minigl_types.h"
#include "pd_api.h"

#define OBJ_LINE_BUFFER_SIZE 50

extern PlaydateAPI* pd;

int minigl_obj_read_file(char* path, minigl_obj_t* out);

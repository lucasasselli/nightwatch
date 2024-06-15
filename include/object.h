#pragma once

#include "minigl/object.h"

typedef enum {
    OBJ_FLOOR,
    OBJ_WALL,
    OBJ_ENEMY,
    OBJ_STATUE,
    OBJ_COLUMN,
    OBJ_BASE,
    OBJ_WETFLOOR,
    OBJ_NOTE
} obj_id_t;

#define OBJ_NUM 9

int obj_init(void);

minigl_obj_t* obj_get(obj_id_t id);

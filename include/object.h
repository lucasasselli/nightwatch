#pragma once

#include "minigl/object.h"

typedef enum {
    OBJ_FLOOR,
    OBJ_WALL,
    OBJ_WALL_GRAFFITI,
    OBJ_ENEMY,
    OBJ_STATUE,
    OBJ_COLUMN,
    OBJ_COLUMN_GRAFFITI,
    OBJ_BASE,
    OBJ_WETFLOOR,
    OBJ_NOTE,
    OBJ_BENCH,
    OBJ_WC_PANEL,
    OBJ_WC_COLUMN,
    OBJ_WC_SINK,
    OBJ_WC_SIGN,
} obj_id_t;

#define OBJ_NUM 15

int obj_init(void);

minigl_obj_t* obj_get(obj_id_t id);

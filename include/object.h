#pragma once

#include "minigl/object.h"

typedef enum {
    OBJ_ID_FLOOR,
    OBJ_ID_WALL,
    OBJ_ID_WALL_GRAFFITI,
    OBJ_ID_SIGN_SQUARE,
    OBJ_ID_SIGN_HUGE,
    OBJ_ID_SIGN_SMALL_SIDE,
    OBJ_ID_ENEMY,
    OBJ_ID_STATUE,
    OBJ_ID_COLUMN,
    OBJ_ID_COLUMN_GRAFFITI,
    OBJ_ID_SHUTTER_CLOSED,
    OBJ_ID_SHUTTER_OPEN,
    OBJ_ID_BASE,
    OBJ_ID_WETFLOOR,
    OBJ_ID_NOTE,
    OBJ_ID_BENCH,
    OBJ_ID_WC_PANEL,
    OBJ_ID_WC_COLUMN,
    OBJ_ID_WC_SINK
} obj_id_t;

#define OBJ_ID_NUM 19

int obj_init(void);

minigl_obj_t* obj_get(obj_id_t id);

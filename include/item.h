#pragma once

#include <stdbool.h>

#include "object.h"
#include "texture.h"

typedef enum {
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST
} dir_t;

typedef enum {
    ITEM_NORMAL,
    ITEM_FLOOR,
    ITEM_WALL,
    ITEM_BILLBOARD,
} item_type_t;

typedef enum {
    TEX_MODE_NONE,
    TEX_MODE_IMAGE,
    TEX_MODE_MDBB
} tex_mode_t;

typedef enum {
    ACTION_NONE,
    ACTION_NOTE,
    ACTION_KEYPAD
} action_type_t;

typedef struct {
    action_type_t type;
    int arg;
} action_t;

enum {
    EFFECT_NONE = 0,
    EFFECT_SPIN = 1
};

typedef struct item_t item_t;

struct item_t {
    bool hidden;

    item_type_t type;
    obj_id_t obj_id;
    dir_t dir;

    tex_mode_t tex_mode;
    union {
        tex_id_t tex_id;
        tex_mdbb_id_t tex_mdbb_id;
    };
    uint8_t color;

    int effects;
    action_t action;

    item_t* next;  // Next item (for lists)
};

item_t* item_new(void);

item_t* item_new_tex(obj_id_t obj_id, tex_id_t tex_id, dir_t dir);

item_t* item_new_color(obj_id_t obj_id, uint8_t color, dir_t dir);

item_t* item_new_mdbb(tex_mdbb_id_t tex_id, dir_t dir);

void item_set_color(item_t* item, uint8_t color);

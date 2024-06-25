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
    DRAW_MODE_COLOR,
    DRAW_MODE_TEX,
    DRAW_MODE_MATERIAL,
    DRAW_MODE_MDBB
} draw_mode_t;

typedef enum {
    ACTION_NONE,
    ACTION_NOTE,
    ACTION_KEYPAD,
    ACTION_INSPECT
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
    bool collide;

    item_type_t type;
    obj_id_t obj_id;
    dir_t dir;

    draw_mode_t draw_mode;
    union {
        tex_id_t tex_id;
        tex_mdbb_id_t tex_mdbb_id;
    };
    uint8_t color;
    minigl_matgroup_t* matgroup;

    int effects;
    action_t action;

    item_t* next;  // Next item (for lists)
};

item_t* item_new(void);

void item_list_free(item_t* item);

item_t* item_new_color(obj_id_t obj_id, uint8_t color, dir_t dir, bool collide);

item_t* item_new_tex(obj_id_t obj_id, tex_id_t tex_id, dir_t dir, bool collide);

item_t* item_new_mat(obj_id_t obj_id, minigl_matgroup_t* matgroup, dir_t dir, bool collide);

item_t* item_new_mdbb(tex_mdbb_id_t tex_id, dir_t dir, bool collide);

void item_set_color(item_t* item, uint8_t color);

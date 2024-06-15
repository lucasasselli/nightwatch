#pragma once

#include <cglm/cglm.h>

#include "camera.h"
#include "constants.h"

#define MAP_TILE_MAX_ITEMS 10

typedef enum {
    ITEM_FLOOR,
    ITEM_STATUE,
    ITEM_WALL,
    ITEM_COLUMN,
    ITEM_BASE,
    ITEM_WETFLOOR,
    ITEM_DOOR,
    ITEM_NOTE
} map_item_type_t;

typedef enum {
    DIR_ANY = -1,
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST
} map_item_dir_t;

typedef struct {
    map_item_type_t type;
    map_item_dir_t dir;
    bool hidden;
    bool action;
    int id;
    int arg;
} map_item_t;

typedef struct {
    map_item_t *items;
    int item_cnt;
    bool collide;
    bool visible;  // FIXME: Move elsewhere
} map_tile_t;

// clang-format off
#define MAP_ITEM_STATIC_INIT(x,y) (map_item_t){x, y, false, false, 0, 0}
#define MAP_ITEM_ACTION_INIT(x,y,z) (map_item_t){x, y, false, true, z, 0}
// clang-format on

typedef map_tile_t map_t[MAP_SIZE][MAP_SIZE];

map_tile_t map_get_tile_xy(map_t map, int x, int y);

map_tile_t map_get_tile_ivec2(map_t map, ivec2 pos);

map_tile_t map_get_tile_vec2(map_t map, vec2 pos);

bool map_tile_has_item(map_tile_t tile, map_item_type_t type, map_item_dir_t dir);

void map_viz_update(map_t map, camera_t camera);

bool map_tile_collide(map_tile_t tile);

bool map_viz_xy(map_t map, vec2 pos, int x, int y);

bool map_viz_ivec2(map_t map, vec2 pos, ivec2 tile);

void map_init(map_t map);

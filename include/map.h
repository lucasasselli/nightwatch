#pragma once

#define MAP_TILE_MAX_ITEMS 10

#include <cglm/cglm.h>

#include "constants.h"
#include "types.h"

typedef enum {
    ITEM_FLOOR,
    ITEM_STATUE,
    ITEM_WALL,
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
} map_item_t;

typedef struct {
    map_item_t *items;
    int item_cnt;
    bool collide;
    bool visible;
} map_tile_t;

typedef map_tile_t map_t[MAP_SIZE][MAP_SIZE];

map_tile_t map_get_tile_xy(map_t map, int x, int y);

map_tile_t map_get_tile_ivec2(map_t map, ivec2 pos);

map_tile_t map_get_tile_vec2(map_t map, vec2 pos);

void map_update_viz(map_t map, camera_t camera);

bool map_tile_collide(map_tile_t tile);

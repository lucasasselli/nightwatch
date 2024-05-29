#pragma once

#define MAP_SIZE 64
#define MAP_TILE_MAX_ITEMS 10

#define MAP_TILE_SIZE 4.0f

#include <cglm/cglm.h>

typedef enum {
    ITEM_FLOOR,
    ITEM_STATUE,
    ITEM_WALL,
} map_item_type_t;

typedef enum {
    DIR_ANY,
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
} map_tile_t;

typedef map_tile_t map_t[MAP_SIZE][MAP_SIZE];

void pos_tile_to_world(ivec2 tile, vec3 world);

void pos_world_to_tile(vec3 world, ivec2 tile);

map_tile_t map_get_tile(map_t map, ivec2 pos);

bool map_tile_collide(map_tile_t tile);

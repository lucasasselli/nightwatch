#pragma once

#include <cglm/cglm.h>

#include "camera.h"
#include "item.h"

#define MAP_SIZE 64
#define MAP_DRAW_SIZE 24

typedef struct {
    item_t* items;
    bool is_clone;
} map_tile_t;

typedef struct {
    map_tile_t** grid;
    bool** viz;
} map_t;

map_tile_t map_get_tile_xy(map_t* map, int x, int y);

map_tile_t map_get_tile_ivec2(map_t* map, ivec2 pos);

map_tile_t map_get_tile_vec2(map_t* map, vec2 pos);

item_t* tile_find_item(map_tile_t tile, int item_type, int dir, int action_type);

bool tile_has_item(map_tile_t tile, int item_type, int dir, int action_type);

bool tile_get_collide(map_tile_t tile);

bool map_is_empty_xy(map_t* map, int x, int y);

void map_item_add_xy(map_t* map, int x, int y, item_t* item);

item_t* map_has_interact_item_vec2(map_t* map, vec2 pos);

bool map_get_collide_xy(map_t* map, int x, int y);

bool map_get_collide_ivec2(map_t* map, ivec2 pos);

bool map_get_collide_vec2(map_t* map, vec2 pos);

void map_set_collide_xy(map_t* map, int x, int y, bool collide);

void map_viz_update(map_t* map, camera_t camera);

bool map_viz_get_xy(map_t* map, vec2 pos, int x, int y);

bool map_viz_get_ivec2(map_t* map, vec2 pos, ivec2 tile);

void map_tile_clone_xy(map_t* map, int dst_x, int dst_y, int src_x, int src_y);

map_t* map_new(void);

void map_free(map_t* map);

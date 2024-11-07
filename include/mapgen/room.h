#pragma once

#include "map.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int r;
} bounds_t;

typedef void (*furnish_fun_t)(map_t*, bounds_t);

typedef struct {
    int x;
    int y;
    int size;
    dir_t dir;
} door_t;

void room_add_item(map_t* map, bounds_t b, int pos_x, int pos_y, item_t* item);

map_tile_t room_get_tile(map_t* map, bounds_t b, int pos_x, int pos_y);

bounds_t room_norm_r(bounds_t room);

void room_add_floor(map_t* map, bounds_t b, int pos_x, int pos_y, int width, int height);

void add_picture(map_t* map, bounds_t b, int x, int y, dir_t dir);

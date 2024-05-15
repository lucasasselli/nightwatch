#pragma once

#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "utils.h"

#define MAP_SIZE 64
#define MAP_MAX_ROOMS 16
#define MAP_RAND_EFFORT 50

typedef enum {
    TILE_EMPTY,
    TILE_FLOOR,
    TILE_WALL_NS,
    TILE_WALL_EW,
    TILE_DOOR_NS,
    TILE_DOOR_EW,
    TILE_CORNER0,
    TILE_CORNER1,
    TILE_CORNER2,
    TILE_CORNER3
} map_tile_t;

typedef enum {
    ROOM_BASE,      // Simple room
    ROOM_CORRIDOR,  // Narrow room
} RoomType;

#define ROOM_TYPE_SIZE 2

typedef union {
    struct {
        int x, y;
    };
    int r;
} RoomSize;

typedef struct {
    RoomType type;
    int x;
    int y;
    RoomSize size;
    int exit_cnt;
    RoomSize way_in;
} map_room_t;

typedef struct {
    map_tile_t **grid;
    map_room_t *rooms;
    int room_cnt;
} map_t;

char mapgen_tile_char(map_tile_t tile);

void mapgen_init(map_t *map);

void mapgen_grid_print(map_t map);

void mapgen_gen(map_t *map);

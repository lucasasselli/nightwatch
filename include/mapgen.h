#pragma once

#include <stdbool.h>

#define MAP_SIZE 64
#define MAP_MAX_ROOMS 5
#define MAP_RAND_EFFORT 50
#define MAP_TILE_MAX_ITEMS 10

typedef enum {
    TILE_FLOOR,
    TILE_STATUE,
    TILE_WALL_N,
    TILE_WALL_E,
    TILE_WALL_S,
    TILE_WALL_W
} map_item_type_t;

typedef struct {
    map_item_type_t type;
} map_item_t;

typedef enum {
    ROOM_BASE,      // Simple room
    ROOM_CORRIDOR,  // Narrow room
} map_room_type_t;

#define ROOM_TYPE_SIZE 1

// FIXME: Replace with Ivec2
typedef union {
    struct {
        int x, y;
    };
    int r;
} RoomSize;

typedef struct {
    map_room_type_t type;
    int x;
    int y;
    RoomSize size;
    int exit_cnt;
    RoomSize way_in;
} map_room_t;

typedef struct {
    map_item_t *items;
    int item_cnt;
} map_tile_t;

typedef struct {
    map_tile_t **grid;
    map_room_t *rooms;
    int room_cnt;
} map_t;

void mapgen_init(map_t *map);

void mapgen_grid_print(map_t map);

void mapgen_gen(map_t *map);

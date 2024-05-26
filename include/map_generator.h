#pragma once

#include <stdbool.h>

#include "map.h"

#define MAP_MAX_ROOMS 5
#define MAP_RAND_EFFORT 50

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

void mapgen_grid_print(map_t map);

void mapgen_gen(map_t map);

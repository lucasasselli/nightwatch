#pragma once

#include <stdbool.h>

#include "map.h"

#define MAP_MAX_ROOMS 32
#define MAP_RAND_EFFORT 50

typedef enum {
    ROOM_BASE,
    ROOM_CORRIDOR_NS,
    ROOM_CORRIDOR_EW
} map_room_type_t;

typedef struct {
    map_room_type_t type;
    ivec2 pos;
    ivec2 size;
} map_room_t;

void mapgen_gen(map_t map);

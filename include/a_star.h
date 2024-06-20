#pragma once

#include <cglm/cglm.h>
#include <stdbool.h>

#include "map.h"

typedef struct {
    ivec2 pos;
    void* parent;
    int h;
    int g;
    int c;
    void* next;
} a_star_node_t;

typedef struct {
    ivec2 pos[100];
    int size;
} a_star_path_t;

bool a_star_navigate(map_t* map, ivec2 start, ivec2 stop, a_star_path_t* path);

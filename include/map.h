#pragma once

#include <cglm/cglm.h>

#include "game.h"
#include "mapgen.h"

#define MAP_DRAW_SIZE 13
#define MAP_TILE_SIZE 4.0f

#define MINIMAP_TILE_SIZE 4
#define MINIMAP_SIZE_X 100
#define MINIMAP_SIZE_Y 100

void minimap_init(void);

void minimap_gen(map_t map);

void minimap_draw(int x, int y, camera_t camera);

void map_init(void);

void map_draw(map_t map, mat4 trans, camera_t camera);

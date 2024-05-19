#pragma once

#include <cglm/cglm.h>

#include "game.h"
#include "mapgen.h"

#define MAP_DRAW_SIZE MAP_SIZE
#define MAP_TILE_SIZE 4.0f

#define MINIMAP_TILE_SIZE 4
#define MINIMAP_SIZE_X 100
#define MINIMAP_SIZE_Y 100

void map_init(void);

void map_gen_grid(void);

void map_gen_poly(void);

void map_draw(mat4 trans, camera_t camera);

void minimap_draw(int x, int y, camera_t camera);

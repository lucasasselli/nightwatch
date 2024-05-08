#pragma once

#include <cglm/cglm.h>

#include "minigl.h"
#include "object.h"
#include "texture.h"

#define MAP_GRID_SIZE 32
#define MAP_GRID_DRAW_SIZE 32
#define MAP_TILE_SIZE 4.0f

void map_init(void);

void map_generate(void);

void map_draw(mat4 trans);

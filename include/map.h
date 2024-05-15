#pragma once

#include <cglm/cglm.h>

#include "mapgen.h"
#include "minigl.h"
#include "object.h"
#include "texture.h"
#include "utils.h"

#define MAP_DRAW_SIZE MAP_SIZE
#define MAP_TILE_SIZE 4.0f

void map_init(void);

void map_gen_grid(void);

void map_gen_poly(void);

void map_draw(mat4 trans);

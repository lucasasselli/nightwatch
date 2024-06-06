#pragma once

#include "game.h"
#include "map.h"

#define MINIMAP_TILE_SIZE 4
#define MINIMAP_SIZE_X 100
#define MINIMAP_SIZE_Y 100

void minimap_init(void);

void minimap_gen(map_t map);

void minimap_draw(int x, int y, game_state_t* state);

void minimap_debug_draw(int x, int y, game_state_t* state);

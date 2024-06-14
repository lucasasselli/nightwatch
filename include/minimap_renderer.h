#pragma once

#include "game_state.h"

#define MINIMAP_TILE_SIZE 4
#define MINIMAP_SIZE_X 100
#define MINIMAP_SIZE_Y 100

void minimap_init(void);

void minimap_debug_draw(int x, int y, game_state_t* state);

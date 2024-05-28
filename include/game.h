#pragma once

#include <cglm/cglm.h>

#include "map.h"
#include "minigl_types.h"
#include "pd_api.h"

#define INPUT_CAMERA_TSPEED 0.5f
#define INPUT_CAMERA_RSPEED 5.0f  // Deg per Frame

#define CAMERA_FOV 60

extern PlaydateAPI* pd;

typedef struct {
    bool minimap_show;
    map_t map;
    minigl_camera_t camera;
} game_state_t;

void game_init(game_state_t* state);

void handle_keys(game_state_t* state, PDButtons pushed);

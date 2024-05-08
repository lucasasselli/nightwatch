#pragma once

#include <cglm/cglm.h>

#include "pd_api.h"

#define INPUT_CAMERA_TSPEED 0.1f
#define INPUT_CAMERA_RSPEED 4.0f  // Deg per Frame

extern PlaydateAPI* pd;

typedef struct {
    vec3 pos;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
} camera_t;

typedef struct {
    camera_t camera;
} game_state_t;

void game_init(game_state_t* state);

void handle_keys(game_state_t* state, PDButtons pushed);

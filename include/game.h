#pragma once

#include <cglm/cglm.h>

#include "map.h"
#include "minigl_types.h"
#include "pd_api.h"

#define CAMERA_FOV 60

extern PlaydateAPI* pd;

typedef enum {
    ENEMY_HIDDEN,
    ENEMY_FROZEN,
    ENEMY_ROAMING,
    ENEMY_CHASING
} enemy_state_t;

typedef struct {
    bool minimap_show;
    map_t map;
    minigl_camera_t player_camera;
    float torch_charge;
    float torch_on;
    minigl_camera_t enemy_camera;
    enemy_state_t enemy_state;
} game_state_t;

void game_init(game_state_t* state);

void game_handle_keys(game_state_t* state, PDButtons pushed);

void game_handle_crank(game_state_t* state);

void game_handle_enemy(game_state_t* state);

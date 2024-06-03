#pragma once

#include <cglm/cglm.h>

#include "map.h"
#include "minigl_types.h"
#include "pd_api.h"

#define CAMERA_FOV 60
#define CAMERA_MAX_Z 50.0f

extern PlaydateAPI* pd;

typedef enum {
    ENEMY_RESET,
    ENEMY_HIDDEN,
    ENEMY_ROAMING,
    ENEMY_ALERT,
    ENEMY_CHASING,
    ENEMY_SEARCHING
} enemy_state_t;

typedef struct {
    bool minimap_show;
    map_t map;

    minigl_camera_t player_camera;
    ivec2 player_tile;

    ivec2 enemy_tile;
    enemy_state_t enemy_state;
    bool enemy_in_fov;
    float enemy_awareness;

    float torch_flicker;
    float torch_charge;
    float torch_on;
} game_state_t;

void game_init(game_state_t* state);

void game_handle_keys(game_state_t* state, PDButtons pushed, float delta_t);

void game_handle_crank(game_state_t* state, float delta_t);

void game_update(game_state_t* state, float delta_t);

void game_enemy_ai(game_state_t* state, float delta_t);

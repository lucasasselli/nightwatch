#pragma once

#include "map.h"
#include "pd_api.h"

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

    camera_t camera;
    ivec2 player_tile;

    ivec2 enemy_tile;
    enemy_state_t enemy_state;
    bool enemy_in_fov;
    float enemy_awareness;

    float torch_flicker;
    float torch_charge;
    float torch_on;
} game_state_t;

extern game_state_t gs;

void game_init(void);

void game_handle_keys(PDButtons pushed, float delta_t);

void game_handle_crank(float delta_t);

void game_update(float delta_t);

void game_viz_update(void);

void game_enemy_ai(float delta_t);

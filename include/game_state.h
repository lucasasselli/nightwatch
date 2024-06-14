#pragma once

#include <cglm/cglm.h>

#include "a_star.h"

typedef enum {
    ENEMY_HIDDEN,
    ENEMY_FOLLOW,
    ENEMY_SPOTTED,
    ENEMY_DESPAWN_WAIT,
    ENEMY_DESPAWN,
    ENEMY_CHASING,
    ENEMY_WON
} enemy_state_t;

typedef enum {
    PLAYER_ACTIVE,
    PLAYER_READING,
    PLAYER_GAMEOVER
} player_state_t;

typedef struct {
    map_t map;

    camera_t camera;

    player_state_t player_state;
    bool player_interact;
    map_item_t* player_interact_item;

    enemy_state_t enemy_state;
    ivec2 enemy_tile;
    bool enemy_in_fov;
    int enemy_spotted_cnt;
    float enemy_awareness;   // Increases with the use of the torch and when you are spotted
    float enemy_aggression;  // Increases when spotted

    float torch_flicker;
    float torch_charge;
    float torch_on;

    a_star_path_t path_to_player;
} game_state_t;

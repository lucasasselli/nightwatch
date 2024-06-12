#pragma once

#include "a_star.h"
#include "map.h"
#include "pd_api.h"

extern PlaydateAPI* pd;

typedef enum {
    PLAYER_ACTIVE,
    PLAYER_GAMEOVER
} player_state_t;

typedef enum {
    ENEMY_RESET,
    ENEMY_HIDDEN,
    ENEMY_FOLLOW,
    ENEMY_SPOTTED,
    ENEMY_CHASING,
    ENEMY_WON
} enemy_state_t;

typedef struct {
    player_state_t player_state;
    map_t map;

    camera_t camera;

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

extern game_state_t gs;

void game_init(void);

void game_update(float delta_t);

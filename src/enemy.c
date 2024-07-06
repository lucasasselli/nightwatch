#include "enemy.h"

#include "game_state.h"
#include "utils.h"

// Enemy
#define ENEMY_ROAM_SPEED 1.0f
#define ENEMY_CHASING_SPEED 1.0f

#define ENEMY_DESPAWN_TIME 4.0f           // s
#define ENEMY_DESPAWN_FLICKER_DELAY 1.0f  // s

#define ENEMY_AWARE_SIGHT_K 0.1f
#define ENEMY_AWARE_IDLE_K 0.01f
#define ENEMY_AWARE_DARK_K -0.1f
#define ENEMY_AWARE_MAX 5.0f
#define ENEMY_AWARE_SPAWN_TH 1.0f

#define ENEMY_AGGR_SIGHT_K 1.0f
#define ENEMY_AGGR_DIST_K 3.0f
#define ENEMY_AGGR_ATTACK_TH 100.0f
#define ENEMY_AGGR_MAX ENEMY_AGGR_ATTACK_TH + 1.0f

#define ENEMY_DIST_MAX 8
#define ENEMY_DIST_MIN 1
#define ENEMY_DIST_ESCAPE 8

const char* enemy_state_names[] = {"HIDDEN", "FOLLOW", "SPOTTED", "DESPAWN_WAIT", "DESPAWN", "CHASING", "WON"};

extern game_state_t gs;

int enemy_path_prog = 0;
float enemy_move_cnt = 0;
float enemy_despawn_timer;

void enemy_reset(void) {
    gs.enemy_state = ENEMY_HIDDEN;
    gs.enemy_awareness = 0.0f;
    gs.enemy_aggression = 0.0f;
    gs.enemy_spotted_cnt = 0;
}

void enemy_update_path(void) {
    a_star_navigate(gs.map, gs.enemy_tile, IVEC2_INIT(gs.camera.pos), &gs.path_to_player);
    enemy_path_prog = 0;
}

void enemy_action_move(float speed, float delta_t) {
    if (enemy_path_prog < gs.path_to_player.size) {
        if (enemy_move_cnt > 1.0f) {
            glm_ivec2_copy(gs.path_to_player.pos[enemy_path_prog++], gs.enemy_tile);
            enemy_move_cnt = 0.0f;
        } else {
            enemy_move_cnt += delta_t * speed;
        }
    }
}

int enemy_get_distance(void) {
    return gs.path_to_player.size - enemy_path_prog;
}

static void enemy_fsm_change_state(enemy_state_t next_state) {
    debug("Enemy: %s -> %s", enemy_state_names[gs.enemy_state], enemy_state_names[next_state]);

    // Do transition stuff
    switch (next_state) {
        case ENEMY_HIDDEN:
            gs.enemy_awareness = 0.0f;
            gs.enemy_aggression = 0.0f;
            break;

        case ENEMY_FOLLOW:
            if (gs.enemy_state == ENEMY_HIDDEN) {
                // Spawn the enemy
                do {
                    gs.enemy_tile[0] = rand() % MAP_SIZE;
                    gs.enemy_tile[1] = rand() % MAP_SIZE;
                } while (map_get_collide_ivec2(gs.map, gs.enemy_tile) || map_viz_get_ivec2(gs.map, gs.camera.pos, gs.enemy_tile));
            }
            break;

        case ENEMY_DESPAWN_WAIT:
            enemy_despawn_timer = ENEMY_DESPAWN_TIME;
            gs.enemy_spotted_cnt++;
            break;

        case ENEMY_DESPAWN:
            break;

        case ENEMY_SPOTTED:
            gs.enemy_spotted_cnt++;
            break;

        case ENEMY_CHASING:
            break;

        case ENEMY_WON:
            gs.player_state = PLAYER_GAMEOVER;
            break;
    }

    gs.enemy_state = next_state;
}

void enemy_fsm_do(float delta_t) {
    switch (gs.enemy_state) {
        // Enemy not spawned in map
        case ENEMY_HIDDEN:
            if (gs.enemy_awareness > ENEMY_AWARE_SPAWN_TH) {
                enemy_fsm_change_state(ENEMY_FOLLOW);
            }
            break;

        // Enemy navigating map, not actively searching player
        case ENEMY_FOLLOW:
            if (!gs.enemy_in_fov) {
                // Navigate from the spwan point to the player position, but stay at a distance.
                // Get closes as awareness increments
                if (enemy_get_distance() > ENEMY_DIST_MAX - gs.enemy_awareness) {
                    enemy_action_move(ENEMY_ROAM_SPEED, delta_t);
                }

                // If awareness is zero despawn again
                if (gs.enemy_awareness == 0.0f) {
                    enemy_fsm_change_state(ENEMY_DESPAWN);
                }
            } else {
                // Enemy seen by the player!
                if (gs.enemy_spotted_cnt == 0) {
                    // First time spotted! Always despawn
                    enemy_fsm_change_state(ENEMY_DESPAWN_WAIT);
                } else {
                    enemy_fsm_change_state(ENEMY_SPOTTED);
                }
            }
            break;

        case ENEMY_DESPAWN_WAIT:
            enemy_despawn_timer -= delta_t;
            if (enemy_despawn_timer < ENEMY_DESPAWN_TIME) {
                enemy_fsm_change_state(ENEMY_DESPAWN);
            }
            break;

        case ENEMY_DESPAWN:
            enemy_despawn_timer -= delta_t;
            if (enemy_despawn_timer < 0.0f) {
                enemy_fsm_change_state(ENEMY_HIDDEN);
            }
            break;

        // Enemy spotted by player, preparing to chase
        case ENEMY_SPOTTED:
            if (gs.enemy_in_fov) {
                if (gs.enemy_aggression > ENEMY_AGGR_ATTACK_TH) {
                    enemy_fsm_change_state(ENEMY_CHASING);
                }
            } else {
                enemy_fsm_change_state(ENEMY_FOLLOW);
            }
            break;

        // Enemy actively chaing player
        case ENEMY_CHASING:
            enemy_action_move(ENEMY_CHASING_SPEED, delta_t);

            if (enemy_get_distance() == ENEMY_DIST_MIN) {
                // Reached last known position
                enemy_fsm_change_state(ENEMY_WON);
            }

            // TODO: Awareness increases distance
            // TODO: Torch intensitity decreases distance
            if (gs.path_to_player.size > ENEMY_DIST_ESCAPE) {
                // Player escaped the monster!
                enemy_fsm_change_state(ENEMY_HIDDEN);
            }
            break;

        case ENEMY_WON:
            // Wait for reset
            break;
    }
}

void enemy_update_state(float delta_t) {
    // Is enemy visible?
    if (gs.enemy_state != ENEMY_HIDDEN && gs.torch_charge > 0.0f) {
        gs.enemy_in_fov = map_viz_get_ivec2(gs.map, gs.camera.pos, gs.enemy_tile) && enemy_get_distance() <= ENEMY_DIST_MAX;
    } else {
        gs.enemy_in_fov = false;
    }

    int dist = enemy_get_distance();
    float dist_k;
    if (dist < ENEMY_DIST_MAX) {
        dist_k = ENEMY_DIST_MAX - (float)dist / ENEMY_DIST_MAX;
    } else {
        dist_k = 0.0;
    }

    // Update awareness
    float awareness_delta;
    if (gs.enemy_state == ENEMY_HIDDEN || gs.enemy_state == ENEMY_FOLLOW) {
        if (gs.torch_charge > 0) {
            // If torch is on, has charge and player is not being chased, update awareness
            if (gs.enemy_in_fov) {
                awareness_delta = ENEMY_AWARE_SIGHT_K;
            } else {
                awareness_delta = ENEMY_AWARE_IDLE_K;
            }
            awareness_delta *= gs.torch_charge;
            awareness_delta *= gs.enemy_spotted_cnt + 1;  // Getting spotted makes the game harder
            awareness_delta *= delta_t;
        } else {
            // Slowly decrease awareness when torch is off
            awareness_delta = ENEMY_AWARE_DARK_K;
            awareness_delta *= delta_t;
        }
        gs.enemy_awareness = glm_clamp(gs.enemy_awareness + awareness_delta, 0.0f, ENEMY_AWARE_MAX);
    }

    // Update aggression
    if (gs.enemy_state == ENEMY_SPOTTED) {
        // If torch is on,
        if (gs.enemy_in_fov) {
            gs.enemy_aggression += delta_t * (gs.torch_charge * ENEMY_AGGR_SIGHT_K + dist_k * ENEMY_AGGR_DIST_K);
        } else {
            gs.enemy_aggression -= delta_t * (gs.torch_charge * ENEMY_AGGR_SIGHT_K);
        }
        gs.enemy_aggression = glm_clamp(gs.enemy_aggression, 0.0f, ENEMY_AGGR_MAX);
    }

    // Update FSM
    enemy_fsm_do(delta_t);
}

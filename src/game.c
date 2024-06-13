#include "game.h"

#include "map.h"
#include "random.h"
#include "sound.h"
#include "utils.h"

// Input
#define INPUT_CAMERA_TSPEEDB -0.5f
#define INPUT_CAMERA_TSPEED0 1.0f
#define INPUT_CAMERA_TSPEED1 4.0f
#define INPUT_CAMERA_RSPEED 5.0f

#define HEADBOB_RANGE0 0.025f
#define HEADBOB_SPEED0 7.0f
#define HEADBOB_RANGE1 0.038f
#define HEADBOB_SPEED1 18.0f

// Torch
#define TORCH_DISCHARGE_RATE 0.2f
#define TORCH_CHARGE_RATE 0.01f  // Charge x deg x s

// Enemy
#define ENEMY_ROAM_SPEED 1.0f
#define ENEMY_CHASING_SPEED 1.0f

#define ENEMY_DESPAWN_TIME 2.0f           // s
#define ENEMY_DESPAWN_FLICKER_DELAY 1.0f  // s

#define ENEMY_AWARE_SIGHT_K 0.1f
#define ENEMY_AWARE_IDLE_K 0.03f
#define ENEMY_AWARE_DARK_K -0.01f
#define ENEMY_AWARE_MAX 5.0f
#define ENEMY_AWARE_SPAWN_TH 1.0f

#define ENEMY_AGGR_SIGHT_K 1.0f
#define ENEMY_AGGR_DIST_K 3.0f
#define ENEMY_AGGR_ATTACK_TH 100.0f
#define ENEMY_AGGR_MAX ENEMY_AGGR_ATTACK_TH + 1.0f

#define ENEMY_DIST_MAX 8
#define ENEMY_DIST_MIN 1
#define ENEMY_DIST_ESCAPE 8

const char* enemy_state_names[] = {"HIDDEN", "FOLLOW", "SPOTTED", "DESPAWN", "CHASING", "WON"};

// Game state
int enemy_path_prog = 0;
float enemy_move_cnt = 0;
float enemy_despawn_timer;

float headbob_timer = 0.0;
int step_sound = 0;

void game_init(void) {
    //---------------------------------------------------------------------------
    // Camera
    //---------------------------------------------------------------------------

    gs.camera.yaw = -90.0f;
    glm_vec2_copy((vec3){0.0f, -1.0f}, gs.camera.front);

    //---------------------------------------------------------------------------
    // State
    //---------------------------------------------------------------------------

    // Player state
    gs.player_state = PLAYER_ACTIVE;

    // Pick a random starting position in the map
    ivec2 t;
    t[0] = 31;  // TODO: Encode in the map or the level itself
    t[1] = 36;
    ivec2_to_vec2_center(t, gs.camera.pos);
    map_viz_update(gs.map, gs.camera);

    // Torch
    gs.torch_charge = 0.0f;

    // Enemy
    gs.enemy_state = ENEMY_HIDDEN;
    gs.enemy_awareness = 0.0f;
    gs.enemy_aggression = 0.0f;
    gs.enemy_spotted_cnt = 0;
}

static void handle_keys(PDButtons pushed, float delta_t) {
    vec2 old_pos;  // In case of collision we use this
    glm_vec2_copy(gs.camera.pos, old_pos);

    float old_bob = gs.camera.bob;

    bool is_moving = false;
    float speed = 0;
    float bob_speed;
    float bob_range;
    if (pushed & kButtonUp) {
        // Walks forward
        if (pushed & kButtonA) {
            speed = INPUT_CAMERA_TSPEED1;
            bob_speed = HEADBOB_SPEED1;
            bob_range = HEADBOB_RANGE1;
        } else {
            speed = INPUT_CAMERA_TSPEED0;
            bob_speed = HEADBOB_SPEED0;
            bob_range = HEADBOB_RANGE0;
        }
        is_moving = true;
    } else if (pushed & kButtonDown) {
        // Walk backward
        speed = INPUT_CAMERA_TSPEEDB;
        bob_speed = HEADBOB_SPEED0;
        bob_range = HEADBOB_RANGE0;
        is_moving = true;
    }

    vec2 camera_delta;
    glm_vec2_scale_as(gs.camera.front, speed * delta_t, camera_delta);
    if (speed != 0) {
        headbob_timer += delta_t * bob_speed;
        gs.camera.bob = sinf(headbob_timer) * bob_range;
        glm_vec2_add(gs.camera.pos, camera_delta, gs.camera.pos);
    }

    if (pushed & kButtonRight) {
        gs.camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        gs.camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    //---------------------------------------------------------------------------
    // Collisions
    //---------------------------------------------------------------------------

    // TODO: Make collisions not sticky!
    map_tile_t tile = map_get_tile_vec2(gs.map, gs.camera.pos);
    if (map_tile_collide(tile)) {
        glm_vec2_copy(old_pos, gs.camera.pos);
    }

    // Update the direction
    vec2 direction;
    direction[0] = cosf(glm_rad(gs.camera.yaw));
    direction[1] = sinf(glm_rad(gs.camera.yaw));
    glm_vec2_normalize_to(direction, gs.camera.front);

    // Update map visibility
    map_viz_update(gs.map, gs.camera);

    //---------------------------------------------------------------------------
    // Steps
    //---------------------------------------------------------------------------

    if (is_moving) {
        if (old_bob > 0 && gs.camera.bob < 0) {
            if (step_sound) {
                sound_effect_play(SOUND_STEP0);
            } else {
                sound_effect_play(SOUND_STEP1);
            }
            step_sound ^= 1;
        }
    } else {
        sound_effect_stop(SOUND_STEP0);
        sound_effect_stop(SOUND_STEP1);
    }
}

void enemy_move(float speed, float delta_t) {
    if (enemy_path_prog < gs.path_to_player.size) {
        if (enemy_move_cnt > 1.0f) {
            glm_ivec2_copy(gs.path_to_player.pos[enemy_path_prog++], gs.enemy_tile);
            enemy_move_cnt = 0.0f;
        } else {
            enemy_move_cnt += delta_t * speed;
        }
    }
}

int enemy_player_dist(void) {
    return gs.path_to_player.size - enemy_path_prog;
}

void enemy_fsm_change_state(enemy_state_t next_state) {
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
                map_tile_t tile;
                do {
                    gs.enemy_tile[0] = rand() % MAP_SIZE;
                    gs.enemy_tile[1] = rand() % MAP_SIZE;
                    tile = map_get_tile_ivec2(gs.map, gs.enemy_tile);
                } while (map_tile_collide(tile) || map_viz_ivec2(gs.map, gs.camera.pos, gs.enemy_tile));
            }
            break;

        case ENEMY_DESPAWN:
            enemy_despawn_timer = ENEMY_DESPAWN_TIME;
            gs.enemy_spotted_cnt++;
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
                if (enemy_player_dist() > ENEMY_DIST_MAX - gs.enemy_awareness) {
                    enemy_move(ENEMY_ROAM_SPEED, delta_t);
                }
            } else {
                // Enemy seen by the player!
                if (gs.enemy_spotted_cnt == 0) {
                    // First time spotted! Always despawn
                    enemy_fsm_change_state(ENEMY_DESPAWN);
                } else {
                    // TODO: Randomly despawn!
                    enemy_fsm_change_state(ENEMY_SPOTTED);
                }
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
            enemy_move(ENEMY_CHASING_SPEED, delta_t);

            if (enemy_player_dist() == ENEMY_DIST_MIN) {
                // Reached last known position
                enemy_fsm_change_state(ENEMY_WON);
            }

            // TODO: Awareness increases distance
            // TODO: Torch intensitity increases distance
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

void torch_flicker(bool enable) {
    gs.torch_flicker = enable;
    if (enable) {
        sound_effect_start(SOUND_FLICKER);
    } else {
        sound_effect_stop(SOUND_FLICKER);
    }
}

void game_update(float delta_t) {
    //---------------------------------------------------------------------------
    // Handle input
    //---------------------------------------------------------------------------

    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);

    if (pushed) {
        handle_keys(pushed, delta_t);
    }

    float crank_delta = fabsf(pd->system->getCrankChange());
    gs.torch_charge += crank_delta * TORCH_CHARGE_RATE * delta_t;
    if (crank_delta == 0) {
        gs.torch_charge += -TORCH_DISCHARGE_RATE * delta_t;
    }
    gs.torch_charge = glm_clamp(gs.torch_charge, 0.0f, 1.0f);

    //---------------------------------------------------------------------------
    // Update state
    //---------------------------------------------------------------------------

    // Is enemy visible?
    if (gs.enemy_state != ENEMY_HIDDEN && gs.torch_charge > 0.0f) {
        gs.enemy_in_fov = map_viz_ivec2(gs.map, gs.camera.pos, gs.enemy_tile) && enemy_player_dist() <= ENEMY_DIST_MAX;
    } else {
        gs.enemy_in_fov = false;
    }

    // Should torch flicker?
    if (gs.torch_charge > 0.0f) {
        // Torch is on!
        if (gs.enemy_state == ENEMY_CHASING) {
            torch_flicker(true);
        } else if (gs.torch_charge > 0.0f && gs.enemy_state == ENEMY_DESPAWN && enemy_despawn_timer < ENEMY_DESPAWN_FLICKER_DELAY) {
            torch_flicker(true);
        } else {
            torch_flicker(false);
        }
    } else {
        // Never flicker if torch is off!
        torch_flicker(false);
    }

    int dist = enemy_player_dist();
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

    // Handle movement
    // FIXME: Only if player changes tile
    if (gs.enemy_state != ENEMY_HIDDEN) {
        // Update the path to player
        a_star_navigate(gs.map, gs.enemy_tile, IVEC2_INIT(gs.camera.pos), &gs.path_to_player);
        enemy_path_prog = 0;
    }

    // Update FSM
    enemy_fsm_do(delta_t);
}

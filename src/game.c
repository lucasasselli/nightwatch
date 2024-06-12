#include "game.h"

#include "map.h"
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
#define TORCH_DISCHARGE_RATE 1.0f
#define TORCH_CHARGE_RATE 0.01f  // Charge x deg x sec

// Enemy
#define ENEMY_ROAM_SPEED 1.0f
#define ENEMY_CHASING_SPEED 1.0f

#define ENEMY_SPAWN_TIME 10.0f

#define ENEMY_AWARE_SIGHT_K 10.0f
#define ENEMY_AWARE_IDLE_K 1.0f
#define ENEMY_AWARE_MAX 500.0f

#define ENEMY_AGGR_SIGHT_K 1.0f
#define ENEMY_AGGR_DIST_K 3.0f
#define ENEMY_AGGR_ATTACK_THRESHOLD 100.0f
#define ENEMY_AGGR_MAX ENEMY_AGGR_ATTACK_THRESHOLD + 1.0f

#define ENEMY_BASE_DIST 7

// Heartbeat sound
#define HEARTBEAT_DIST_MIN 10
#define HEARTBEAT_DIST_MAX 50

const char* enemy_state_names[] = {"RESET", "HIDDEN", "FOLLOW", "SPOTTED", "CHASING"};

// Game state
int enemy_path_prog = 0;
float enemy_move_cnt = 0;
float enemy_spawn_timer = 0;

float heart_speed = 0.0;

float head_bob = 0.0;

void game_init(void) {
    //---------------------------------------------------------------------------
    // Camera
    //---------------------------------------------------------------------------

    gs.camera.yaw = -90.0f;
    glm_vec2_copy((vec3){0.0f, -1.0f}, gs.camera.front);

    //---------------------------------------------------------------------------
    // State
    //---------------------------------------------------------------------------

    // Pick a random starting position in the map
    ivec2 t;
    t[0] = 31;  // FIXME: Good start position
    t[1] = 36;
    ivec2_to_vec2_center(t, gs.camera.pos);
    map_viz_update(gs.map, gs.camera);

    // Torch
    gs.torch_charge = 0.0f;

    // Enemy
    gs.enemy_state = ENEMY_RESET;
}

static void handle_keys(PDButtons pushed, float delta_t) {
    vec2 camera_delta;

    vec2 old_pos;  // In case of collision we use this

    glm_vec2_copy(gs.camera.pos, old_pos);

    float speed = 0;
    if (pushed & kButtonUp) {
        // Walks forward
        if (pushed & kButtonA) {
            glm_vec2_scale_as(gs.camera.front, INPUT_CAMERA_TSPEED1 * delta_t, camera_delta);
            head_bob += delta_t * HEADBOB_SPEED1;
            gs.camera.bob = sinf(head_bob) * HEADBOB_RANGE1;
        } else {
            glm_vec2_scale_as(gs.camera.front, INPUT_CAMERA_TSPEED0 * delta_t, camera_delta);
            head_bob += delta_t * HEADBOB_SPEED0;
            gs.camera.bob = sinf(head_bob) * HEADBOB_RANGE0;
        }
    } else if (pushed & kButtonDown) {
        // Walk backward
        glm_vec2_scale_as(gs.camera.front, INPUT_CAMERA_TSPEEDB * delta_t, camera_delta);
        head_bob += delta_t * HEADBOB_SPEED0;
        gs.camera.bob = sinf(head_bob) * HEADBOB_RANGE0;
    }

    glm_vec2_add(gs.camera.pos, camera_delta, gs.camera.pos);

    if (pushed & kButtonRight) {
        gs.camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        gs.camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    //---------------------------------------------------------------------------
    // Collisions
    //---------------------------------------------------------------------------

    // FIXME: Make collisions not sticky!
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
    gs.enemy_state = next_state;
}

void enemy_fsm_do(float delta_t) {
    switch (gs.enemy_state) {
        // Dummy state to initialize FSM
        case ENEMY_RESET:
            gs.enemy_awareness = 0.0f;
            gs.enemy_spotted_cnt = 0;
            enemy_spawn_timer = ENEMY_SPAWN_TIME;
            enemy_fsm_change_state(ENEMY_HIDDEN);
            break;

        // Enemy not spawned in map
        case ENEMY_HIDDEN:
            enemy_spawn_timer -= delta_t;

            if (enemy_spawn_timer < 0.0f) {
                // Spwan the enemy
                map_tile_t tile;
                do {
                    gs.enemy_tile[0] = rand() % MAP_SIZE;
                    gs.enemy_tile[1] = rand() % MAP_SIZE;
                    tile = map_get_tile_ivec2(gs.map, gs.enemy_tile);
                } while (map_tile_collide(tile) || map_viz_ivec2(gs.map, gs.camera.pos, gs.enemy_tile));
                enemy_fsm_change_state(ENEMY_FOLLOW);

                debug("Enemy (FOLLOW): spawned at %d %d!", gs.enemy_tile[0], gs.enemy_tile[1]);
            }
            break;

        // Enemy navigating map, not actively searching player
        case ENEMY_FOLLOW:
            if (!gs.enemy_in_fov) {
                // When the player doesn't see the enemy, follow him
                if (enemy_player_dist() > ENEMY_BASE_DIST) {
                    enemy_move(ENEMY_ROAM_SPEED, delta_t);
                }
            } else {
                // Enemy seen by the player!
                gs.enemy_spotted_cnt++;
                if (gs.enemy_spotted_cnt == 1) {
                    sound_effect_play(SOUND_DISCOVERED);
                }
                enemy_fsm_change_state(ENEMY_SPOTTED);
            }
            break;

        // Enemy spotted by player, preparing to chase
        case ENEMY_SPOTTED:
            if (gs.enemy_in_fov) {
                if (gs.enemy_aggression > ENEMY_AGGR_ATTACK_THRESHOLD) {
                    enemy_fsm_change_state(ENEMY_CHASING);
                }
            } else {
                enemy_fsm_change_state(ENEMY_FOLLOW);
            }
            // TODO: Randomly despawn!
            break;

        // Enemy actively chaing player
        case ENEMY_CHASING:
            enemy_move(ENEMY_CHASING_SPEED, delta_t);

            if (enemy_player_dist() == 0) {
                // Reached last known position
                // TODO: Game over
                enemy_fsm_change_state(ENEMY_HIDDEN);
            }

            // TODO: Awareness increases distance
            // TODO: Torch intensitity increases distance
            if (gs.path_to_player.size > 10) {
                // Player escaped the monster!
                gs.enemy_aggression = 0.0f;
                enemy_spawn_timer = ENEMY_SPAWN_TIME;  // TODO: Randomize spawn time
                enemy_fsm_change_state(ENEMY_HIDDEN);
            }
            break;
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

    gs.torch_on = gs.torch_charge > 0.0f;

    // Is enemy visible?
    if (gs.enemy_state != ENEMY_HIDDEN && gs.torch_on) {
        gs.enemy_in_fov = map_viz_ivec2(gs.map, gs.camera.pos, gs.enemy_tile) && enemy_player_dist() <= ENEMY_BASE_DIST;
    } else {
        gs.enemy_in_fov = false;
    }

    // Flicker effect
    if (gs.enemy_state == ENEMY_CHASING) {
        gs.torch_flicker = true;
    } else {
        gs.torch_flicker = false;
    }

    if (gs.torch_flicker && gs.torch_on) {
        sound_effect_start(SOUND_FLICKER);
    } else {
        sound_effect_stop(SOUND_FLICKER);
    }

    int dist = enemy_player_dist();
    float dist_k;
    if (dist < ENEMY_BASE_DIST) {
        dist_k = ENEMY_BASE_DIST - (float)dist / ENEMY_BASE_DIST;
    } else {
        dist_k = 0.0;
    }

    // Update awareness
    if (gs.torch_on && gs.enemy_state != ENEMY_CHASING) {
        // If torch is on, has charge and player is not being chased, update awareness
        if (gs.enemy_in_fov) {
            gs.enemy_awareness += delta_t * gs.torch_charge * ENEMY_AWARE_SIGHT_K;
        } else {
            gs.enemy_awareness += delta_t * gs.torch_charge * ENEMY_AWARE_IDLE_K;
        }
    } else if (gs.enemy_state == ENEMY_CHASING) {
        // If enemy is chasing, awareness doesn't change.
    } else {
        // Slowly decrease awareness when torch is off
        gs.enemy_awareness -= delta_t;
    }
    gs.enemy_awareness = glm_clamp(gs.enemy_awareness, 0.0f, ENEMY_AWARE_MAX);

    // Update aggression
    if (gs.enemy_state == ENEMY_SPOTTED) {
        // If torch is on,
        if (gs.enemy_in_fov) {
            gs.enemy_aggression += delta_t * (gs.torch_charge * ENEMY_AGGR_SIGHT_K + dist_k * ENEMY_AGGR_DIST_K);
        }
        gs.enemy_aggression = glm_clamp(gs.enemy_aggression, 0.0f, ENEMY_AGGR_MAX);
    }

    // Update FSM
    enemy_fsm_do(delta_t);

    // Handle movement
    // FIXME: Only if player changes tile
    if (gs.enemy_state != ENEMY_HIDDEN) {
        // Update the path to player
        a_star_navigate(gs.map, gs.enemy_tile, IVEC2_INIT(gs.camera.pos), &gs.path_to_player);
        enemy_path_prog = 0;

        // Heartbeat sound
        // FIXME: Add low pass filter
        if (gs.enemy_state == ENEMY_SPOTTED || gs.enemy_state == ENEMY_CHASING) {
            heart_speed = 1.0;
        } else if (gs.enemy_state == ENEMY_HIDDEN) {
            heart_speed = 0.0;
        } else {
            // Heartbeat sound
            heart_speed = (HEARTBEAT_DIST_MAX - ((float)clampi(gs.path_to_player.size, HEARTBEAT_DIST_MIN, HEARTBEAT_DIST_MAX))) /
                          (HEARTBEAT_DIST_MAX - HEARTBEAT_DIST_MIN);
        }

        // sound_play_range(SOUND_HEARTBEAT, 0, 200000 - 170000 * heart_speed);
    }
}

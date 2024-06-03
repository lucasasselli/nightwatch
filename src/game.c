#include "game.h"

#include "a_star.h"
#include "map.h"
#include "sound.h"
#include "utils.h"

const int ENEMY_AWARE_SPAWN_THRESHOLD = 1000.0f;
const int ENEMY_AWARE_CHASE_THRESHOLD = 5000.0f;
const int ENEMY_AWARE_MAX = 6000.0f;

a_star_path_t enemy_path;
int enemy_path_prog = 0;
float enemy_move_cnt = 0;
ivec2 enemy_player_last_pos;

void rand_empty_tile(map_t map, ivec2 tile_pos) {
    map_tile_t tile;
    do {
        tile_pos[0] = rand() % MAP_SIZE;
        tile_pos[1] = rand() % MAP_SIZE;
        tile = map_get_tile(map, tile_pos);
    } while (map_tile_collide(tile));
}

void game_init(game_state_t* state) {
    // Player camera
    state->player_camera.yaw = -90.0f;
    state->player_camera.pitch = 0.0f;
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, state->player_camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, state->player_camera.up);

    // Pick a random starting position in the map
    ivec2 player_spawn_tile;
    rand_empty_tile(state->map, player_spawn_tile);
    pos_tile_to_world(player_spawn_tile, state->player_camera.pos);

    // Torch
    state->torch_charge = 0.0;

    // Enemy
    state->enemy_state = ENEMY_RESET;
}

void game_handle_keys(game_state_t* state, PDButtons pushed, float delta_t) {
    vec3 camera_delta;

    const float INPUT_CAMERA_TSPEED1 = 0.5f;
    const float INPUT_CAMERA_TSPEED2 = 0.7f;
    const float INPUT_CAMERA_RSPEED = 5.0f;  // Deg per Frame;

    vec3 old_pos;
    glm_vec3_copy(state->player_camera.pos, old_pos);

    if (pushed & kButtonUp) {
        // Walks forward
        float speed = INPUT_CAMERA_TSPEED1;
        if (pushed & kButtonA) {
            speed = INPUT_CAMERA_TSPEED2;
        }
        glm_vec3_scale_as(state->player_camera.front, speed, camera_delta);
        glm_vec3_add(state->player_camera.pos, camera_delta, state->player_camera.pos);
    } else if (pushed & kButtonDown) {
        // Walk backward
        glm_vec3_scale_as(state->player_camera.front, INPUT_CAMERA_TSPEED1, camera_delta);
        glm_vec3_sub(state->player_camera.pos, camera_delta, state->player_camera.pos);
    }

    if (pushed & kButtonRight) {
        state->player_camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        state->player_camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    if (pushed & kButtonB) {
        // state->minimap_show ^= 1;
    }

    //---------------------------------------------------------------------------
    // Collisions
    //---------------------------------------------------------------------------

    pos_world_to_tile(state->player_camera.pos, state->player_tile);
    map_tile_t tile = map_get_tile(state->map, state->player_tile);

    if (map_tile_collide(tile)) {
        glm_vec3_copy(old_pos, state->player_camera.pos);
    }

    // Update the direction
    vec3 direction;
    direction[0] = cosf(glm_rad(state->player_camera.yaw)) * cosf(glm_rad(state->player_camera.pitch));
    direction[1] = sinf(glm_rad(state->player_camera.pitch));
    direction[2] = sinf(glm_rad(state->player_camera.yaw)) * cosf(glm_rad(state->player_camera.pitch));
    glm_vec3_normalize_to(direction, state->player_camera.front);

    // debug("Player: %d %d", state->player_tile[0], state->player_tile[1]);
    // debug("Camera: %f %f", state->player_camera.pos[0], state->player_camera.pos[2]);
}

void game_handle_crank(game_state_t* state, float delta_t) {
    const float TORCH_DISCHARGE_RATE = 0.01f;
    const float TORCH_CHARGE_RATE = 0.001f;

    float crank_delta = fabsf(pd->system->getCrankChange());
    state->torch_charge += -TORCH_DISCHARGE_RATE;
    state->torch_charge += crank_delta * TORCH_CHARGE_RATE;
    state->torch_charge = glm_clamp(state->torch_charge, 0.0f, 1.0f);

    state->torch_on = !pd->system->isCrankDocked();
}

void game_update(game_state_t* state, float delta_t) {
    const float ENEMY_AWARE_TORCH_K = 1.0f;
    const float ENEMY_AWARE_SIGHT_K = 50.0f;

    // Is enemy visible?
    // FIXME: In direct sight????
    if (state->enemy_state != ENEMY_HIDDEN && state->torch_on) {
        state->enemy_in_fov = tile_in_fov(state->enemy_tile, state->player_camera, 60, 0);
    } else {
        state->enemy_in_fov = false;
    }

    // Flicker effect
    if (state->enemy_state == ENEMY_CHASING) {
        state->torch_flicker = true;
    } else {
        state->torch_flicker = false;
    }

    if (state->torch_flicker && state->torch_on) {
        sound_effect_start(SOUND_FLICKER);
    } else {
        sound_effect_stop(SOUND_FLICKER);
    }

    // Update awareness
    if (state->torch_on && state->torch_charge > 0.0f && state->enemy_state != ENEMY_CHASING) {
        // If torch is on, has charge and player is not being chased, update awareness
        if (state->enemy_in_fov) {
            state->enemy_awareness += delta_t * state->torch_charge * ENEMY_AWARE_SIGHT_K;
        } else {
            state->enemy_awareness += delta_t * state->torch_charge * ENEMY_AWARE_TORCH_K;
        }
    } else if (state->enemy_state == ENEMY_CHASING) {
        // If enemy is chasing, awareness doesn't change.
    } else {
        // Slowly decrease awareness when torch is off
        state->enemy_awareness -= delta_t;
    }

    state->enemy_awareness = glm_clamp(state->enemy_awareness, 0.0f, ENEMY_AWARE_MAX);

    // Make this grow with the distance instead of awareness
    sound_play_range(SOUND_HEARTBEAT, 0, 120000 - 100000 * (state->enemy_awareness / ENEMY_AWARE_MAX));
}

void enemy_path_move(game_state_t* state, float speed, float delta_t) {
    const float ENEMY_MOVE_THRESHOLD_MAX = 1000.0f;

    assert(speed <= ENEMY_MOVE_THRESHOLD_MAX);

    if (enemy_path_prog < enemy_path.size) {
        if (enemy_move_cnt > (ENEMY_MOVE_THRESHOLD_MAX - speed)) {
            glm_ivec2_copy(enemy_path.pos[enemy_path_prog++], state->enemy_tile);
            debug("Enemy: %d %d", state->enemy_tile[0], state->enemy_tile[1]);
            enemy_move_cnt = 0.0f;
        } else {
            enemy_move_cnt += delta_t;
        }
    }
}

bool enemy_path_complete(void) {
    return !(enemy_path_prog < enemy_path.size);
}

void enemy_path_to_player(game_state_t* state) {
    debug("Enemy (CHASING): player moved to %d %d", state->player_tile[0], state->player_tile[1]);

    // Update path
    if (!ivec2_eq(enemy_player_last_pos, state->player_tile)) {
        // Update path only if player has moved
        a_star_navigate(state->map, state->enemy_tile, state->player_tile, &enemy_path);
        enemy_path_prog = 0;
    }

    // Store last known position
    glm_ivec2_copy(state->player_tile, enemy_player_last_pos);
}

void game_enemy_ai(game_state_t* state, float delta_t) {
    const float ENEMY_ROAMING_SPEED = 200.0f;
    const float ENEMY_CHASING_SPEED = 400.0f;

    switch (state->enemy_state) {
        // Dummy state to initialize FSM
        case ENEMY_RESET:
            state->enemy_awareness = 0.0f;
            state->enemy_state = ENEMY_HIDDEN;
            break;

        // Enemy not spawned in map
        case ENEMY_HIDDEN:
            if (state->enemy_awareness > ENEMY_AWARE_SPAWN_THRESHOLD) {
                // FIXME: Spawn out of sight an far from player
                rand_empty_tile(state->map, state->enemy_tile);
                state->enemy_state = ENEMY_ROAMING;
                debug("Enemy (ROAMING): spawned at %d %d!", state->enemy_tile[0], state->enemy_tile[1]);
            }
            break;

        // Enemy navigating map, not actively searching player
        case ENEMY_ROAMING:
            if (!state->enemy_in_fov) {
                // Enemy not in the FOV of the player
                enemy_path_move(state, ENEMY_ROAMING_SPEED, delta_t);

                if (enemy_path_complete()) {
                    ivec2 target_tile;
                    rand_empty_tile(state->map, target_tile);
                    a_star_navigate(state->map, state->enemy_tile, target_tile, &enemy_path);
                    enemy_path_prog = 0;
                    debug("Enemy (ROAMING): path complete! Begin new path...");
                }
            } else {
                // Enemy seen by the player!
                state->enemy_state = ENEMY_ALERT;
                sound_effect_play(SOUND_DISCOVERED);
            }
            break;

        // Enemy spotted by player, preparing to chase
        case ENEMY_ALERT:
            if (state->enemy_awareness > ENEMY_AWARE_CHASE_THRESHOLD) {
                state->enemy_state = ENEMY_CHASING;
            }
            break;

        // Enemy actively chaing player
        case ENEMY_CHASING:
            if (state->torch_on) {
                enemy_path_to_player(state);
            }

            enemy_path_move(state, ENEMY_CHASING_SPEED, delta_t);

            if (enemy_path_complete()) {
                // Reached last known position
                if (ivec2_eq(state->enemy_tile, state->player_tile)) {
                    // GAME OVER!
                    debug("Enemy (CHASING): player reached!");
                } else {
                    // Begin search
                    state->enemy_state = ENEMY_SEARCHING;
                    debug("Enemy (SEARCHING): player lost!");
                }
            }
            break;

        // Enemy actively chasing player
        case ENEMY_SEARCHING:
            if (state->torch_on) {
                debug("Enemy (CHASING): player visible!");
                state->enemy_state = ENEMY_CHASING;
                enemy_path_to_player(state);
            } else if (state->enemy_awareness < ENEMY_AWARE_SPAWN_THRESHOLD) {
                state->enemy_state = ENEMY_HIDDEN;
                debug("Enemy (HIDDEN)");
            }
            break;
    }
}

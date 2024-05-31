#include "game.h"

#include "a_star.h"
#include "map.h"
#include "utils.h"

a_star_path_t enemy_path;
int enemy_path_prog = 0;
int enemy_hide_cntdown;

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
    enemy_hide_cntdown = 1;
    state->enemy_state = ENEMY_HIDDEN;
    state->enemy_awareness = 0;
}

void game_handle_keys(game_state_t* state, PDButtons pushed) {
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

    debug("Player: %d %d", state->player_tile[0], state->player_tile[1]);
    // debug("Camera: %f %f", state->player_camera.pos[0], state->player_camera.pos[2]);
}

void game_handle_crank(game_state_t* state) {
    const float TORCH_DISCHARGE_RATE = 0.01f;
    const float TORCH_CHARGE_RATE = 0.001f;

    float crank_delta = fabsf(pd->system->getCrankChange());
    state->torch_charge += -TORCH_DISCHARGE_RATE;
    state->torch_charge += crank_delta * TORCH_CHARGE_RATE;
    state->torch_charge = glm_clamp(state->torch_charge, 0.0f, 1.0f);

    state->torch_on = !pd->system->isCrankDocked();
}

void game_update(game_state_t* state) {
    if (state->enemy_state != ENEMY_HIDDEN && state->torch_on) {
        state->enemy_in_fov = tile_in_fov(state->enemy_tile, state->player_camera, 60, 0);
    } else {
        state->enemy_in_fov = false;
    }

    if (state->enemy_state == ENEMY_CHASING && state->enemy_in_fov) {
        state->torch_flicker = true;
    } else {
        state->torch_flicker = false;
    }
}

bool enemy_path_move(game_state_t* state) {
    if (enemy_path_prog < enemy_path.size) {
        glm_ivec2_copy(enemy_path.pos[enemy_path_prog++], state->enemy_tile);
        debug("Enemy: %d %d", state->enemy_tile[0], state->enemy_tile[1]);
        return true;
    } else {
        return false;
    }
}

void game_handle_enemy(game_state_t* state) {
    const int AWARENESS_MAX = 3;

    switch (state->enemy_state) {
        case ENEMY_HIDDEN:
            if (enemy_hide_cntdown == 0) {
                // FIXME: Spawn out of sight an far from player
                rand_empty_tile(state->map, state->enemy_tile);
                debug("Enemy (ROAMING): spawned at %d %d!", state->enemy_tile[0], state->enemy_tile[1]);
                state->enemy_state = ENEMY_ROAMING;
            } else {
                enemy_hide_cntdown--;
            }
            break;

        case ENEMY_ROAMING:
            if (!state->enemy_in_fov) {
                // Enemy not in the FOV of the player
                if (!enemy_path_move(state)) {
                    // Enemy path complete
                    ivec2 target_tile;
                    rand_empty_tile(state->map, target_tile);
                    a_star_navigate(state->map, state->enemy_tile, target_tile, &enemy_path);
                    enemy_path_prog = 0;
                    debug("Enemy (ROAMING): path complete! Begin new path...");
                }
            } else {
                // Enemy seen by the player!
                state->enemy_state = ENEMY_ALERT;
            }
            break;

        case ENEMY_ALERT:
            if (state->enemy_in_fov) {
                state->enemy_awareness++;
                debug("Enemy (ALERT): player seen! Awareness %d", state->enemy_awareness);

                if (state->enemy_awareness > AWARENESS_MAX) {
                    state->enemy_state = ENEMY_CHASING;
                }
            } else {
                debug("Enemy (ALERT): player NOT seen! Awareness %d", state->enemy_awareness);
                state->enemy_awareness--;
                if (state->enemy_awareness == 0) {
                    state->enemy_state = ENEMY_ROAMING;
                }
            }

            break;

        case ENEMY_CHASING:
            // Assume player constantly moving
            // FIXME: Store the known position!
            if (state->enemy_in_fov) {
                debug("Enemy (CHASING): player visible!");
                a_star_navigate(state->map, state->enemy_tile, state->player_tile, &enemy_path);
                enemy_path_prog = 0;
            }
            // FIXME: Once is path is over despawn...
            enemy_path_move(state);
            break;
    }
}

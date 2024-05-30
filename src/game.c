#include "game.h"

#include "a_star.h"
#include "map.h"
#include "utils.h"

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
    ivec2 enemy_spawn_tile;
    rand_empty_tile(state->map, enemy_spawn_tile);
    pos_tile_to_world(enemy_spawn_tile, state->enemy_camera.pos);

    state->enemy_state = ENEMY_ROAMING;
}

bool move_camera(minigl_camera_t* camera, map_t map, bool forward, float speed) {
    vec3 camera_delta;
    vec3 old_pos;

    // Move
    glm_vec3_copy(camera->pos, old_pos);
    glm_vec3_scale_as(camera->front, speed, camera_delta);

    if (forward) {
        glm_vec3_add(camera->pos, camera_delta, camera->pos);
    } else {
        glm_vec3_add(camera->pos, camera_delta, camera->pos);
    }

    // Check collision
    ivec2 tile_pos;
    pos_world_to_tile(camera->pos, tile_pos);
    map_tile_t tile = map_get_tile(map, tile_pos);

    if (map_tile_collide(tile)) {
        glm_vec3_copy(old_pos, camera->pos);
        return false;
    }
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

    ivec2 tile_pos;
    pos_world_to_tile(state->player_camera.pos, tile_pos);
    map_tile_t tile = map_get_tile(state->map, tile_pos);

    if (map_tile_collide(tile)) {
        glm_vec3_copy(old_pos, state->player_camera.pos);
    }

    // Update the direction
    vec3 direction;
    direction[0] = cosf(glm_rad(state->player_camera.yaw)) * cosf(glm_rad(state->player_camera.pitch));
    direction[1] = sinf(glm_rad(state->player_camera.pitch));
    direction[2] = sinf(glm_rad(state->player_camera.yaw)) * cosf(glm_rad(state->player_camera.pitch));
    glm_vec3_normalize_to(direction, state->player_camera.front);

    debug("Tile  : %d %d", tile_pos[0], tile_pos[1]);
    debug("Camera: %f %f", state->player_camera.pos[0], state->player_camera.pos[2]);
    debug("Enemy : %f %f", state->enemy_camera.pos[0], state->enemy_camera.pos[2]);
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

void game_handle_enemy(game_state_t* state) {
    const float ENEMY_SPEED = 0.2f;

    // Enemy navigation algorithm:
    // 1) Pick a final target (can be player pos)
    // 2) Calculate direction to final target
    // 3) Check for collisions along the way
    //    3.1) If a collision is found, pick an intermediate point
    //         That can be reached without collisions
    //    3.2) Once the intermediate point is reached, restart from 2)
    // 4) Once final point is reached restart from 1)

    // Position reached, pick new position

    // Pick position to target
    ivec2 enemy_target_tile;
    vec3 enemy_target_world;
    rand_empty_tile(state->map, enemy_target_tile);
    pos_tile_to_world(enemy_target_tile, enemy_target_world);

    // Chase
    a_star_path_t path_to_player;
    ivec2 enemy_tile;
    ivec2 player_tile;
    pos_world_to_tile(state->enemy_camera.pos, enemy_tile);
    pos_world_to_tile(state->player_camera.pos, player_tile);
    a_star_navigate(state->map, enemy_tile, player_tile, &path_to_player);

    // Update position
    pos_tile_to_world(path_to_player.pos[1], state->enemy_camera.pos);

    // Check collisions
}

#include "game.h"

#include "map.h"
#include "utils.h"

void game_init(game_state_t* state) {
    state->camera.yaw = -90.0f;
    state->camera.pitch = 0.0f;
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, state->camera.pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, state->camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, state->camera.up);
}

void handle_keys(game_state_t* state, PDButtons pushed) {
    vec3 camera_delta;

    const float INPUT_CAMERA_TSPEED1 = 0.5f;
    const float INPUT_CAMERA_TSPEED2 = 0.7f;
    const float INPUT_CAMERA_RSPEED = 5.0f;  // Deg per Frame;

    vec3 old_pos;
    glm_vec3_copy(state->camera.pos, old_pos);

    // FIXME: Improve once keymap is decided
    if (pushed & kButtonUp) {
        // Walks forward
        float speed = INPUT_CAMERA_TSPEED1;
        if (pushed & kButtonA) {
            speed = INPUT_CAMERA_TSPEED2;
        }
        glm_vec3_scale_as(state->camera.front, speed, camera_delta);
        glm_vec3_add(state->camera.pos, camera_delta, state->camera.pos);
    } else if (pushed & kButtonDown) {
        // Walk backward
        glm_vec3_scale_as(state->camera.front, INPUT_CAMERA_TSPEED1, camera_delta);
        glm_vec3_sub(state->camera.pos, camera_delta, state->camera.pos);
    }

    if (pushed & kButtonRight) {
        state->camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        state->camera.yaw -= INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonB) {
        // state->minimap_show ^= 1;
    }

    // Check collision
    ivec2 tile_pos;
    pos_world_to_tile(state->camera.pos, tile_pos);
    map_tile_t tile = map_get_tile(state->map, tile_pos);

    if (map_tile_collide(tile)) {
        glm_vec3_copy(old_pos, state->camera.pos);
    }

    // Update the direction
    vec3 direction;
    direction[0] = cosf(glm_rad(state->camera.yaw)) * cosf(glm_rad(state->camera.pitch));
    direction[1] = sinf(glm_rad(state->camera.pitch));
    direction[2] = sinf(glm_rad(state->camera.yaw)) * cosf(glm_rad(state->camera.pitch));
    glm_vec3_normalize_to(direction, state->camera.front);

    debug("Tile  : %d %d", tile_pos[0], tile_pos[1]);
    debug("Camera: %f %f", state->camera.pos[0], state->camera.pos[2]);
}

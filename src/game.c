#include "game.h"

#include "map.h"

void game_init(game_state_t* state) {
    state->camera.yaw = -90.0f;
    state->camera.pitch = 0.0f;
    // FIXME
    glm_vec3_copy((vec3){MAP_TILE_SIZE * MAP_SIZE / 2, 1.0f, MAP_TILE_SIZE * MAP_SIZE / 2}, state->camera.pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, state->camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, state->camera.up);
}

void handle_keys(game_state_t* state, PDButtons pushed) {
    vec3 camera_delta;

    // FIXME: Improve once keymap is decided
    if (pushed & kButtonUp) {
        glm_vec3_scale_as(state->camera.front, INPUT_CAMERA_TSPEED, camera_delta);
        glm_vec3_add(state->camera.pos, camera_delta, state->camera.pos);
    }
    if (pushed & kButtonDown) {
        glm_vec3_scale_as(state->camera.front, INPUT_CAMERA_TSPEED, camera_delta);
        glm_vec3_sub(state->camera.pos, camera_delta, state->camera.pos);
    }
    if (pushed & kButtonRight) {
        state->camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        state->camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    // Update the direction
    vec3 direction;
    direction[0] = cosf(glm_rad(state->camera.yaw)) * cosf(glm_rad(state->camera.pitch));
    direction[1] = sinf(glm_rad(state->camera.pitch));
    direction[2] = sinf(glm_rad(state->camera.yaw)) * cosf(glm_rad(state->camera.pitch));
    glm_vec3_normalize_to(direction, state->camera.front);
}

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

    vec3 old_pos;
    glm_vec3_copy(state->camera.pos, old_pos);

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
    if (pushed & kButtonB) {
        state->minimap_show ^= 1;
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

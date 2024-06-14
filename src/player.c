#include "player.h"

#include "game_state.h"
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

extern game_state_t gs;

float headbob_timer = 0.0;
int step_sound = 0;

void player_reset(void) {
    // Player state
    gs.player_state = PLAYER_ACTIVE;

    // Pick a random starting position in the map
    ivec2 t;
    t[0] = 31;  // TODO: Encode in the map or the level itself
    t[1] = 36;
    ivec2_to_vec2_center(t, gs.camera.pos);
    map_viz_update(gs.map, gs.camera);
}

void player_action_note(bool open) {
    if (open) {
        gs.player_state = PLAYER_READING;
        sound_effect_play(SOUND_NOTE);
    } else {
        gs.player_state = PLAYER_ACTIVE;
        gs.player_interact_item->hidden = true;
        gs.player_interact = false;
    }
}

void player_check_interaction(void) {
    // What is the tile directly in front of the player?
    vec2 action_tile_pos;
    glm_vec2_add(gs.camera.pos, gs.camera.front, action_tile_pos);
    map_tile_t action_tile = map_get_tile_vec2(gs.map, action_tile_pos);
    gs.player_interact = false;
    for (int i = 0; i < action_tile.item_cnt; i++) {
        if (action_tile.items[i].action && !action_tile.items[i].hidden) {
            gs.player_interact = true;
            gs.player_interact_item = &action_tile.items[i];
            break;  // Only one interactable item per tile
        }
    }
}

void player_action_move(PDButtons pushed, float delta_t) {
    float old_bob = gs.camera.bob;

    vec2 old_pos;  // In case of collision we use this
    glm_vec2_copy(gs.camera.pos, old_pos);

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

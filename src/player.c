#include "player.h"

#include "constants.h"
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
    t[1] = 59;
    ivec2_to_vec2_center(t, gs.camera.pos);

    camera_update_trans(&gs.camera);
}

void player_action_note(bool show) {
    if (show) {
        gs.player_state = PLAYER_READING;
        gs.notes_found[gs.player_interact_item->action.arg] = true;
        sound_effect_play(SOUND_NOTE);
    } else {
        gs.player_state = PLAYER_ACTIVE;
        gs.player_interact_item->hidden = true;
        gs.player_interact = false;
    }
}

void player_action_inventory(bool show) {
    if (show) {
        gs.player_state = PLAYER_INVENTORY;
        sound_effect_play(SOUND_NOTE);
    } else {
        gs.player_state = PLAYER_ACTIVE;
    }
}

void player_action_keypad(bool show) {
    if (show) {
        gs.player_state = PLAYER_KEYPAD;
        gs.keypad_sel = 5;
        gs.keypad_cnt = 0;
    } else {
        gs.player_state = PLAYER_ACTIVE;
    }
}

void player_action_inspect(bool show) {
    if (show) {
        gs.player_state = PLAYER_INSPECT;
    } else {
        gs.player_state = PLAYER_ACTIVE;
    }
}

void player_check_interaction(void) {
    // What is the tile directly in front of the player?
    vec2 action_tile_pos;
    glm_vec2_add(gs.camera.pos, gs.camera.front, action_tile_pos);
    map_tile_t tile = map_get_tile_vec2(gs.map, action_tile_pos);
    gs.player_interact = false;

    // TODO: Use callback to game?
    item_t* item = tile.items;
    while (item != NULL) {
        if (item->action.type != ACTION_NONE && !item->hidden) {
            gs.player_interact = true;
            gs.player_interact_item = item;
            break;  // Only one interactable item per tile
        }
        item = item->next;
    }
}

static bool is_colliding(vec2 pos) {
    map_tile_t tile = map_get_tile_vec2(gs.map, pos);

    if (tile_get_collide(tile)) {
        return true;
    }

    // Special case for doors!
    item_t* item = tile_find_item(tile, -1, -1, ACTION_KEYPAD);
    if (item != NULL) {
        return true;
    }

    return false;
}

void move_to_dir(vec2 pos, vec2 dir, float speed, vec2 out) {
    vec2 camera_delta;
    glm_vec2_scale_as(dir, speed, camera_delta);
    glm_vec2_add(pos, camera_delta, out);
}

bool player_handle_keys(float delta_t) {
    PDButtons pushed;
    PDButtons pressed;
    pd->system->getButtonState(&pushed, &pressed, NULL);

    if (pressed & kButtonB) {
        player_action_inventory(true);
        return false;
    }
    player_check_interaction();
    if (gs.player_interact) {
        action_t action = gs.player_interact_item->action;
        if (pressed & kButtonA) {
            switch (action.type) {
                case ACTION_NONE:
                    assert(0);
                    break;
                case ACTION_NOTE:
                    player_action_note(true);
                    break;
                case ACTION_KEYPAD:
                    player_action_keypad(true);
                    break;
                case ACTION_INSPECT:
                    player_action_inspect(true);
                    break;
            }
        }
    }

    // Player movement is pretty expensive!
    if (pushed == 0) return false;

    float old_bob = gs.camera.bob;

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
    } else if (pushed & kButtonDown) {
        // Walk backward
        speed = INPUT_CAMERA_TSPEEDB;
        bob_speed = HEADBOB_SPEED0;
        bob_range = HEADBOB_RANGE0;
    }

    if (speed != 0) {
        // Head bobbing
        gs.camera.bob = sinf(headbob_timer) * bob_range;
        headbob_timer += delta_t * bob_speed;

        // Moving!
        vec2 new_pos;
        move_to_dir(gs.camera.pos, gs.camera.front, speed * delta_t, new_pos);

        if (is_colliding(new_pos)) {
            ivec2 old_posi;
            ivec2 new_posi;
            ivec2 coll_normi;
            vec2 coll_normf;
            glm_ivec2_sub(new_posi, old_posi, coll_normi);

            // Collision vector is the perpendicular to the
            // tile coordinate direction
            vec2_to_ivec2(gs.camera.pos, old_posi);
            vec2_to_ivec2(new_pos, new_posi);
            glm_ivec2_sub(new_posi, old_posi, coll_normi);
            ivec2_to_vec2(coll_normi, coll_normf);

            // Calculate perpendicular vector
            glm_swapf(&coll_normf[0], &coll_normf[1]);
            coll_normf[0] *= -1.0;

            if (glm_vec2_dot(gs.camera.front, coll_normf) < 0.0f) {
                glm_vec2_scale_as(coll_normf, -1.0, coll_normf);
            }

            move_to_dir(gs.camera.pos, coll_normf, speed * delta_t, new_pos);
            if (!is_colliding(new_pos)) {
                glm_vec2_copy(new_pos, gs.camera.pos);
            }
        } else {
            glm_vec2_copy(new_pos, gs.camera.pos);
        }

        if (old_bob > 0 && gs.camera.bob < 0) {
            // Play the step sound
            if (step_sound) {
                sound_effect_play(SOUND_STEP0);
            } else {
                sound_effect_play(SOUND_STEP1);
            }
            step_sound ^= 1;
        }
    } else {
        // Not moving
        sound_effect_stop(SOUND_STEP0);
        sound_effect_stop(SOUND_STEP1);
    }

    // Look left/right
    if (pushed & kButtonRight) {
        gs.camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        gs.camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    // Update the direction
    vec2 direction;
    direction[0] = cosf(glm_rad(gs.camera.yaw));
    direction[1] = sinf(glm_rad(gs.camera.yaw));

    glm_vec2_normalize_to(direction, gs.camera.front);

    // Update camera
    camera_update_trans(&gs.camera);

    return true;
}

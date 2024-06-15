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
    t[1] = 59;
    ivec2_to_vec2_center(t, gs.camera.pos);
    map_viz_update(gs.map, gs.camera);
}

void player_action_note(bool show) {
    if (show) {
        gs.player_state = PLAYER_READING;
        sound_effect_play(SOUND_NOTE);
    } else {
        gs.player_state = PLAYER_ACTIVE;
        gs.player_interact_item->hidden = true;
        gs.player_interact = false;
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

void player_action_keypress(PDButtons pushed, PDButtons pushed_old, float delta_t) {
    (void)delta_t;
    if (pushed_old == 0) {
        if (pushed & kButtonUp) {
            if (gs.keypad_sel == 0) {
                gs.keypad_sel = 8;
            } else {
                gs.keypad_sel -= 3;
            }
        } else if (pushed & kButtonDown) {
            gs.keypad_sel += 3;
        } else if (pushed & kButtonRight) {
            gs.keypad_sel += 1;
        } else if (pushed & kButtonLeft) {
            gs.keypad_sel -= 1;
        } else if (pushed & kButtonA) {
            gs.keypad_val[gs.keypad_cnt] = gs.keypad_sel;
            gs.keypad_cnt++;
            if (gs.keypad_cnt < KEYPAD_PIN_SIZE) {
                sound_effect_play(SOUND_KEY);
            } else {
                int insert_pin = 0;
                // Check if pin is correct
                for (int i = 0; i < KEYPAD_PIN_SIZE; i++) {
                    insert_pin *= 10;
                    insert_pin += gs.keypad_val[i];
                }

                if (insert_pin == gs.player_interact_item->action.arg) {
                    sound_effect_play(SOUND_KEYPAD_CORRECT);
                    gs.player_interact_item->action.type = ACTION_NONE;
                    gs.player_interact_item->tex_id = TEX_FENCE_OPEN;
                    gs.player_interact = false;
                    sound_effect_play(SOUND_FENCE_OPEN);
                    player_action_keypad(false);
                } else {
                    sound_effect_play(SOUND_KEYPAD_WRONG);
                    gs.keypad_cnt = 0;
                }
            }
        } else if (pushed & kButtonB) {
            player_action_keypad(false);
        }

        if (gs.keypad_sel < 0) {
            gs.keypad_sel = 9;
        } else if (gs.keypad_sel > 9) {
            gs.keypad_sel = 0;
        }
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

static bool player_collide(vec2 pos) {
    // TODO: Make collisions not sticky!
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

void player_action_move(PDButtons pushed, float delta_t) {
    // Player movement is pretty expensive!
    if (pushed == 0) return;

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
    vec2 new_pos;
    glm_vec2_scale_as(gs.camera.front, speed * delta_t, camera_delta);

    // Do bobbing effect only when moving
    if (speed != 0) {
        headbob_timer += delta_t * bob_speed;
        gs.camera.bob = sinf(headbob_timer) * bob_range;
        glm_vec2_add(gs.camera.pos, camera_delta, new_pos);
    }

    if (pushed & kButtonRight) {
        gs.camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        gs.camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    if (!player_collide(new_pos)) {
        glm_vec2_copy(new_pos, gs.camera.pos);
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

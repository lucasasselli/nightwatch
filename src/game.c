#include "game.h"

#include "enemy.h"
#include "game_state.h"
#include "map.h"
#include "map_reader.h"
#include "pd_api.h"
#include "player.h"
#include "sound.h"
#include "utils.h"

// Torch
#define TORCH_DISCHARGE_RATE 0.2f
#define TORCH_CHARGE_RATE 0.01f  // Charge x deg x s

extern game_state_t gs;
extern PlaydateAPI* pd;

PDButtons pushed_old;

void game_reset(void) {
    // Camera
    camera_init(&gs.camera);

    // State
    player_reset();
    enemy_reset();

    // Setup map
    if (gs.map != NULL) {
        // If map is not NULL free
        map_free(gs.map);
    }
    gs.map = map_new();
    map_read(gs.map);
    map_viz_update(gs.map, gs.camera);

    gs.torch_charge = 0.0f;
}

static void handle_keys(PDButtons pushed, float delta_t) {
    player_check_interaction();

    if (gs.player_state == PLAYER_ACTIVE) {
        if (gs.player_interact && pushed_old == 0) {
            action_t action = gs.player_interact_item->action;
            if (pushed & kButtonA) {
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
        if (player_action_move(pushed, delta_t)) {
            // Update map visibility
            map_viz_update(gs.map, gs.camera);
        }
    } else if (gs.player_state == PLAYER_READING) {
        if (pushed & kButtonB) {
            player_action_note(false);
        }
    } else if (gs.player_state == PLAYER_KEYPAD) {
        player_action_keypress(pushed, pushed_old, delta_t);
    }

    pushed_old = pushed;
}

void action_torch_flicker(bool enable) {
    gs.torch_flicker = enable;
    if (enable) {
        sound_effect_start(SOUND_FLICKER);
    } else {
        sound_effect_stop(SOUND_FLICKER);
    }
}

void game_update(float delta_t) {
    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);
    handle_keys(pushed, delta_t);

    if (gs.player_state == PLAYER_ACTIVE) {
        // Torch
        float crank_delta = fabsf(pd->system->getCrankChange());
        if (crank_delta == 0) {
            gs.torch_charge += -TORCH_DISCHARGE_RATE * delta_t;
        } else {
            gs.torch_charge += crank_delta * TORCH_CHARGE_RATE * delta_t;
        }
        gs.torch_charge = glm_clamp(gs.torch_charge, 0.0f, 1.0f);

        // Should torch flicker?
        if (gs.torch_charge > 0.0f) {
            // Torch is on!
            if (gs.enemy_state == ENEMY_CHASING) {
                action_torch_flicker(true);
            } else if (gs.torch_charge > 0.0f && gs.enemy_state == ENEMY_DESPAWN) {
                action_torch_flicker(true);
            } else {
                action_torch_flicker(false);
            }
        } else {
            // Never flicker if torch is off!
            action_torch_flicker(false);
        }

        // Enemy
        enemy_update_state(delta_t);

        // FIXME: Only if player changes tile
        if (gs.enemy_state != ENEMY_HIDDEN) {
            enemy_update_path();
        }
    }
}

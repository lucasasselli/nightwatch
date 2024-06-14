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

void game_reset(void) {
    // Camera
    gs.camera.yaw = -90.0f;
    glm_vec2_copy((vec3){0.0f, -1.0f}, gs.camera.front);

    // State
    player_reset();
    enemy_reset();
    map_read(gs.map);

    gs.torch_charge = 0.0f;
}

static void handle_keys(PDButtons pushed, float delta_t) {
    player_check_interaction();

    if (gs.player_state == PLAYER_ACTIVE) {
        if (gs.player_interact) {
            if (pushed & kButtonA) {
                if (gs.player_interact_item->type == ITEM_NOTE) {
                    player_action_note(true);
                }
            }
        }
        player_action_move(pushed, delta_t);
    } else if (gs.player_state == PLAYER_READING) {
        if (pushed & kButtonB) {
            player_action_note(false);
        }
    }
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
    //---------------------------------------------------------------------------
    // Handle input
    //---------------------------------------------------------------------------

    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);

    if (pushed) {
        handle_keys(pushed, delta_t);
    }

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

#include "game.h"

#include "enemy.h"
#include "game_state.h"
#include "map.h"
#include "mapgen/mapgen.h"
#include "pd_api.h"
#include "player.h"
#include "sound.h"
#include "utils.h"

// Torch
#define TORCH_DISCHARGE_RATE 0.1f
#define TORCH_CHARGE_RATE 0.01f  // Charge x deg x s

extern game_state_t gs;
extern PlaydateAPI* pd;

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
    mapgen_gen(gs.map);
    map_viz_update(gs.map, gs.camera);

    gs.torch_charge = 0.0f;

    for (int i = 0; i < NOTES_CNT; i++) {
        gs.notes_found[i] = false;
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
    // Handle keys
    if (player_handle_keys(delta_t)) {
        // Update map visibility
        map_viz_update(gs.map, gs.camera);

        if (gs.enemy_state != ENEMY_HIDDEN) {
            enemy_update_path();
        }
    }

    if (gs.player_state == PLAYER_ACTIVE) {
        // Torch
        float crank_delta = pd->system->getCrankChange();
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
    }
}

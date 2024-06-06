#include "game.h"

#include "a_star.h"
#include "constants.h"
#include "map.h"
#include "sound.h"
#include "utils.h"

const int ENEMY_AWARE_SPAWN_THRESHOLD = 1000.0f;
const int ENEMY_AWARE_CHASE_THRESHOLD = 5000.0f;
const int ENEMY_AWARE_MAX = 6000.0f;

// Game state
a_star_path_t enemy_path;
int enemy_path_prog = 0;
float enemy_move_cnt = 0;
ivec2 enemy_player_last_pos;

int map_viz[MAP_SIZE][MAP_SIZE];

void rand_empty_tile(map_t map, ivec2 tile_pos) {
    map_tile_t tile;
    do {
        tile_pos[0] = rand() % MAP_SIZE;
        tile_pos[1] = rand() % MAP_SIZE;
        tile = map_get_tile_ivec2(map, tile_pos);
    } while (map_tile_collide(tile));
}

void game_init(void) {
    //---------------------------------------------------------------------------
    // Camera
    //---------------------------------------------------------------------------

    gs.camera.yaw = -90.0f;
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, gs.camera.front);

    //---------------------------------------------------------------------------
    // State
    //---------------------------------------------------------------------------

    // Pick a random starting position in the map
    rand_empty_tile(gs.map, gs.player_tile);
    ivec2_to_vec2_center(gs.player_tile, gs.camera.pos);
    map_viz_update(gs.map, gs.camera);

    // Torch
    gs.torch_charge = 0.0;

    // Enemy
    gs.enemy_state = ENEMY_RESET;
}

void game_handle_keys(PDButtons pushed, float delta_t) {
    (void)delta_t;  // TODO: make speed delta_t dependent

    vec2 camera_delta;

    vec2 old_pos;
    glm_vec2_copy(gs.camera.pos, old_pos);

    if (pushed & kButtonUp) {
        // Walks forward
        float speed = INPUT_CAMERA_TSPEED1;
        if (pushed & kButtonA) {
            speed = INPUT_CAMERA_TSPEED2;
        }
        glm_vec2_scale_as(gs.camera.front, speed, camera_delta);
        glm_vec2_add(gs.camera.pos, camera_delta, gs.camera.pos);
    } else if (pushed & kButtonDown) {
        // Walk backward
        glm_vec2_scale_as(gs.camera.front, INPUT_CAMERA_TSPEED1, camera_delta);
        glm_vec2_sub(gs.camera.pos, camera_delta, gs.camera.pos);
    }

    if (pushed & kButtonRight) {
        gs.camera.yaw += INPUT_CAMERA_RSPEED;
    }
    if (pushed & kButtonLeft) {
        gs.camera.yaw -= INPUT_CAMERA_RSPEED;
    }

    if (pushed & kButtonB) {
        // gs.minimap_show ^= 1;
    }

    //---------------------------------------------------------------------------
    // Collisions
    //---------------------------------------------------------------------------

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
}

void game_handle_crank(float delta_t) {
    (void)delta_t;  // TODO: Make crank rates delta_t dependent

    const float TORCH_DISCHARGE_RATE = 0.01f;
    const float TORCH_CHARGE_RATE = 0.001f;

    float crank_delta = fabsf(pd->system->getCrankChange());
    gs.torch_charge += -TORCH_DISCHARGE_RATE;
    gs.torch_charge += crank_delta * TORCH_CHARGE_RATE;
    gs.torch_charge = glm_clamp(gs.torch_charge, 0.0f, 1.0f);

    gs.torch_on = !pd->system->isCrankDocked();
}

void game_update(float delta_t) {
    const float ENEMY_AWARE_TORCH_K = 1.0f;
    const float ENEMY_AWARE_SIGHT_K = 50.0f;

    //---------------------------------------------------------------------------
    // Handle input
    //---------------------------------------------------------------------------

    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);

    if (pushed) {
        game_handle_keys(pushed, delta_t);
    }

    game_handle_crank(delta_t);

    //---------------------------------------------------------------------------
    // Update state
    //---------------------------------------------------------------------------

    // Is enemy visible?
    if (gs.enemy_state != ENEMY_HIDDEN && gs.torch_on) {
        gs.enemy_in_fov = map_viz_ivec2(gs.map, gs.camera.pos, gs.enemy_tile);
    } else {
        gs.enemy_in_fov = false;
    }

    // Flicker effect
    if (gs.enemy_state == ENEMY_CHASING) {
        gs.torch_flicker = true;
    } else {
        gs.torch_flicker = false;
    }

    if (gs.torch_flicker && gs.torch_on) {
        sound_effect_start(SOUND_FLICKER);
    } else {
        sound_effect_stop(SOUND_FLICKER);
    }

    // Update awareness
    if (gs.torch_on && gs.torch_charge > 0.0f && gs.enemy_state != ENEMY_CHASING) {
        // If torch is on, has charge and player is not being chased, update awareness
        if (gs.enemy_in_fov) {
            gs.enemy_awareness += delta_t * gs.torch_charge * ENEMY_AWARE_SIGHT_K;  // FIXME: make factor of distance
        } else {
            gs.enemy_awareness += delta_t * gs.torch_charge * ENEMY_AWARE_TORCH_K;
        }
    } else if (gs.enemy_state == ENEMY_CHASING) {
        // If enemy is chasing, awareness doesn't change.
    } else {
        // Slowly decrease awareness when torch is off
        gs.enemy_awareness -= delta_t;
    }

    gs.enemy_awareness = glm_clamp(gs.enemy_awareness, 0.0f, ENEMY_AWARE_MAX);

    // FIXME: Make this grow with the distance instead of awareness
    sound_play_range(SOUND_HEARTBEAT, 0, 120000 - 100000 * (gs.enemy_awareness / ENEMY_AWARE_MAX));

    game_enemy_ai(delta_t);
}

void enemy_path_move(float speed, float delta_t) {
    const float ENEMY_MOVE_THRESHOLD_MAX = 1000.0f;

    assert(speed <= ENEMY_MOVE_THRESHOLD_MAX);

    if (enemy_path_prog < enemy_path.size) {
        if (enemy_move_cnt > (ENEMY_MOVE_THRESHOLD_MAX - speed)) {
            glm_ivec2_copy(enemy_path.pos[enemy_path_prog++], gs.enemy_tile);
            debug("Enemy: %d %d", gs.enemy_tile[0], gs.enemy_tile[1]);
            enemy_move_cnt = 0.0f;
        } else {
            enemy_move_cnt += delta_t;
        }
    }
}

bool enemy_path_complete(void) {
    return !(enemy_path_prog < enemy_path.size);
}

void enemy_path_to_player(void) {
    debug("Enemy (CHASING): player moved to %d %d", gs.player_tile[0], gs.player_tile[1]);

    // Update path
    if (!glm_ivec2_eqv(enemy_player_last_pos, gs.player_tile)) {
        // Update path only if player has moved
        a_star_navigate(gs.map, gs.enemy_tile, gs.player_tile, &enemy_path);
        enemy_path_prog = 0;
    }

    // Store last known position
    glm_ivec2_copy(gs.player_tile, enemy_player_last_pos);
}

void game_enemy_ai(float delta_t) {
    const float ENEMY_ROAMING_SPEED = 200.0f;
    const float ENEMY_CHASING_SPEED = 400.0f;

    switch (gs.enemy_state) {
        // Dummy state to initialize FSM
        case ENEMY_RESET:
            gs.enemy_awareness = 0.0f;
            gs.enemy_state = ENEMY_HIDDEN;
            break;

        // Enemy not spawned in map
        case ENEMY_HIDDEN:
            if (gs.enemy_awareness > ENEMY_AWARE_SPAWN_THRESHOLD) {
                // FIXME: Spawn out of sight an far from player
                rand_empty_tile(gs.map, gs.enemy_tile);
                gs.enemy_state = ENEMY_ROAMING;
                debug("Enemy (ROAMING): spawned at %d %d!", gs.enemy_tile[0], gs.enemy_tile[1]);
            }
            break;

        // Enemy navigating map, not actively searching player
        case ENEMY_ROAMING:
            if (!gs.enemy_in_fov) {
                // Enemy not in the FOV of the player
                enemy_path_move(ENEMY_ROAMING_SPEED, delta_t);

                if (enemy_path_complete()) {
                    ivec2 target_tile;
                    rand_empty_tile(gs.map, target_tile);
                    a_star_navigate(gs.map, gs.enemy_tile, target_tile, &enemy_path);
                    enemy_path_prog = 0;
                    debug("Enemy (ROAMING): path complete! Begin new path...");
                }
            } else {
                // Enemy seen by the player!
                gs.enemy_state = ENEMY_ALERT;
                sound_effect_play(SOUND_DISCOVERED);
            }
            break;

        // Enemy spotted by player, preparing to chase
        case ENEMY_ALERT:
            if (gs.enemy_awareness > ENEMY_AWARE_CHASE_THRESHOLD) {
                gs.enemy_state = ENEMY_CHASING;
            }
            break;

        // Enemy actively chaing player
        case ENEMY_CHASING:
            if (gs.torch_on) {
                enemy_path_to_player();
            }

            enemy_path_move(ENEMY_CHASING_SPEED, delta_t);

            if (enemy_path_complete()) {
                // Reached last known position
                if (glm_ivec2_eqv(gs.enemy_tile, gs.player_tile)) {
                    // GAME OVER!
                    debug("Enemy (CHASING): player reached!");
                } else {
                    // Begin search
                    gs.enemy_state = ENEMY_SEARCHING;
                    debug("Enemy (SEARCHING): player lost!");
                }
            }
            break;

        // Enemy actively chasing player
        case ENEMY_SEARCHING:
            if (gs.torch_on) {
                debug("Enemy (CHASING): player visible!");
                gs.enemy_state = ENEMY_CHASING;
                enemy_path_to_player();
            } else if (gs.enemy_awareness < ENEMY_AWARE_SPAWN_THRESHOLD) {
                gs.enemy_state = ENEMY_HIDDEN;
                debug("Enemy (HIDDEN)");
            }
            break;
    }
}

#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "map_generator.h"
#include "map_renderer.h"
#include "minigl.h"
#include "pd_api.h"
#include "pd_system.h"
#include "sound.h"
#include "utils.h"

#define SETUP_STEPS 5

// API handle
PlaydateAPI *pd;

// Support
int setup_step = 0;
float update_delta_t = 0.0f;
int update_cnt = 0;

// Game state
game_state_t gs;
mat4 proj;
mat4 trans;

// Resources
LCDBitmap *bg_loading;

minigl_tex_t tex_dither;
minigl_tex_t tex_enemy;
minigl_obj_t obj_enemy;

minigl_objbuf_t obj_buf;

// #define TORCH_DISABLE

#define TORCH_MASK_STEPS 32
fp16_t torch_mask[TORCH_MASK_STEPS][SCREEN_SIZE_Y][SCREEN_SIZE_X];

void view_update(void) {
    // Update camera position
    vec3 camera_center;
    mat4 view;
    glm_vec3_add(gs.player_camera.pos, gs.player_camera.front, camera_center);
    glm_lookat(gs.player_camera.pos, camera_center, gs.player_camera.up, view);
    glm_mat4_mul(proj, view, trans);
}

// FIXME: Write directly on the screen buffer?
void screen_update(void) {
    // Offset to center in screen
    const int X_OFFSET = (LCD_COLUMNS - SCREEN_SIZE_X) / 2;

    // Handle flickering
    float torch_intensity = gs.torch_charge;
    if (gs.torch_on) {
        if (gs.torch_flicker) {
            torch_intensity = glm_clamp(rand_range(0, 100) - 50, 0.0f, 50.0f) / 50.0f;
        }
    } else {
        torch_intensity = 0.0f;
    }

    int torch_mask_i = (TORCH_MASK_STEPS - 1) * torch_intensity;

    // FIXME: This is garbage! 0 means fully back!
    const float TORCH_FADE_Z = 0.60f + 0.35f * torch_intensity;
    const float TORCH_FADE_Z_K = 1.0f / (1.0f - TORCH_FADE_Z);

    uint8_t *pd_frame = pd->graphics->getFrame();
    minigl_frame_t *minigl_frame = minigl_get_frame();

    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            uint8_t color = minigl_frame->c_buff[y][x];

#ifndef TORCH_DISABLE
            float m = torch_mask[torch_mask_i][y][x];

            if (torch_intensity == 0.0f || m == 0.0f) {
                color = 0;
            } else if (color > 0.0) {
                // If pixel is already fully back, don't bother
                float z = minigl_frame->z_buff[y][x];

                if (z > TORCH_FADE_Z) {
                    color = ((float)color) * (1.0f - z) * TORCH_FADE_Z_K;
                }

                color *= m;

                color = (color >= tex_dither.color[y & 0x0000001F][x & 0x0000001F]);
            }
#else
            color = (color >= tex_dither.color[y & 0x0000001F][x & 0x0000001F]);
#endif

            if (color) {
                clearpixel(pd_frame, X_OFFSET + x, SCREEN_SIZE_Y - y - 1, LCD_ROWSIZE);
            } else {
                setpixel(pd_frame, X_OFFSET + x, SCREEN_SIZE_Y - y - 1, LCD_ROWSIZE);
            }
        }
    }
    pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
}

void minigl_perf_print(void) {
#ifdef DEBUG_PERF
    minigl_perf_data_t perf_data = minigl_perf_get();
    debug("Clip count: %d", perf_data.clip);
    debug("Cull count: %d", perf_data.cull);
    debug("Poly count: %d", perf_data.poly);
    debug("Frag count: %d", perf_data.frag);
#endif
}

void gen_torch_mask() {
    // Torch mask
    for (int i = 0; i < TORCH_MASK_STEPS; i++) {
        float torch_intensity = (((float)i) / ((float)TORCH_MASK_STEPS - 1));
        float torch_fade_r = 100.0f * torch_intensity;
        float torch_black_r = 120.0f - 20.0f * (1.0f - torch_intensity);

        for (int y = 0; y < SCREEN_SIZE_Y; y++) {
            for (int x = 0; x < SCREEN_SIZE_X; x++) {
                float r = sqrtf(pow((x - SCREEN_SIZE_X / 2), 2) + pow((y - SCREEN_SIZE_Y / 2), 2));

                float color;

                if (r > torch_black_r) {
                    // If pixel is outside torch radius
                    color = 0.0f;
                } else if (r > torch_fade_r && r <= torch_black_r) {
                    // Fade
                    color = (torch_black_r - r) / (torch_black_r - torch_fade_r);
                } else {
                    // Full brightness
                    color = 1.0f;
                }

                torch_mask[i][y][x] = color;
            }
        }
    }
}

void setup(int step) {
    debug("Setup %d", step);

    switch (step) {
        case 0:
            // Load textures
            minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);
            minigl_tex_read_file("res/textures/test.tex", &tex_enemy);

            // Config dither texture
            minigl_set_dither(tex_dither);

            // Object buffer
            obj_buf = minigl_objbuf_init(50);

            // Enemy
            minigl_obj_read_file("res/models/tile.obj", &obj_enemy);
            glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
            glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
            glm_scale_uni(trans, MAP_TILE_SIZE);
            minigl_obj_trans(&obj_enemy, trans);
            break;

        case 1:
            gen_torch_mask();
            break;

        case 2:
            // Setup map
            mapgen_gen(gs.map);
            mapgen_grid_print(gs.map);
            map_init();
            break;

        case 3:
            // Setup minimap
            minimap_init();
            minimap_gen(gs.map);

            // Initialize game
            game_init(&gs);

            glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 1.0f, 50.0f, proj);
            view_update();  // Setup view matrix

            // Sound
            sound_init();
            // sound_bg_start();
            sound_effect_start(SOUND_HEARTBEAT);
            break;

        case 4:
            break;
    }
}

static int update(void) {
    //---------------------------------------------------------------------------
    // Handle input
    //---------------------------------------------------------------------------

    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);

    if (pushed) {
        game_handle_keys(&gs, pushed, update_delta_t);
        view_update();
    }

    game_handle_crank(&gs, update_delta_t);

    game_update(&gs, update_delta_t);

    game_enemy_ai(&gs, update_delta_t);

    //---------------------------------------------------------------------------
    // Draw graphics
    //---------------------------------------------------------------------------

    // Flush buffer
    minigl_clear(0.0f, 1.0f);

    // Draw map
    map_draw(gs.map, trans, gs.player_camera);

    // Draw enemy
    if (gs.enemy_state != ENEMY_HIDDEN) {
        mat4 enemy_trans = GLM_MAT4_IDENTITY_INIT;
        vec3 enemy_pos;
        pos_tile_to_world(gs.enemy_tile, enemy_pos);
        glm_translate(enemy_trans, enemy_pos);
        mat4_billboard(gs.player_camera, enemy_trans);
        glm_mat4_mul(trans, enemy_trans, enemy_trans);
        minigl_obj_to_objbuf_trans(obj_enemy, enemy_trans, &obj_buf);
        minigl_set_tex(tex_enemy);
        minigl_draw(obj_buf);
    }

    // Not used
    if (gs.minimap_show) {
        minimap_draw(0, 0, &gs);
    }

    // Update the screen
    screen_update();

#ifdef DEBUG
    if (update_cnt++ % 100 == 0) {
        minigl_perf_print();
        debug("Awareness: %f", gs.enemy_awareness);
    }
    minigl_perf_clear();

    pd->system->drawFPS(0, 0);
#endif

    return 1;
}

static int pd_update_callback(void *userdata) {
    // Calculate delta T
    update_delta_t = pd->system->getElapsedTime() - update_delta_t;

    // Do stuff
    if (setup_step < SETUP_STEPS) {
        // Load resources
        if (setup_step == 0) {
            pd->graphics->drawBitmap(bg_loading, 0, 0, 0);
        }

        setup(setup_step);
        setup_step++;

        pd->graphics->fillRect(100, 160, setup_step * (200 / SETUP_STEPS), 5, kColorWhite);

        if (setup_step == SETUP_STEPS) {
            pd->graphics->clear(kColorBlack);
        }
    } else {
        // Real work is done here!
        update();
    }

    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *_pd, PDSystemEvent event, uint32_t arg) {

    if (event == kEventInit) {
        pd = _pd;

        //---------------------------------------------------------------------------
        // Device config
        //---------------------------------------------------------------------------

        pd->system->setUpdateCallback(pd_update_callback, pd);
        pd->display->setRefreshRate(30);
        pd->graphics->clear(kColorBlack);

        // int seed = time(NULL);
        int seed = 0;
        debug("SEED: %d", seed);
        srand(seed);

        bg_loading = pd->graphics->loadBitmap("res/images/loading.pdi", NULL);
    }

    return 0;
}

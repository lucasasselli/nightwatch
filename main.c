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
#include "utils.h"

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

// Constants
const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";

// API handle
PlaydateAPI *pd;

// Game state
game_state_t gs;
mat4 proj;
mat4 trans;

// Resources
LCDFont *font = NULL;
minigl_tex_t tex_dither;
minigl_tex_t tex_enemy;
minigl_obj_t obj_enemy;

minigl_obj_buf_t obj_buf;

unsigned int update_cnt = 0;

// #define TORCH_DISABLE

float frame_radius[SCREEN_SIZE_Y][SCREEN_SIZE_X];

void view_update(void) {
    // Update camera poistion
    vec3 camera_center;
    mat4 view;
    glm_vec3_add(gs.player_camera.pos, gs.player_camera.front, camera_center);
    glm_lookat(gs.player_camera.pos, camera_center, gs.player_camera.up, view);
    glm_mat4_mul(proj, view, trans);
}

// FIXME: Write directly on the screen buffer?
void screen_update(void) {
    const int X_OFFSET = (LCD_COLUMNS - SCREEN_SIZE_X) / 2;

    // FIXME: This is garbage! 0 means fully back!
    const float TORCH_FADE_Z = 0.60f + 0.35f * gs.torch_charge;
    const float TORCH_FADE_Z_K = 1.0f / (1.0f - TORCH_FADE_Z);
    float TORCH_FADE_R = 100.0f * gs.torch_charge;
    float TORCH_BLACK_R = gs.torch_on ? 120.0f - 20.0f * (1.0f - gs.torch_charge) : 0.0f;
    float TORCH_FADE_K = 1.0f / (TORCH_BLACK_R - TORCH_FADE_R);

    uint8_t *pd_frame = pd->graphics->getFrame();
    minigl_frame_t *minigl_frame = minigl_get_frame();

    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            uint8_t color = minigl_frame->c_buff[y][x];

#ifndef TORCH_DISABLE
            float r = frame_radius[y][x];

            if (r > TORCH_BLACK_R) {
                // If pixel is outside torch radius
                color = 0;
            } else if (color > 0.0 || r > TORCH_BLACK_R) {
                // If pixel is already fully back, don't bother
                float z = minigl_frame->z_buff[y][x];

                if (z > TORCH_FADE_Z) {
                    color = ((float)color) * (1.0f - z) * TORCH_FADE_Z_K;
                }

                if (r > TORCH_FADE_R && r <= TORCH_BLACK_R) {
                    color *= (TORCH_BLACK_R - r) * TORCH_FADE_K;
                } else if (r > TORCH_BLACK_R) {
                    color = 0;
                }

                // FIXME:
                color = (color >= tex_dither.color[y & 0x0000001F][x & 0x0000001F]);
            }
#else
            color = (color >= tex_dither.color[y % tex_dither.size_y][x % tex_dither.size_x]);
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
    minigl_perf_data_t perf_data = minigl_perf_get();
#ifdef DEBUG_PERF
    debug("Clip count: %d", perf_data.clip);
    debug("Cull count: %d", perf_data.cull);
    debug("Poly count: %d", perf_data.poly);
    debug("Frag count: %d", perf_data.frag);
#endif
}

static int update(void *userdata) {
    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);

    if (pushed) {
        game_handle_keys(&gs, pushed);
        view_update();
    }

    game_handle_crank(&gs);

    if (update_cnt % 100 == 0) {
        game_handle_enemy(&gs);
    }

    meas_time_start(0);

    if (gs.minimap_show) {
        meas_time_start(4);
        minimap_draw((LCD_COLUMNS - SCREEN_SIZE_X) / 2, 0, gs.player_camera);
        meas_time_stop(4);
    } else {
        meas_time_start(1);
        minigl_clear(0.0f, 1.0f);
        meas_time_stop(1);

        // Draw map
        meas_time_start(2);
        map_draw(gs.map, trans, gs.player_camera);
        meas_time_stop(2);

        // Draw enemy
        if (gs.enemy_state != ENEMY_HIDDEN) {
            mat4 enemy_trans = GLM_MAT4_IDENTITY_INIT;
            glm_translate(enemy_trans, gs.enemy_camera.pos);
            mat4_billboard(gs.player_camera, enemy_trans);
            glm_mat4_mul(trans, enemy_trans, enemy_trans);
            minigl_obj_to_obj_buf_trans(obj_enemy, enemy_trans, &obj_buf);
            minigl_set_tex(tex_enemy);
            minigl_draw(obj_buf);
        }

        // Update the screen
        meas_time_start(3);
        screen_update();
        meas_time_stop(3);
    }

    meas_time_stop(0);

#ifdef DEBUG_PERF
    if (update_cnt % 100 == 0) {
        meas_time_print(1, "Buffer flush ");
        meas_time_print(2, "Map draw     ");
        meas_time_print(3, "Screen update");
        meas_time_print(4, "Minimap draw ");
        meas_time_print(0, "Full         ");
        minigl_perf_print();
    }
    minigl_perf_clear();
#endif

    update_cnt++;

#ifdef DEBUG
    pd->system->drawFPS(0, 0);
#endif

    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *_pd, PDSystemEvent event, uint32_t arg) {

    if (event == kEventInit) {
        pd = _pd;

        debug("Begin setup...");

        //---------------------------------------------------------------------------
        // Device config
        //---------------------------------------------------------------------------

        pd->system->setUpdateCallback(update, pd);
        pd->display->setRefreshRate(30);
        pd->graphics->clear(kColorBlack);

        //---------------------------------------------------------------------------
        // Game resources
        //---------------------------------------------------------------------------

        // Load textures
        minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);
        minigl_tex_read_file("res/textures/test.tex", &tex_enemy);

        // Object buffer
        obj_buf = minigl_obj_buf_init(50);

        // Enemy
        minigl_obj_read_file("res/models/tile.obj", &obj_enemy);
        glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
        glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
        glm_translate(trans, (vec3){0.0f, 0.0f, 0.0f});
        glm_scale_uni(trans, MAP_TILE_SIZE);
        minigl_obj_copy_trans(obj_enemy, trans, &obj_enemy);

        // Load fonts
        const char *err;
        font = pd->graphics->loadFont(fontpath, &err);

        if (font == NULL) {
            pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);
        }
        pd->graphics->setFont(font);

        //---------------------------------------------------------------------------
        // Precalculate stuff
        //---------------------------------------------------------------------------

        for (int y = 0; y < SCREEN_SIZE_Y; y++) {
            for (int x = 0; x < SCREEN_SIZE_X; x++) {
                frame_radius[y][x] = sqrtf(pow((x - SCREEN_SIZE_X / 2), 2) + pow((y - SCREEN_SIZE_Y / 2), 2));
            }
        }

        //---------------------------------------------------------------------------
        // Game config
        //---------------------------------------------------------------------------

        // int seed = time(NULL);
        int seed = 0;
        debug("SEED: %d", seed);
        srand(seed);

        minigl_set_dither(tex_dither);

        // Setup map
        mapgen_gen(gs.map);
        map_init();

        // Setup minimap
        minimap_init();
        minimap_gen(gs.map);

        // Initialize game
        game_init(&gs);

        glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 1.0f, 50.0f, proj);
        view_update();  // Setup view matrix

        debug("Setup complete!");
    }

    return 0;
}

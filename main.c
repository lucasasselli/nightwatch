#include <cglm/cglm.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "game.h"
#include "map_reader.h"
#include "map_renderer.h"
#include "minigl/minigl.h"
#include "minimap_renderer.h"
#include "pd_api.h"
#include "pd_system.h"
#include "sound.h"
#include "utils.h"

#define LOAD_STEP_CNT 5

// API handle
PlaydateAPI *pd;

// Support
int load_step = 0;

float update_et_last = 0.0f;
int update_cnt = 0;

game_state_t gs;

// Resources
minigl_tex_t tex_dither;
minigl_tex_t tex_enemy;
minigl_obj_t obj_enemy;

minigl_objbuf_t obj_buf;

// Transforms
mat4 proj;
mat4 trans;

// #define TORCH_DISABLE
#define TORCH_INT_STEPS 32
#define TORCH_FADE_STEPS 64

uint8_t torch_mask[TORCH_INT_STEPS][SCREEN_SIZE_Y][SCREEN_SIZE_X];
fp16_t torch_fade[TORCH_INT_STEPS][TORCH_FADE_STEPS];

// FIXME: Write directly on the screen buffer?
void screen_update(void) {
    // Offset to center in screen
    const int X_OFFSET = (LCD_COLUMNS - SCREEN_SIZE_X) / 2;

#ifndef TORCH_DISABLE

    // Handle flickering
    float torch_intensity = gs.torch_charge;
    if (gs.torch_on) {
        if (gs.torch_flicker) {
            torch_intensity = glm_clamp(rand_range(0, 100) - 50, 0.0f, 50.0f) / 50.0f;
        }
    } else {
        torch_intensity = 0.0f;
    }

    int torch_mask_i = (TORCH_INT_STEPS - 1) * torch_intensity;

#endif

    uint8_t *pd_frame = pd->graphics->getFrame();
    minigl_frame_t *minigl_frame = minigl_get_frame();

    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            uint8_t color = minigl_frame->c_buff[y][x];

#ifndef TORCH_DISABLE
            uint8_t m = torch_mask[torch_mask_i][y][x];

            if (m) {
                if (color > 0) {
                    // If pixel is already fully back, don't bother
                    float z = minigl_frame->z_buff[y][x];
                    // FIXME: There are Nan! Start fixing things Luca!!!
                    if (z > 0.0f && z < 1.0f) {
                        int torch_fade_i = (TORCH_FADE_STEPS - 1) * z;
                        color *= torch_fade[torch_mask_i][torch_fade_i];
                        color = (color >= tex_dither.color[y & 0x0000001F][x & 0x0000001F]);
                    }
                }
            } else {
                color = 0;
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
#ifdef MINIGL_DEBUG_PERF
    minigl_perf_data_t perf_data = minigl_perf_get();
    debug("Clip count: %d", perf_data.clip);
    debug("Cull count: %d", perf_data.cull);
    debug("Poly count: %d", perf_data.poly);
    debug("Frag count: %d", perf_data.frag);
#endif
}

void gen_torch_mask(void) {
    // Torch mask
    for (int i = 0; i < TORCH_INT_STEPS; i++) {
        float intensity = ((float)i) / ((float)TORCH_INT_STEPS - 1);

        // Overlay
        const float TORCH_FADE_R = 60.0f * intensity;
        const float TORCH_BLACK_R = 100 + 20.0f * intensity;

        for (int y = 0; y < SCREEN_SIZE_Y; y++) {
            for (int x = 0; x < SCREEN_SIZE_X; x++) {
                int dx = x - SCREEN_SIZE_X / 2;
                int dy = y - SCREEN_SIZE_Y / 2;
                float r = sqrtf(pow(dx, 2) + pow(dy, 2));  // FIXME: WTF? No error?

                uint8_t color = 255;

                if (i == 0) {
                    // Torch is dead
                    color = 0;
                } else if (r > TORCH_BLACK_R) {
                    // If pixel is outside torch radius
                    color = 0;
                } else if (r >= TORCH_FADE_R) {
                    // Fade
                    color *= (TORCH_BLACK_R - r) / (TORCH_BLACK_R - TORCH_FADE_R);
                }

                if (color > 0) {
                    color = (color >= tex_dither.color[y & 0x0000001F][x & 0x0000001F]);
                }
                torch_mask[i][y][x] = color;
            }
        }

        // Fade values
        const float TORCH_MIN_Z = CAMERA_MIN_Z;
        const float TORCH_MAX_Z = CAMERA_MAX_Z * intensity;

        for (int j = 0; j < TORCH_FADE_STEPS; j++) {
            float z = ((float)j) / TORCH_FADE_STEPS;
            float lin_z = (2 * TORCH_MIN_Z * TORCH_MAX_Z) / (TORCH_MAX_Z + TORCH_MIN_Z - z * (TORCH_MAX_Z - TORCH_MIN_Z));
            if (lin_z < TORCH_MAX_Z) {
                torch_fade[i][j] = (TORCH_MAX_Z - lin_z) / (TORCH_MAX_Z - TORCH_MIN_Z);
            } else {
                torch_fade[i][j] = 0;
            }
        }
    }
}

void view_trans_update(void) {
    // Update camera position
    vec2 camera_center;
    mat4 view;
    glm_vec2_add(gs.camera.pos, gs.camera.front, camera_center);
    glm_lookat(CAMERA_VEC3(gs.camera.pos), CAMERA_VEC3(camera_center), CAMERA_UP, view);
    glm_mat4_mul(proj, view, trans);
}

static int lua_load(lua_State *L) {
    (void)L;  // Unused

    if (load_step < LOAD_STEP_CNT) {
        debug("Loading %d...", load_step);

        switch (load_step) {
            case 0:
                // Load textures
                minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);
                minigl_tex_read_file("res/textures/test.tex", &tex_enemy);

                // Config dither texture
                minigl_set_dither(tex_dither);

                // Object buffer
                obj_buf = minigl_objbuf_init(200);

                // Perspective transform
                glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), CAMERA_MIN_Z, CAMERA_MAX_Z, proj);
                view_trans_update();  // Setup view matrix

                // Enemy model
                minigl_obj_read_file("res/models/tile.obj", &obj_enemy, 0);
                glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
                glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
                minigl_obj_trans(&obj_enemy, trans);

                break;

            case 1:
                gen_torch_mask();
                break;

            case 2:
                // Setup map
                map_read(gs.map);
                map_init();
                break;

            case 3:
                // Setup minimap
                minimap_init();
                minimap_gen(gs.map);

                // Initialize game
                game_init();

                // Sound
                sound_init();
                // sound_bg_start();
                sound_effect_start(SOUND_HEARTBEAT);
                break;

            case 4:
                break;
        }
        load_step++;
    } else {
        debug("Loading done!");
    }

    pd->lua->pushInt(load_step);
    pd->lua->pushInt(LOAD_STEP_CNT);
    return 2;
}

static int lua_update(lua_State *L) {
    (void)L;  // Unused

    // Calculate delta T
    float update_et_this = pd->system->getElapsedTime();
    float update_delta_t = update_et_this - update_et_last;
    update_et_last = update_et_this;

    // Real work is done here!
    game_update(update_delta_t);
    view_trans_update();

    //---------------------------------------------------------------------------
    // Draw graphics
    //---------------------------------------------------------------------------

    // Flush buffer
    minigl_clear(0.0f, 1.0f);

#ifdef DEBUG_MINIMAP
    pd->graphics->clear(kColorBlack);
#endif

    // Draw map
    map_draw(gs.map, trans, gs.camera);

    // Draw enemy
    if (gs.enemy_state != ENEMY_HIDDEN) {
        // FIXME: This is drawn indipendently to the raycasting viz!
        mat4 enemy_trans = GLM_MAT4_IDENTITY_INIT;
        vec2 enemy_pos;
        ivec2_to_vec2_center(gs.enemy_tile, enemy_pos);
        glm_translate(enemy_trans, CAMERA_VEC3(enemy_pos));
        mat4_billboard(gs.camera, enemy_trans);
        glm_mat4_mul(trans, enemy_trans, enemy_trans);

        minigl_obj_to_objbuf_trans(obj_enemy, enemy_trans, &obj_buf);
        minigl_set_tex(tex_enemy);
        minigl_draw(obj_buf);
    }

    // Update the screen
    screen_update();

#ifdef DEBUG
#ifdef DEBUG_MINIMAP
    minimap_debug_draw(0, 0, &gs);
#endif

    // Print periodically
    if (update_cnt++ % 100 == 0) {
        minigl_perf_print();
        debug("Awareness: %f", (double)gs.enemy_awareness);
    }
    minigl_perf_clear();

    pd->system->drawFPS(0, 0);
#endif

    return 0;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *_pd, PDSystemEvent event, uint32_t arg) {

    (void)arg;  // Unused

    const char *err;

    switch (event) {
        case kEventInitLua:
            //---------------------------------------------------------------------------
            // LUA Init
            //---------------------------------------------------------------------------

            if (!pd->lua->addFunction(lua_load, "loadC", &err)) {
                pd->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
            }
            if (!pd->lua->addFunction(lua_update, "updateC", &err)) {
                pd->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
            }
            break;

        case kEventInit:
            //---------------------------------------------------------------------------
            // Init
            //---------------------------------------------------------------------------

            pd = _pd;

            // Configure device
            pd->display->setRefreshRate(30);

            int seed = time(NULL);
            // int seed = 0;
            debug("SEED: %d", seed);
            srand(seed);
            break;

        case kEventTerminate:
            // Cleanup!
            break;

        default:
            break;
    }

    return 0;
}

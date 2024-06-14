#include <cglm/cglm.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "game.h"
#include "map_reader.h"
#include "map_renderer.h"
#include "minigl/minigl.h"
#include "minimap_renderer.h"
#include "notes.h"
#include "pd_api.h"
#include "pd_system.h"
#include "random.h"
#include "sound.h"
#include "utils.h"

#define LOAD_STEP_CNT 5

// #define TORCH_DISABLE
#define TORCH_INT_STEPS 32
#define TORCH_FADE_STEPS 512

// API handle
PlaydateAPI *pd;

//---------------------------------------------------------------------------
// Resources
//---------------------------------------------------------------------------

// Textures
minigl_tex_t tex_dither;
minigl_tex_t tex_enemy;

// Images
LCDBitmap *img_gameover;

// Objects
minigl_obj_t obj_enemy;

// Object buffer
minigl_objbuf_t obj_buf;

// Fonts
LCDFont *font_system;
LCDFont *font_note;

// Precalculated
// TODO: Generate at compile time with Cog?
uint8_t torch_mask[TORCH_INT_STEPS][SCREEN_SIZE_Y][SCREEN_SIZE_X];
fp16_t torch_fade[TORCH_INT_STEPS][TORCH_FADE_STEPS];

// clang-format off
RANDW_CONSTR_BEGIN(float,CONSTR_FLICKER,2)
    RANDW_POINT(0.0, 8),
    RANDW_RANGE(0.0, 1.0, 2)
RANDW_CONSTR_END;
// clang-format on

//---------------------------------------------------------------------------
// Global vars
//---------------------------------------------------------------------------

int load_step = 0;
float update_et_last = 0.0f;
int update_cnt = 0;
int gameover_time = 0;

game_state_t gs;

// Transforms
mat4 proj;
mat4 trans;

void screen_update(void) {
    // Offset to center in screen
    const int X_OFFSET = (LCD_COLUMNS - SCREEN_SIZE_X) / 2;

#ifndef TORCH_DISABLE

    // Handle flickering
    float torch_intensity = gs.torch_charge;
    if (gs.torch_charge > 0.0f) {
        if (gs.torch_flicker) {
            torch_intensity = randwf(&CONSTR_FLICKER, 0.1);
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
                    int torch_fade_i = (TORCH_FADE_STEPS - 1) * z;
                    color *= torch_fade[torch_mask_i][torch_fade_i];
                    color = (color >= tex_dither.color[y & 0x0000001F][x & 0x0000001F]);
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

void print_perf(void) {
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

        //---------------------------------------------------------------------------
        // Overlay
        //---------------------------------------------------------------------------

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

        //---------------------------------------------------------------------------
        // Fade with Z
        //---------------------------------------------------------------------------

        // Fade linearly with the distance after a certain threshold (fade_z), based
        // on intensity using a linear spline.

        // NOTE: These thresholds are completely empirical
        const float TORCH_S0_I = 1.0f;
        const float TORCH_S0_K = 0.99f;

        const float TORCH_S1_I = 0.1f;
        const float TORCH_S1_K = 0.8f;

        const float TORCH_S2_I = 0.0f;
        const float TORCH_S2_K = 0.0f;

        float fade_z;
        if (intensity > TORCH_S1_I) {
            fade_z = TORCH_S1_K + (intensity - TORCH_S1_I) * (TORCH_S0_K - TORCH_S1_K) / (TORCH_S0_I - TORCH_S1_I);
        } else if (intensity > TORCH_S2_I) {
            fade_z = TORCH_S2_K + (intensity - TORCH_S2_I) * (TORCH_S1_K - TORCH_S2_K) / (TORCH_S1_I - TORCH_S2_I);
        } else {
            fade_z = 0.0f;
        }

        for (int j = 0; j < TORCH_FADE_STEPS; j++) {
            float z = ((float)j) / (float)(TORCH_FADE_STEPS - 1);
            if (z > fade_z) {
                // Fade to black (far)
                torch_fade[i][j] = 1.0f - (z - fade_z) / (1.0f - fade_z);
            } else {
                // Fully lit (near)
                torch_fade[i][j] = 1.0f;
            }
        }
    }
}

void view_trans_update(void) {
    mat4 view;
    vec3 camera_center;

    vec3 vec3_pos;
    vec3_pos[0] = gs.camera.pos[0];
    vec3_pos[1] = gs.camera.bob;
    vec3_pos[2] = gs.camera.pos[1];

    vec3 vec3_front;
    vec3_front[0] = gs.camera.front[0];
    vec3_front[1] = 0.0f;
    vec3_front[2] = gs.camera.front[1];

    glm_vec3_add(vec3_pos, vec3_front, camera_center);

    // Update camera position

    glm_vec3_add(vec3_pos, vec3_front, camera_center);
    glm_lookat(vec3_pos, camera_center, CAMERA_UP, view);
    glm_mat4_mul(proj, view, trans);
}

static int lua_reset(lua_State *L) {
    (void)L;  // Unused
    game_init();
    return 0;
}

static int lua_load(lua_State *L) {
    (void)L;  // Unused

    if (load_step < LOAD_STEP_CNT) {
        debug("Loading %d...", load_step);

        switch (load_step) {
            case 0:
                // Sound
                sound_init();
                break;

            case 1:
                // Load Fonts
                font_system = pd->graphics->loadFont("/System/Fonts/Asheville-Sans-14-Bold.pft", NULL);
                font_note = pd->graphics->loadFont("res/fonts/handwriting.pft", NULL);

                // Load textures
                minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);
                minigl_tex_read_file("res/textures/monster_idle.tex", &tex_enemy);

                // Load image
                img_gameover = pd->graphics->loadBitmap("res/images/gameover.pdi", NULL);

                // Config dither texture
                minigl_set_dither(tex_dither);

                // Object buffer
                obj_buf = minigl_objbuf_init(200);

                // Perspective transform
                glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), CAMERA_MIN_Z, CAMERA_MAX_Z, proj);
                view_trans_update();  // Setup view matrix

                // Enemy model
                minigl_obj_read_file("res/models/tile.obj", &obj_enemy, MINIGL_OBJ_TEXFLIPY);
                glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
                glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
                minigl_obj_trans(&obj_enemy, trans);

                break;

            case 2:
                gen_torch_mask();
                break;

            case 3:
                map_renderer_init();
                break;

            case 4:
                // Setup map
                map_read(gs.map);

#ifdef DEBUG_MINIMAP
                // Setup minimap
                minimap_init();
#endif

                // Initialize game
                game_init();
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

void show_prompt(void) {
    pd->graphics->setFont(font_system);
    pd->graphics->setDrawMode(kDrawModeNXOR);
    const char *msg = "\xE2\x93\x90 Read";
    pd->graphics->drawText(msg, strlen(msg), kUTF8Encoding, 180, 160);
}

void show_game(float delta_t) {
    // Real work is done here!
    game_update(delta_t);

    view_trans_update();

    // Flush buffer
    minigl_clear(0.0f, 1.0f);

#ifdef DEBUG_MINIMAP
    pd->graphics->clear(kColorBlack);
#endif

    // Draw map
    map_renderer_draw(gs.map, trans, gs.camera, delta_t);

    // Draw enemy
    if (gs.enemy_state != ENEMY_HIDDEN) {
        // TODO: More than one enemy?
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

    // Show prompt
    if (gs.player_interact) {
        show_prompt();
    }
}

void show_gameover(void) {
    if (gameover_time++ == 0) {
        sound_effect_play(SOUND_DISCOVERED);
    }
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawBitmap(img_gameover, randi(0, 10), randi(0, 10), kBitmapUnflipped);
}

int note_offset = 0;

void show_note(void) {
    const int MARGIN = 15;
    const int LINE_HEIGHT = 18;
    const int X_OFFSET = 30;
    const int Y_OFFSET = 20;

    const char **text = NOTES[gs.note_id];

    note_offset = clampi(note_offset + pd->system->getCrankChange(), 0, 310);

    int y = Y_OFFSET - note_offset;

    pd->graphics->setDrawMode(kDrawModeFillBlack);
    pd->graphics->clear(kColorBlack);
    pd->graphics->fillRect(X_OFFSET, y, 340, 510, kColorWhite);

    pd->graphics->setFont(font_note);
    int i = 0;
    while (text[i][0] != '\0') {
        pd->graphics->drawText(text[i], strlen(text[i]), kUTF8Encoding, X_OFFSET + MARGIN, y + MARGIN + i * LINE_HEIGHT);
        i++;
    }
}

static int lua_update(lua_State *L) {
    (void)L;  // Unused

    // Calculate delta T
    float update_et_this = pd->system->getElapsedTime();
    float update_delta_t = update_et_this - update_et_last;
    update_et_last = update_et_this;

    switch (gs.player_state) {
        case PLAYER_ACTIVE:
            show_game(update_delta_t);
            break;
        case PLAYER_READING:
            show_note();
            break;
        case PLAYER_GAMEOVER:
            show_gameover();
            break;
    }

#ifdef DEBUG

#ifdef DEBUG_MINIMAP
    minimap_debug_draw(0, 0, &gs);
#endif

    // Print periodically
    if (update_cnt++ % 100 == 0) {
        print_perf();
        debug("Charge    : %f", (double)gs.torch_charge);
        debug("Awareness : %f", (double)gs.enemy_awareness);
        debug("Aggression: %f", (double)gs.enemy_aggression);
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

            if (!pd->lua->addFunction(lua_reset, "resetC", &err)) {
                pd->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
            }
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

            // int seed = time(NULL);
            int seed = 0;
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

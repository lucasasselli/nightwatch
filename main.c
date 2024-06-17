#include <cglm/cglm.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "game.h"
#include "minigl/minigl.h"
#include "minimap.h"
#include "notes.h"
#include "pd_api.h"
#include "pd_system.h"
#include "random.h"
#include "renderer.h"
#include "sound.h"
#include "utils.h"

#define LOAD_STEP_LOCAL_CNT 6
#define LOAD_STEP_CNT LOAD_STEP_LOCAL_CNT + TEX_NUM

// #define TORCH_DISABLE
#define TORCH_INT_STEPS 32
#define TORCH_FADE_STEPS 512

// API handle
PlaydateAPI *pd;

const char *PROMPT_STR_READ = "\xE2\x92\xB6 Read";
const char *PROMPT_STR_ENTER = "\xE2\x92\xB6 Enter";
const char *PROMPT_STR_KEYPAD = "\xE2\x92\xB6 Keypad";
const char *PROMPT_STR_CLOSE = "\xE2\x92\xB7 Close";

#define DRAW_UTF8_STRING(s, x, y) pd->graphics->drawText(s, strlen(s), kUTF8Encoding, x, y)

//---------------------------------------------------------------------------
// Resources
//---------------------------------------------------------------------------

// Textures
minigl_tex_t tex_dither;

// Images
LCDBitmap *img_gameover;

// Objects
minigl_obj_t obj_enemy;

// Object buffer
minigl_objbuf_t obj_buf;

// Fonts
LCDFont *font_gui;
LCDFont *font_note;
LCDFont *font_keypad;

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

char str_buffer[50];

int load_step = 0;
float update_et_last = 0.0f;
int update_cnt = 0;
int gameover_time = 0;
player_state_t player_state_old;
bool prompt_shown = true;
int note_offset = 0;
bool req_clear;

game_state_t gs;

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

static int lua_reset(lua_State *L) {
    (void)L;  // Unused
    game_reset();
    return 0;
}

static int lua_load(lua_State *L) {
    (void)L;  // Unused

    if (load_step < TEX_NUM) {
        // Load textures
        tex_load(load_step);
    } else if (load_step < (TEX_MDBB_NUM + TEX_NUM)) {
        tex_mdbb_load(load_step - TEX_NUM);
    } else if (load_step < (LOAD_STEP_LOCAL_CNT + TEX_NUM + TEX_MDBB_NUM)) {
        switch (load_step - TEX_NUM - TEX_MDBB_NUM) {
            case 0:
                // Sound
                sound_init();
                break;

            case 1:
                // Load Fonts
                font_gui = pd->graphics->loadFont("/System/Fonts/Asheville-Sans-14-Light.pft", NULL);
                font_note = pd->graphics->loadFont("res/fonts/handwriting.pft", NULL);
                font_keypad = pd->graphics->loadFont("res/fonts/Sasser-Slab-Bold.pft", NULL);

                // load Objects
                obj_init();

                // Load image
                img_gameover = pd->graphics->loadBitmap("res/images/gameover.pdi", NULL);

                // Object buffer
                obj_buf = minigl_objbuf_init(200);

                break;

            case 2:
                // Set the dither texture
                // FIXME:
                tex_dither = *tex_get(TEX_DITHER);
                minigl_set_dither(tex_dither);
                break;

            case 3:
                gen_torch_mask();
                break;

            case 4:
                // Setup map
                map_init(gs.map);

#ifdef DEBUG_MINIMAP
                // Setup minimap
                minimap_init();
#endif

                // Initialize game
                game_reset();
                break;
        }
    } else {
        debug("Loading done!");
    }
    load_step++;

    pd->lua->pushInt(load_step);
    pd->lua->pushInt(LOAD_STEP_CNT);
    return 2;
}

void show_prompt(void) {
    pd->graphics->setFont(font_gui);
    pd->graphics->setDrawMode(kDrawModeFillWhite);

    switch (gs.player_interact_item->action.type) {
        case ACTION_NONE:
            break;
        case ACTION_NOTE:
            DRAW_UTF8_STRING(PROMPT_STR_READ, 330, 210);
            break;
        case ACTION_KEYPAD:
            DRAW_UTF8_STRING(PROMPT_STR_KEYPAD, 310, 210);
            break;
    }
    prompt_shown = true;
}

void show_game(float delta_t) {
    // Flush buffer
    minigl_clear(0.0f, 1.0f);

#ifdef DEBUG_MINIMAP
    pd->graphics->clear(kColorBlack);
#endif

    // Draw
    renderer_draw(&gs, delta_t);

    // Update the screen
    screen_update();

    // Show prompt
    if (gs.player_interact) {
        show_prompt();
    } else {
        if (prompt_shown) req_clear = true;
    }
    // FIXME: Clear the screen when interact state changes

#ifdef DEBUG_MINIMAP
    minimap_debug_draw(0, 0, &gs);
#endif
}

void show_note(void) {
    const int MARGIN = 15;
    const int LINE_HEIGHT = 18;
    const int NOTE_WIDTH = 340;
    const int NOTE_HEIGHT = 510;
    const int X_OFFSET = (LCD_COLUMNS - NOTE_WIDTH) / 2;
    const int Y_OFFSET = 20;

    const char **text = NOTES[gs.player_interact_item->action.arg];

    note_offset = clampi(note_offset + pd->system->getCrankChange(), 0, NOTE_HEIGHT - LCD_ROWS / 2);

    int y = Y_OFFSET - note_offset;

    // Paper
    pd->graphics->clear(kColorBlack);
    pd->graphics->fillRect(X_OFFSET, y, NOTE_WIDTH, NOTE_HEIGHT, kColorWhite);

    // Lines
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    pd->graphics->setFont(font_note);
    int i = 0;
    while (text[i][0] != '\0') {
        pd->graphics->drawText(text[i], strlen(text[i]), kUTF8Encoding, X_OFFSET + MARGIN, y + MARGIN + i * LINE_HEIGHT);
        i++;
    }

    // Close prompt
    pd->graphics->setFont(font_gui);
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    DRAW_UTF8_STRING(PROMPT_STR_CLOSE, 170, y + NOTE_HEIGHT + MARGIN);
}

void show_keypad(void) {
    const int KEY_SIZE = 40;
    const int KEY_MARGIN = 5;
    const int DIGIT_X_MARGIN = 15;
    const int DIGIT_Y_MARGIN = 12;

    const int KEY_X_MARGIN = (LCD_COLUMNS - 2 * KEY_MARGIN - 3 * KEY_SIZE) / 2;
    const int KEY_Y_MARGIN = 50;

    const int PIN_X_MARGIN = (LCD_COLUMNS - (KEYPAD_PIN_SIZE - 1) * KEY_MARGIN - KEYPAD_PIN_SIZE * KEY_SIZE) / 2;
    const int PIN_Y_MARGIN = 20;

    pd->graphics->setFont(font_keypad);
    pd->graphics->clear(kColorBlack);

    // Draw display
    for (int i = 0; i < KEYPAD_PIN_SIZE; i++) {
        int x = PIN_X_MARGIN + i * (KEY_MARGIN + KEY_SIZE);
        pd->graphics->setDrawMode(kDrawModeFillWhite);
        if (i < gs.keypad_cnt) {
            sprintf(str_buffer, "%d", gs.keypad_val[i]);
            pd->graphics->drawText(str_buffer, 1, kUTF8Encoding, x + DIGIT_X_MARGIN, PIN_Y_MARGIN);
        } else {
            pd->graphics->drawText("_", 1, kUTF8Encoding, x + DIGIT_X_MARGIN, PIN_Y_MARGIN);
        }
    }

    // Draw keys
    int n = 1;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 3; i++) {
            if (j == 3 && i != 1) continue;
            int x = KEY_X_MARGIN + i * (KEY_MARGIN + KEY_SIZE);
            int y = KEY_Y_MARGIN + j * (KEY_MARGIN + KEY_SIZE);
            sprintf(str_buffer, "%d", n);
            if (n == gs.keypad_sel) {
                pd->graphics->setDrawMode(kDrawModeFillBlack);
                pd->graphics->fillRect(x, y, KEY_SIZE, KEY_SIZE, kColorWhite);
                pd->graphics->drawText(str_buffer, 1, kUTF8Encoding, x + DIGIT_X_MARGIN, y + DIGIT_Y_MARGIN);
            } else {
                pd->graphics->setDrawMode(kDrawModeFillWhite);
                pd->graphics->drawRect(x, y, KEY_SIZE, KEY_SIZE, kColorWhite);
                pd->graphics->drawText(str_buffer, 1, kUTF8Encoding, x + DIGIT_X_MARGIN, y + DIGIT_Y_MARGIN);
            }
            if (++n > 9) n = 0;
        }
    }

    // Draw prompt
    pd->graphics->setFont(font_gui);
    DRAW_UTF8_STRING(PROMPT_STR_ENTER, 325, 190);
    DRAW_UTF8_STRING(PROMPT_STR_CLOSE, 325, 210);
}

void show_gameover(void) {
    if (gameover_time++ == 0) {
        sound_effect_play(SOUND_DISCOVERED);
    }
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawBitmap(img_gameover, randi(0, 10), randi(0, 10), kBitmapUnflipped);
}

static int lua_update(lua_State *L) {
    (void)L;  // Unused

    // Calculate delta T
    float update_et_this = pd->system->getElapsedTime();
    float update_delta_t = update_et_this - update_et_last;
    update_et_last = update_et_this;

    // Real work is done here!
    game_update(update_delta_t);

    if (player_state_old != gs.player_state || req_clear) {
        // Clear screen when changing state!
        pd->graphics->clear(kColorBlack);
        req_clear = false;
    }

    switch (gs.player_state) {
        case PLAYER_ACTIVE:
            show_game(update_delta_t);
            break;
        case PLAYER_READING:
            show_note();
            break;
        case PLAYER_KEYPAD:
            show_keypad();
            break;
        case PLAYER_GAMEOVER:
            show_gameover();
            break;
    }

    player_state_old = gs.player_state;

#ifdef DEBUG

    // Print periodically
    if (update_cnt++ % 100 == 0) {
        print_perf();
        // TODO: Print on screen!
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

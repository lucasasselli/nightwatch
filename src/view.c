#include "view.h"

#include "constants.h"
#include "game_state.h"
#include "minigl/minigl.h"
#include "minimap.h"
#include "notes.h"
#include "pd_api.h"
#include "player.h"
#include "random.h"
#include "renderer.h"
#include "sound.h"
#include "utils.h"

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

const char *PROMPT_STR_INSPECT = "\xE2\x92\xB6 Inspect";
const char *PROMPT_STR_READ = "\xE2\x92\xB6 Read";
const char *PROMPT_STR_ENTER = "\xE2\x92\xB6 Enter";
const char *PROMPT_STR_KEYPAD = "\xE2\x92\xB6 Keypad";
const char *PROMPT_STR_ZOOM = "\xF0\x9F\x9F\xA8 Zoom";
const char *PROMPT_STR_MOVE = "\xE2\x9C\x9B Move";
const char *PROMPT_STR_CLOSE = "\xE2\x92\xB7 Close";
const char *PROMPT_STR_NONOTES = "No notes found!";

// clang-format off
RANDW_CONSTR_BEGIN(float,CONSTR_FLICKER,2)
    RANDW_POINT(0.0, 6),
    RANDW_RANGE(0.0, 1.0, 2)
RANDW_CONSTR_END;
// clang-format on

//---------------------------------------------------------------------------
// Extern
//---------------------------------------------------------------------------

extern PlaydateAPI *pd;

extern game_state_t gs;

//---------------------------------------------------------------------------
// Resources
//---------------------------------------------------------------------------

minigl_frame_t *frame;

// Textures
minigl_tex_t tex_dither;

// Images
LCDBitmap *img_gameover;

// Object buffer
minigl_objbuf_t *obj_buf;

// Fonts
LCDFont *font_gui;
LCDFont *font_note;
LCDFont *font_keypad;

uint8_t torch_mask[TORCH_INT_STEPS][SCREEN_SIZE_Y][SCREEN_SIZE_X];
fp16_t torch_fade[TORCH_INT_STEPS][TORCH_FADE_STEPS];

#define DRAW_UTF8_STRING(s, x, y) pd->graphics->drawText(s, strlen(s), kUTF8Encoding, x, y)

char str_buffer[100];
int x_offset = 0;
int y_offset = 0;
float zoom = 1.0f;
LCDBitmap *img_bm;
int img_width;
int img_height;

action_type_t old_action = ACTION_NONE;

static void torch_mask_init(void) {
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
                    color = (color >= tex_dither.data_g8[y & 0x0000001F][x & 0x0000001F].color);
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
        const float TORCH_S0_K = 0.98f;

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

void view_init(void) {
    // Load Fonts
    font_gui = pd->graphics->loadFont("/System/Fonts/Asheville-Sans-14-Light.pft", NULL);
    font_note = pd->graphics->loadFont("res/fonts/handwriting.pft", NULL);
    font_keypad = pd->graphics->loadFont("res/fonts/Sasser-Slab-Bold.pft", NULL);

    // Load image
    img_gameover = pd->graphics->loadBitmap("res/images/gameover.pdi", NULL);

    // 3D
    frame = minigl_frame_new(SCREEN_SIZE_X, SCREEN_SIZE_Y);
    minigl_set_frame(frame);
    obj_buf = minigl_objbuf_new(200);

    tex_dither = *tex_get(TEX_ID_DITHER);

    // Torch mask
    torch_mask_init();

#ifdef DEBUG_MINIMAP
    // Setup minimap
    minimap_init();
#endif
}

static void screen_update(void) {
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

    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            minigl_pixel_t p = frame->data[y][x];

            assert(p.depth >= 0.0f && p.depth <= 1.0f);

#ifndef TORCH_DISABLE

            // If max brightess mask is 0 then pixel will never be set
            // or cleared
            if (torch_mask[TORCH_INT_STEPS - 1][y][x] == 0) continue;

            uint8_t m = torch_mask[torch_mask_i][y][x];
            if (m) {
                if (p.color > 0) {
                    // If pixel is already fully back, don't bother
                    int torch_fade_i = (TORCH_FADE_STEPS - 1) * p.depth;
                    p.color *= torch_fade[torch_mask_i][torch_fade_i];
                    p.color = (p.color >= tex_dither.data_g8[y & 0x0000001F][x & 0x0000001F].color);
                }
            } else {
                p.color = 0;
            }
#else
            p.color = (p.color >= tex_dither.data[y & 0x0000001F][x & 0x0000001F].color);
#endif

            if (p.color) {
                clearpixel(pd_frame, X_OFFSET + x, SCREEN_SIZE_Y - y - 1, LCD_ROWSIZE);
            } else {
                setpixel(pd_frame, X_OFFSET + x, SCREEN_SIZE_Y - y - 1, LCD_ROWSIZE);
            }
        }
    }
    pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
}

void view_game_draw(float time, float delta_t) {
    // Flush buffer
    minigl_clear(0.0f, 1.0f);

#ifdef DEBUG_MINIMAP
    pd->graphics->clear(kColorBlack);
#endif

    // Draw
    renderer_draw(&gs, delta_t);

    // Update the screen
    screen_update();

    // Draw prompt
    action_type_t action;

    if (gs.player_interact) {
        action = gs.player_interact_item->action.type;
    } else {
        action = ACTION_NONE;
    }

#ifndef DEBUG_MINIMAP
    if (old_action == action && time >= 0.0f) {
        // No need to redraw the prompt!
        return;
    }
#endif

    pd->graphics->setFont(font_gui);
    pd->graphics->fillRect(310, 210, LCD_COLUMNS, LCD_ROWS, kColorBlack);
    pd->graphics->setDrawMode(kDrawModeFillWhite);

    switch (action) {
        case ACTION_NONE:
            break;
        case ACTION_NOTE:
            DRAW_UTF8_STRING(PROMPT_STR_READ, 330, 210);
            break;
        case ACTION_KEYPAD:
            DRAW_UTF8_STRING(PROMPT_STR_KEYPAD, 310, 210);
            break;
        case ACTION_INSPECT:
            DRAW_UTF8_STRING(PROMPT_STR_INSPECT, 310, 210);
            break;
    }

    old_action = action;

#ifdef DEBUG_MINIMAP
    minimap_debug_draw(0, 0, &gs);
#endif
}

static void draw_note(int note_id) {
    const int MARGIN = 15;
    const int LINE_HEIGHT = 18;
    const int NOTE_WIDTH = 340;
    const int NOTE_HEIGHT = 510;
    const int X_OFFSET = (LCD_COLUMNS - NOTE_WIDTH) / 2;
    const int Y_OFFSET = 20;

    const char *text = NOTES[note_id];

    y_offset = clampi(y_offset + pd->system->getCrankChange(), 0, NOTE_HEIGHT - LCD_ROWS / 2);

    int y = Y_OFFSET - y_offset;

    // Paper
    pd->graphics->clear(kColorBlack);
    pd->graphics->fillRect(X_OFFSET, y, NOTE_WIDTH, NOTE_HEIGHT, kColorWhite);

    // Lines
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    pd->graphics->setFont(font_note);

    int str_length = strlen(text);

    int buff_i_last_space = 0;
    int lines = 0;

    int text_i = 0;
    int buff_i = 0;

    while (text_i <= str_length) {
        bool print_line = false;

        char c = text[text_i];

        // Keep track of the last space position
        if (c == ' ') {
            buff_i_last_space = buff_i;
        }

        // Always break when there's a newline or a NULL
        if (c == '\n' || c == '\0') {
            print_line = true;
        } else if (pd->graphics->getTextWidth(font_note, str_buffer, buff_i, kUTF8Encoding, false) > (NOTE_WIDTH - 2 * MARGIN)) {
            text_i -= (buff_i - buff_i_last_space);
            buff_i = buff_i_last_space;
            print_line = true;
        }

        if (print_line) {
            pd->graphics->drawText(str_buffer, buff_i, kUTF8Encoding, X_OFFSET + MARGIN, y + MARGIN + lines * LINE_HEIGHT);
            lines++;
            buff_i = 0;
        } else {
            str_buffer[buff_i] = c;
            buff_i++;
        }

        text_i++;
    }

    // Close prompt
    pd->graphics->setFont(font_gui);
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    DRAW_UTF8_STRING(PROMPT_STR_CLOSE, 170, y + NOTE_HEIGHT + MARGIN);
}

void view_note_draw(float time) {
    if (time == 0.0f) {
        y_offset = 0;
    }
    draw_note(gs.player_interact_item->action.arg);

    // Handle keys
    PDButtons pressed;
    pd->system->getButtonState(NULL, &pressed, NULL);
    if (pressed & kButtonB) {
        player_action_note(false);
    }
}

void view_keypad_draw(float time) {
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

#ifdef DEBUG
    char debug_pin[10];
    sprintf(debug_pin, "%d", gs.player_interact_item->action.arg);
    DRAW_UTF8_STRING(debug_pin, 10, 210);
#endif

    // Handle keys
    PDButtons pressed;
    pd->system->getButtonState(NULL, &pressed, NULL);

    if (pressed & kButtonUp) {
        if (gs.keypad_sel == 0) {
            gs.keypad_sel = 8;
        } else {
            gs.keypad_sel -= 3;
        }
    } else if (pressed & kButtonDown) {
        gs.keypad_sel += 3;
    } else if (pressed & kButtonRight) {
        gs.keypad_sel += 1;
    } else if (pressed & kButtonLeft) {
        gs.keypad_sel -= 1;
    } else if (pressed & kButtonA) {
        gs.keypad_val[gs.keypad_cnt] = gs.keypad_sel;
        gs.keypad_cnt++;
        if (gs.keypad_cnt < KEYPAD_PIN_SIZE) {
            sound_effect_play(SOUND_KEY);
        } else {
            int insert_pin = 0;
            for (int i = 0; i < KEYPAD_PIN_SIZE; i++) {
                insert_pin *= 10;
                insert_pin += gs.keypad_val[i];
            }

            if (insert_pin == gs.player_interact_item->action.arg) {
                // Pin correct!
                sound_effect_play(SOUND_KEYPAD_CORRECT);
                gs.player_interact_item->action.type = ACTION_NONE;
                gs.player_interact_item->obj_id = OBJ_ID_SHUTTER_OPEN;
                gs.player_interact = false;
                sound_effect_play(SOUND_FENCE_OPEN);
                player_action_keypad(false);
            } else {
                // Pin incorrect
                sound_effect_play(SOUND_KEYPAD_WRONG);
                gs.keypad_cnt = 0;
            }
        }
    } else if (pressed & kButtonB) {
        player_action_keypad(false);
    }

    if (gs.keypad_sel < 0) {
        gs.keypad_sel = 9;
    } else if (gs.keypad_sel > 9) {
        gs.keypad_sel = 0;
    }
}

void view_inspect_draw(float time) {
    const float ZOOM_MAX = 3.0f;
    const float ZOOM_K = 0.005f;
    const float TEX_SCALE = 1.0f / ZOOM_MAX;
    const int MARGIN = 18;
    const int MOVE_SPEED = 3;

    // Get the texture
    tex_id_t tex_id = gs.player_interact_item->action.arg;
    minigl_tex_t *tex = tex_get(tex_id);

    if (time == 0.0f) {
        // Init
        x_offset = 0.0f;
        y_offset = 0.0f;
        zoom = 1.0f;

        // Calculate the minimum zoom level to fit the biggest
        // size of the image
        float aspect_ratio = (float)tex->size_x / (float)tex->size_y;
        if (aspect_ratio > 1.0f) {
            // Lanscape
            img_width = LCD_COLUMNS - 2 * MARGIN;
            img_height = img_width / aspect_ratio;
        } else {
            // Portait
            img_height = LCD_ROWS - 2 * MARGIN;
            img_width = img_height / aspect_ratio;
        }

        // Create the bitmap
        img_bm = pd->graphics->newBitmap(img_width * ZOOM_MAX, img_height * ZOOM_MAX, kColorWhite);

        float tex_x_step = (float)tex->size_x / (img_width * ZOOM_MAX);
        float tex_y_step = (float)tex->size_y / (img_height * ZOOM_MAX);

        uint8_t *bm_data = NULL;
        int bm_rowbytes = 0;
        pd->graphics->getBitmapData(img_bm, NULL, NULL, &bm_rowbytes, NULL, &bm_data);

        float tex_y = 0.0f;
        for (int y = 0; y < img_height * ZOOM_MAX; y++) {
            float tex_x = 0.0f;
            for (int x = 0; x < img_width * ZOOM_MAX; x++) {
                uint8_t color;

                if (tex->format == MINIGL_COLOR_FMT_GA8) {
                    uint8_t alpha = tex->data_ga8[(int)tex_y][(int)tex_x].alpha;
                    color = tex->data_ga8[(int)tex_y][(int)tex_x].color;
                    color = alpha > 0.0 ? color : 255;
                } else {
                    color = tex->data_g8[(int)tex_y][(int)tex_x].color;
                }

                if (color > tex_dither.data_g8[y & 0x0000001F][x & 0x0000001F].color) {
                    clearpixel(bm_data, x, y, bm_rowbytes);
                } else {
                    setpixel(bm_data, x, y, bm_rowbytes);
                }
                tex_x += tex_x_step;
            }
            tex_y += tex_y_step;
        }
    }

    // Handle keys
    PDButtons pressed;
    PDButtons pushed;
    pd->system->getButtonState(&pushed, &pressed, NULL);

    if (pushed & kButtonUp) {
        y_offset += MOVE_SPEED;
    }
    if (pushed & kButtonDown) {
        y_offset -= MOVE_SPEED;
    }
    if (pushed & kButtonRight) {
        x_offset -= MOVE_SPEED;
    }
    if (pushed & kButtonLeft) {
        x_offset += MOVE_SPEED;
    }
    if (pressed & kButtonB) {
        player_action_inspect(false);
        pd->graphics->freeBitmap(img_bm);
    }

    x_offset = clampi(x_offset, -LCD_COLUMNS, LCD_COLUMNS);
    y_offset = clampi(y_offset, -LCD_ROWS, LCD_ROWS);

    float crank_delta = pd->system->getCrankChange();

    if (crank_delta != 0.0f) {
        zoom += crank_delta * ZOOM_K;
        zoom = glm_clamp(zoom, 1.0f, ZOOM_MAX);
    }

    // Clear screen
    pd->graphics->clear(kColorBlack);

    // Draw the image
    int img_x = (float)LCD_COLUMNS / 2 - ((float)img_width / 2 - x_offset) * zoom;
    int img_y = (float)LCD_ROWS / 2 - ((float)img_height / 2 - y_offset) * zoom;

    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawScaledBitmap(img_bm, img_x, img_y, TEX_SCALE * zoom, TEX_SCALE * zoom);

    // Commands
    pd->graphics->setFont(font_gui);
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    pd->graphics->fillRect(0, 215, LCD_COLUMNS, 25, kColorBlack);
    pd->graphics->setDrawMode(kDrawModeNXOR);
    DRAW_UTF8_STRING(PROMPT_STR_MOVE, 5, 218);
    DRAW_UTF8_STRING(PROMPT_STR_ZOOM, 170, 218);
    DRAW_UTF8_STRING(PROMPT_STR_CLOSE, 330, 218);
}

int inventory_note = -1;

void view_inventory_draw(float time) {
    if (time == 0.0f) {
        for (int i = 0; i < NOTES_CNT; i++) {
            if (gs.notes_found[i]) inventory_note = i;
        }
    }

    // Handle keys
    PDButtons pressed;
    pd->system->getButtonState(NULL, &pressed, NULL);

    if (inventory_note < 0) {
        // Nothing to show
        pd->graphics->clear(kColorBlack);
        pd->graphics->setDrawMode(kDrawModeFillWhite);
        DRAW_UTF8_STRING(PROMPT_STR_NONOTES, 150, 90);
        DRAW_UTF8_STRING(PROMPT_STR_CLOSE, 175, 120);
    } else {
        int prev_note = -1;
        int next_note = -1;

        // Notes available!
        if (time == 0.0f) {
            y_offset = 0;
        }

        for (int i = 0; i < NOTES_CNT; i++) {
            if (gs.notes_found[i] && i < inventory_note) prev_note = i;
            if (gs.notes_found[i] && i > inventory_note) next_note = i;
        }

        draw_note(inventory_note);

        pd->graphics->setDrawMode(kDrawModeFillWhite);

        if (next_note >= 0) {
            pd->graphics->fillTriangle(385, 115, 390, 120, 385, 125, kPolygonFillEvenOdd);
            if (pressed & kButtonRight) {
                sound_effect_play(SOUND_NOTE);
                inventory_note = next_note;
                y_offset = 0;
            }
        }
        if (prev_note >= 0) {
            pd->graphics->fillTriangle(10, 120, 15, 115, 15, 125, kPolygonFillEvenOdd);
            if (pressed & kButtonLeft) {
                sound_effect_play(SOUND_NOTE);
                inventory_note = prev_note;
                y_offset = 0;
            }
        }
    }

    // B to exit
    if (pressed & kButtonB) {
        player_action_inventory(false);
    }
}

void view_gameover_draw(float time) {
    if (time == 0) {
        sound_effect_play(SOUND_DISCOVERED);
    }
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawBitmap(img_gameover, randi(0, 10), randi(0, 10), kBitmapUnflipped);
}

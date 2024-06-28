#pragma once

#include <stdbool.h>

// #define TORCH_DISABLE
#define TORCH_INT_STEPS 32
#define TORCH_FADE_STEPS 512

void view_init(void);

void view_prompt_draw(void);

void view_game_draw(float time, float delta_t);

void view_note_draw(float time);

void view_keypad_draw(float time);

void view_inspect_draw(float time);

void view_inventory_draw(float time);

void view_gameover_draw(float time);

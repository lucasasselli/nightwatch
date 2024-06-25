#pragma once

#include <stdbool.h>

#include "pd_api.h"

void player_reset(void);

void player_check_interaction(void);

void player_action_note(bool show);

void player_action_keypad(bool show);

void player_action_inspect(bool show);

void player_action_keypress(PDButtons pushed, PDButtons pushed_old, float delta_t);

bool player_action_move(PDButtons pushed, float delta_t);

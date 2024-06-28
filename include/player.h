#pragma once

#include <stdbool.h>

void player_reset(void);

void player_check_interaction(void);

void player_action_note(bool show);

void player_action_keypad(bool show);

void player_action_inspect(bool show);

void player_action_inventory(bool show);

bool player_handle_keys(float delta_t);

#pragma once

#include <stdbool.h>

#include "pd_api.h"

void player_reset(void);

void player_check_interaction(void);

void player_action_note(bool open);

void player_action_move(PDButtons pushed, float delta_t);

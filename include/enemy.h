#pragma once

void enemy_reset(void);

void enemy_update_path(void);

void enemy_update_state(float delta_t);

void enemy_action_move(float speed, float delta_t);

int enemy_get_distance(void);

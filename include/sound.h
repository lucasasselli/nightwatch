#pragma once

#include <stdbool.h>

#include "pd_api.h"

#define SOUNDS_NUM 3

typedef enum {
    SOUND_HEARTBEAT,
    SOUND_DISCOVERED,
    SOUND_FLICKER
} sound_id_t;

extern PlaydateAPI* pd;

void sound_init(void);

bool sound_bg_playing(void);

void sound_bg_start(void);

void sound_bg_stop(void);

void sound_effect_play(sound_id_t sound);

void sound_effect_start(sound_id_t sound);

void sound_effect_stop(sound_id_t sound);

void sound_play_range(sound_id_t sound, int start, int stop);

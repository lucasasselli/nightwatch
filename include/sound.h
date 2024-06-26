#pragma once

#include <stdbool.h>

#include "pd_api.h"

#define SOUNDS_NUM 10

typedef enum {
    SOUND_HEARTBEAT,
    SOUND_DISCOVERED,
    SOUND_FLICKER,
    SOUND_STEP0,
    SOUND_STEP1,
    SOUND_NOTE,
    SOUND_KEY,
    SOUND_KEYPAD_WRONG,
    SOUND_KEYPAD_CORRECT,
    SOUND_FENCE_OPEN
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

void sound_play_volume(sound_id_t id, float vol);

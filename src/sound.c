#include "sound.h"

FilePlayer* bg_music;
AudioSample* audio_sample[SOUNDS_NUM];
SamplePlayer* sample_player[SOUNDS_NUM];
bool sample_playing[SOUNDS_NUM];

void sound_init(void) {
    // bg_music = pd->sound->fileplayer->newPlayer();
    // pd->sound->fileplayer->loadIntoPlayer(bg_music, "res/music/background.pda");
    // pd->sound->fileplayer->setBufferLength(bg_music, 5.0f);

    audio_sample[SOUND_HEARTBEAT] = pd->sound->sample->load("res/sounds/heartbeat.pda");
    audio_sample[SOUND_FLICKER] = pd->sound->sample->load("res/sounds/flicker.pda");
    audio_sample[SOUND_DISCOVERED] = pd->sound->sample->load("res/sounds/discovered.pda");
    audio_sample[SOUND_STEP0] = pd->sound->sample->load("res/sounds/step0.pda");
    audio_sample[SOUND_STEP1] = pd->sound->sample->load("res/sounds/step1.pda");

    for (int i = 0; i < SOUNDS_NUM; i++) {
        sample_player[i] = pd->sound->sampleplayer->newPlayer();
        pd->sound->sampleplayer->setSample(sample_player[i], audio_sample[i]);
    }
}

bool sound_bg_playing(void) {
    // return pd->sound->fileplayer->isPlaying(bg_music);
}

// FIXME: Are we going to keep this?
void sound_bg_start(void) {
    // pd->sound->fileplayer->play(bg_music, 1);
    // pd->sound->fileplayer->setVolume(bg_music, 1.0, 1.0);
}

void sound_bg_stop(void) {}

void sound_effect_play(sound_id_t id) {
    pd->sound->sampleplayer->play(sample_player[id], 1, 1.0f);
}

void sound_effect_start(sound_id_t id) {
    if (!sample_playing[id]) {
        pd->sound->sampleplayer->play(sample_player[id], 0, 1.0f);
        sample_playing[id] = true;
    }
}

void sound_effect_stop(sound_id_t id) {
    if (sample_playing[id]) {
        pd->sound->sampleplayer->stop(sample_player[id]);
        sample_playing[id] = false;
    }
}

void sound_play_range(sound_id_t id, int start, int stop) {
    pd->sound->sampleplayer->setPlayRange(sample_player[id], start, stop);
}

void sound_play_volume(sound_id_t id, float vol) {
    pd->sound->sampleplayer->setVolume(sample_player[id], vol, vol);
}

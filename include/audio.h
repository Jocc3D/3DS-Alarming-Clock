#pragma once
#include "globals.h"
#include <ncsnd.h>

#define SAMPLE_RATE     22050
#define UI_SAMPLE_RATE  44100
#define WAV_HEADER_SIZE 44

typedef struct { void *data; u32 size; } SfxBuf;

// SFX de UI
extern SfxBuf sfx_select, sfx_back, sfx_adjust, sfx_move, sfx_jingle;

void audio_init(void);
void audio_exit(void);
SfxBuf audio_load_sfx(const void *wav, u32 wav_size);

void audio_alarm_start(float volume);
void audio_alarm_stop(void);

void sfx_play_select(void);
void sfx_play_back(void);
void sfx_play_adjust(void);
void sfx_play_move(void);
void sfx_play_jingle(void);

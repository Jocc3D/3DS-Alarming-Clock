#include "audio.h"
#include "button_adjust_wav.h"
#include "button_back_wav.h"
#include "button_move_wav.h"
#include "button_select_wav.h"
#include "jingle_intro_wav.h"
#include <math.h>
#include <string.h>

#define BEEP_DUR   (SAMPLE_RATE / 10)
#define BEEP_GAP   (SAMPLE_RATE / 20)
#define GROUP_GAP  (SAMPLE_RATE * 4 / 10)

// Canal 0: alarma, Canal 1: SFX UI, Canal 2: jingle
#define CH_ALARM  0
#define CH_SFX    1
#define CH_JINGLE 2

SfxBuf sfx_select, sfx_back, sfx_adjust, sfx_move, sfx_jingle;

static s16  *beep_buf         = NULL;
static int   beep_buf_samples = 0;
static volatile bool alarm_thread_run = false;
static Thread alarm_thread = NULL;
static float current_volume = 1.0f;

// Duración del jingle en ticks para bloquear SFX mientras suena
static u64 jingle_end_tick = 0;

static u32 read_wav_rate(const void *wav) {
    const u8 *h = (const u8*)wav;
    return h[24] | (h[25]<<8) | (h[26]<<16) | (h[27]<<24);
}

SfxBuf audio_load_sfx(const void *wav, u32 wav_size) {
    SfxBuf b = {NULL, 0};
    if (wav_size <= WAV_HEADER_SIZE) return b;
    const u8 *header = (const u8*)wav;
    u32 data_size = header[40] | (header[41]<<8) | (header[42]<<16) | (header[43]<<24);
    if (data_size == 0 || data_size > wav_size - WAV_HEADER_SIZE)
        data_size = wav_size - WAV_HEADER_SIZE;
    u32 alloc_size = (data_size + 3) & ~3;
    b.size = alloc_size;
    b.data = linearAlloc(alloc_size);
    if (!b.data) return b;
    memset(b.data, 0, alloc_size);
    memcpy(b.data, header + WAV_HEADER_SIZE, data_size);
    s16 *samples = (s16*)b.data;
    int total_s  = data_size / sizeof(s16);
    int fade_s   = 2000;
    if (fade_s > total_s) fade_s = total_s;
    for (int i = 0; i < fade_s; i++) {
        float f = (float)i / fade_s;
        samples[total_s - fade_s + i] = (s16)(samples[total_s - fade_s + i] * (1.0f - f));
    }
    DSP_FlushDataCache(b.data, alloc_size);
    return b;
}

static void play_sfx_buf(SfxBuf *b, u32 rate) {
    if (!b->data || b->size == 0) return;
    ncsndDirectSound ds;
    ncsndInitializeDirectSound(&ds);
    ds.soundOutputMode                   = NCSND_SOUNDOUTPUT_STEREO;
    ds.channelData.channelAmount         = 2;
    ds.channelData.channelEncoding       = NCSND_ENCODING_PCM16;
    ds.channelData.sampleRate            = rate;
    ds.channelData.leftSampleData        = b->data;
    ds.channelData.rightSampleData       = b->data;
    ds.channelData.sampleDataLength      = b->size;
    ds.soundModifiers.speedMultiplier    = 1.0f;
    ds.soundModifiers.channelVolumes[0]  = NCSND_DIRECTSOUND_MAX_VOLUME;
    ds.soundModifiers.channelVolumes[1]  = NCSND_DIRECTSOUND_MAX_VOLUME;
    ds.soundModifiers.unknown1           = 1.0f;
    ds.soundModifiers.playOnSleep        = 0;
    ds.soundModifiers.forceSpeakerOutput = 0;
    ds.soundModifiers.ignoreVolumeSlider = 0;
    ncsndPlayDirectSound(CH_SFX, 0, &ds);
}

// SFX de UI: solo suenan si el jingle ya terminó
static bool jingle_playing(void) {
    return jingle_end_tick > 0 && svcGetSystemTick() < jingle_end_tick;
}

void sfx_play_select(void) { if (!jingle_playing()) play_sfx_buf(&sfx_select, UI_SAMPLE_RATE); }
void sfx_play_back(void)   { if (!jingle_playing()) play_sfx_buf(&sfx_back,   UI_SAMPLE_RATE); }
void sfx_play_adjust(void) { if (!jingle_playing()) play_sfx_buf(&sfx_adjust, UI_SAMPLE_RATE); }
void sfx_play_move(void)   { if (!jingle_playing()) play_sfx_buf(&sfx_move,   UI_SAMPLE_RATE); }

void sfx_play_jingle(void) {
    if (!sfx_jingle.data || sfx_jingle.size == 0) return;
    u32 rate = read_wav_rate(jingle_intro_wav);
    // Calcular duración en ticks del sistema
    u32 num_samples = sfx_jingle.size / sizeof(s16) / 2; // stereo
    u64 dur_ns = (u64)num_samples * 1000000000ULL / rate;
    u64 dur_ticks = dur_ns * SYSCLOCK_ARM11 / 1000000000ULL;
    jingle_end_tick = svcGetSystemTick() + dur_ticks;

    ncsndDirectSound ds;
    ncsndInitializeDirectSound(&ds);
    ds.soundOutputMode                   = NCSND_SOUNDOUTPUT_STEREO;
    ds.channelData.channelAmount         = 2;
    ds.channelData.channelEncoding       = NCSND_ENCODING_PCM16;
    ds.channelData.sampleRate            = rate;
    ds.channelData.leftSampleData        = sfx_jingle.data;
    ds.channelData.rightSampleData       = sfx_jingle.data;
    ds.channelData.sampleDataLength      = sfx_jingle.size;
    ds.soundModifiers.speedMultiplier    = 1.0f;
    ds.soundModifiers.channelVolumes[0]  = NCSND_DIRECTSOUND_MAX_VOLUME;
    ds.soundModifiers.channelVolumes[1]  = NCSND_DIRECTSOUND_MAX_VOLUME;
    ds.soundModifiers.unknown1           = 1.0f;
    ds.soundModifiers.playOnSleep        = 0;
    ds.soundModifiers.forceSpeakerOutput = 0;
    ds.soundModifiers.ignoreVolumeSlider = 0;
    ncsndPlayDirectSound(CH_JINGLE, 0, &ds);
}

static int gen_pattern(s16 *buf, int count, int offset) {
    int pos = offset;
    for (int b = 0; b < count; b++) {
        float phase = 0.0f;
        for (int i = 0; i < BEEP_DUR; i++) {
            s16 s = (s16)(sinf(phase) * 32767.0f);
            buf[pos*2] = s; buf[pos*2+1] = s; pos++;
            phase += 2.0f * M_PI * 880.0f / SAMPLE_RATE;
            if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
        }
        if (b < count - 1)
            for (int i = 0; i < BEEP_GAP; i++) { buf[pos*2]=0; buf[pos*2+1]=0; pos++; }
    }
    return pos;
}

static void play_alarm_once(void) {
    ncsndDirectSound ds;
    ncsndInitializeDirectSound(&ds);
    ds.soundOutputMode                   = NCSND_SOUNDOUTPUT_STEREO;
    ds.channelData.channelAmount         = 2;
    ds.channelData.channelEncoding       = NCSND_ENCODING_PCM16;
    ds.channelData.sampleRate            = SAMPLE_RATE;
    ds.channelData.leftSampleData        = beep_buf;
    ds.channelData.rightSampleData       = beep_buf;
    ds.channelData.sampleDataLength      = beep_buf_samples * 2 * sizeof(s16);
    ds.soundModifiers.speedMultiplier    = 1.0f;
    ds.soundModifiers.channelVolumes[0]  = (s32)(NCSND_DIRECTSOUND_MAX_VOLUME * current_volume);
    ds.soundModifiers.channelVolumes[1]  = (s32)(NCSND_DIRECTSOUND_MAX_VOLUME * current_volume);
    ds.soundModifiers.unknown1           = 1.0f;
    ds.soundModifiers.playOnSleep        = 1;
    ds.soundModifiers.forceSpeakerOutput = 1;
    ds.soundModifiers.ignoreVolumeSlider = 1;
    ncsndPlayDirectSound(CH_ALARM, 0, &ds);
}

static void alarm_thread_func(void *arg) {
    (void)arg;
    u64 dur_ns = (u64)beep_buf_samples * 1000000000ULL / SAMPLE_RATE;
    while (alarm_thread_run) {
        ncsndStopSound(CH_ALARM);
        play_alarm_once();
        u64 start = svcGetSystemTick();
        while (alarm_thread_run) {
            u64 elapsed = (svcGetSystemTick() - start) * 1000000000ULL / SYSCLOCK_ARM11;
            if (elapsed >= dur_ns) break;
            svcSleepThread(10000000ULL);
        }
    }
    ncsndStopSound(CH_ALARM);
}

void audio_init(void) {
    ncsndInit(true);
    int total = BEEP_DUR*3 + BEEP_GAP*2 + GROUP_GAP + BEEP_DUR*4 + BEEP_GAP*3 + GROUP_GAP;
    beep_buf_samples = total;
    beep_buf = (s16*)linearAlloc(total * 2 * sizeof(s16));
    memset(beep_buf, 0, total * 2 * sizeof(s16));
    int pos = 0;
    pos = gen_pattern(beep_buf, 3, pos);
    for (int i = 0; i < GROUP_GAP; i++) { beep_buf[pos*2]=0; beep_buf[pos*2+1]=0; pos++; }
    pos = gen_pattern(beep_buf, 4, pos);
    for (int i = 0; i < GROUP_GAP; i++) { beep_buf[pos*2]=0; beep_buf[pos*2+1]=0; pos++; }
    DSP_FlushDataCache(beep_buf, total * 2 * sizeof(s16));

    sfx_select = audio_load_sfx(button_select_wav, button_select_wav_size);
    sfx_back   = audio_load_sfx(button_back_wav,   button_back_wav_size);
    sfx_adjust = audio_load_sfx(button_adjust_wav, button_adjust_wav_size);
    sfx_move   = audio_load_sfx(button_move_wav,   button_move_wav_size);
    sfx_jingle = audio_load_sfx(jingle_intro_wav,  jingle_intro_wav_size);
}

void audio_alarm_start(float volume) {
    if (alarm_thread) return;
    current_volume   = volume;
    alarm_thread_run = true;
    alarm_thread = threadCreate(alarm_thread_func, NULL, 32*1024, 0x30, -1, false);
}

void audio_alarm_stop(void) {
    if (!alarm_thread) return;
    alarm_thread_run = false;
    ncsndStopSound(CH_ALARM);
    threadJoin(alarm_thread, 1000000000ULL);
    threadFree(alarm_thread);
    alarm_thread = NULL;
}

void audio_exit(void) {
    audio_alarm_stop();
    linearFree(beep_buf);
    if (sfx_select.data) linearFree(sfx_select.data);
    if (sfx_back.data)   linearFree(sfx_back.data);
    if (sfx_adjust.data) linearFree(sfx_adjust.data);
    if (sfx_move.data)   linearFree(sfx_move.data);
    if (sfx_jingle.data) linearFree(sfx_jingle.data);
    ncsndExit();
}
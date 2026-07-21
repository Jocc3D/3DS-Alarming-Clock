#pragma once
#include "globals.h"
#include "alarm.h"
#include "led.h"

typedef enum {
    SCREEN_MAIN,
    SCREEN_SETTINGS,
    SCREEN_SETTINGS_LANGUAGE,
    SCREEN_ALARM_LIST,
    SCREEN_ALARM_CONFIG,
    SCREEN_CREDITS,
} AppScreen;

void render_init(void);
void render_exit(void);
void render_frame(AppScreen screen, const AppState *state,
                  int selected_opt,
                  int config_opt, bool config_editing, int edit_subfield,
                  int edit_hour, int edit_minute, bool edit_is_pm,
                  float edit_volume,
                  int fade_tick, int fade_frames,
                  int selected_alarm_idx,
                  bool duplicate_error,
                  int tick,
                  int edit_led_color, int edit_led_pattern, bool edit_challenge,
                  bool challenge_active, u32 challenge_seq[4], int challenge_step, bool challenge_fail);

extern C3D_RenderTarget *render_top;
extern C3D_RenderTarget *render_bot;
extern C2D_TextBuf       render_tbuf;
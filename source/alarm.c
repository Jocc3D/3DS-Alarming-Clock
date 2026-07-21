#include "alarm.h"
#include <stdio.h>
#include <time.h>

// Horas default por alarma (en 24h): 0, 4, 8, 12, 16, 20
static const int DEFAULT_HOURS[MAX_ALARMS] = {0, 4, 8, 12, 16, 20};

void alarm_init(AppState *state) {
    state->alarm_count      = MAX_ALARMS;
    state->alarm_active     = false;
    state->active_alarm_idx = -1;
    state->ampm_mode        = false;
    state->language         = 0;
    state->display_mode     = 0;
    for (int i = 0; i < MAX_ALARMS; i++) {
        state->alarms[i].hour      = DEFAULT_HOURS[i];
        state->alarms[i].minute    = 0;
        state->alarms[i].is_pm     = (DEFAULT_HOURS[i] >= 12);
        state->alarms[i].volume    = 0.5f;
        state->alarms[i].enabled   = false;
        state->alarms[i].triggered = false;
        state->alarms[i].used      = false;
        state->alarms[i].led_color   = 0;  // apagado
        state->alarms[i].led_pattern = 0;  // pulso suave
        state->alarms[i].challenge_enabled = false;
    }
}

int alarm_to_24h(int h, bool is_pm, bool ampm_mode) {
    if (!ampm_mode) return h;
    if (is_pm  && h != 12) return h + 12;
    if (!is_pm && h == 12) return 0;
    return h;
}

bool alarm_has_duplicate(const AppState *state, int skip_idx, int h24, int minute) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (i == skip_idx) continue;
        int ah24 = state->alarms[i].hour;
        if (ah24 == h24 && state->alarms[i].minute == minute)
            return true;
    }
    return false;
}

void alarm_find_next(const AppState *state, int *out_h24, int *out_m) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int now_secs = t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec;
    int best_diff = -1;
    *out_h24 = -1;
    *out_m   = -1;
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (!state->alarms[i].enabled) continue;
        int h24 = state->alarms[i].hour;
        int diff = (h24 * 3600 + state->alarms[i].minute * 60) - now_secs;
        if (diff <= 0) diff += 86400;
        if (best_diff < 0 || diff < best_diff) {
            best_diff = diff;
            *out_h24  = h24;
            *out_m    = state->alarms[i].minute;
        }
    }
}

void alarm_check(AppState *state) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    for (int i = 0; i < MAX_ALARMS; i++) {
        Alarm *a = &state->alarms[i];
        if (!a->enabled || a->used) continue;
        int h24 = a->hour;
        if (t->tm_hour == h24 && t->tm_min == a->minute) {
            if (state->alarm_active && state->active_alarm_idx >= 0)
                state->alarms[state->active_alarm_idx].triggered = false;
            a->triggered            = true;
            a->used                 = true;
            state->alarm_active     = true;
            state->active_alarm_idx = i;
            return;
        }
    }
    for (int i = 0; i < MAX_ALARMS; i++) {
        Alarm *a = &state->alarms[i];
        if (a->used && t->tm_min != a->minute) a->used = false;
    }
}

void alarm_stop_active(AppState *state) {
    if (state->active_alarm_idx >= 0)
        state->alarms[state->active_alarm_idx].triggered = false;
    state->alarm_active     = false;
    state->active_alarm_idx = -1;
}

void alarm_format_time(char *out, int size, int h, int m, int s, bool ampm_mode) {
    if (!ampm_mode) {
        snprintf(out, size, "%02d:%02d:%02d", h, m, s);
    } else {
        const char *p = (h < 12) ? "AM" : "PM";
        int h12 = h % 12;
        if (h12 == 0) h12 = 12;
        snprintf(out, size, "%02d:%02d:%02d %s", h12, m, s, p);
    }
}

void alarm_format_short(char *out, int size, int h24, int m, bool ampm_mode, bool is_pm) {
    if (!ampm_mode) {
        snprintf(out, size, "%02d:%02d", h24, m);
    } else {
        const char *p = (h24 < 12) ? "AM" : "PM";
        int h12 = h24 % 12;
        if (h12 == 0) h12 = 12;
        snprintf(out, size, "%02d:%02d %s", h12, m, p);
    }
}

void alarm_time_until(int ah, int am, int ch, int cm, int cs, int *oh, int *om, int *os) {
    int diff = (ah*3600 + am*60) - (ch*3600 + cm*60 + cs);
    if (diff <= 0) diff += 86400;
    *oh = diff / 3600;
    *om = (diff % 3600) / 60;
    *os = diff % 60;
}
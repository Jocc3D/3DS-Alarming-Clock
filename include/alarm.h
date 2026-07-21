#pragma once
#include "globals.h"
 
void alarm_init(AppState *state);
void alarm_check(AppState *state);
void alarm_stop_active(AppState *state);
void alarm_format_time(char *out, int size, int h, int m, int s, bool ampm_mode);
void alarm_format_short(char *out, int size, int h, int m, bool ampm_mode, bool is_pm);
void alarm_time_until(int ah, int am, int ch, int cm, int cs, int *oh, int *om, int *os);
int  alarm_to_24h(int h, bool is_pm, bool ampm_mode);
bool alarm_has_duplicate(const AppState *state, int skip_idx, int h24, int minute);
void alarm_find_next(const AppState *state, int *out_h24, int *out_m);
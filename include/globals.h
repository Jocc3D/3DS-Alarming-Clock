#pragma once
#include <3ds.h>
#include <citro2d.h>

#define SCREEN_W_TOP  400
#define SCREEN_W_BOT  320
#define SCREEN_H      240

#define COL_BG       C2D_Color32(0,   1,   59,  255)
#define COL_STRIPE   C2D_Color32(37,  78,  159, 255)
#define COL_WHITE    C2D_Color32(255, 255, 255, 255)
#define COL_TEXT     C2D_Color32(187, 209, 253, 255)
#define COL_RED      C2D_Color32(220, 50,  50,  255)
#define COL_DARKRED  C2D_Color32(120, 20,  20,  255)
#define COL_ACCENT   C2D_Color32(255, 200, 0,   255)
#define COL_GREEN    C2D_Color32(50,  200, 80,  255)

#define MAX_ALARMS   6

#define LED_COLOR_COUNT   6
#define LED_PATTERN_COUNT 5

typedef struct {
    int   hour;
    int   minute;
    bool  is_pm;
    float volume;
    bool  enabled;
    bool  triggered;
    bool  used;
    int   led_color;
    int   led_pattern;
    bool  challenge_enabled;
} Alarm;

typedef struct {
    Alarm alarms[MAX_ALARMS];
    int   alarm_count;
    bool  ampm_mode;
    bool  alarm_active;
    int   active_alarm_idx;
    int   language;
    int   display_mode;
} AppState;
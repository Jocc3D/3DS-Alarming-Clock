#pragma once

typedef enum {
    LANG_ES = 0,
    LANG_EN,
    LANG_COUNT
} Language;

typedef enum {
    S_APP_TITLE = 0,
    S_TODAY,
    S_NEXT_ALARM_IN,
    S_NEXT_ALARM_TIME,
    S_NO_ALARMS,
    S_ALARM_RINGING,
    S_PRESS_A_STOP,
    S_A_SELECT,
    S_ALARM_SOUNDING,
    S_TURN_OFF_UNLOCK,
    S_HOME_BUTTON,
    S_MENU,
    S_ALARMS,
    S_SETTINGS,
    S_CREDITS,
    S_DPAD_A_TOUCH,
    S_A_CONFIG_TOUCH,
    S_ALARM_ON,
    S_ALARM_OFF,
    S_ALARM_N,
    S_RINGS_IN,
    S_TIME,
    S_VOLUME,
    S_DUPLICATE_ERR,
    S_NAV_HINT,
    S_EDIT_HINT,
    S_LR_FIELD,
    S_CONFIRM_CANCEL,
    S_EDIT_BACK,
    S_SETTINGS_TITLE,
    S_TIME_FORMAT,
    S_FORMAT_AMPM,
    S_FORMAT_24H,
    S_LANGUAGE,
    S_DPAD_A_TOUCH_BACK,
    S_LANG_SELECT_HINT,
    S_LANG_SETTINGS_TITLE,
    S_SETTINGS_SUB,
    S_CREDITS_SUB,
    S_CREDITS_TITLE,
    S_CREDITS_LINE1,
    S_COUNT
} StrID;

extern const char *STR[LANG_COUNT][S_COUNT];

#define s(lang, id) STR[(lang)][(id)]
#include "strings.h"

const char *STR[LANG_COUNT][S_COUNT] = {
    // ===== ESPANOL =====
    {
        "3DS Alarming Clock",          // S_APP_TITLE
        "Hoy es %02d de %s de %04d",   // S_TODAY
        "La próxima alarma suena en",  // S_NEXT_ALARM_IN
        "%02d horas, %02d minutos y %02d segundos", // S_NEXT_ALARM_TIME
        "No hay alarmas activas",      // S_NO_ALARMS
        "!! ALARMA SONANDO !!",        // S_ALARM_RINGING
        "Presiona A para detener",     // S_PRESS_A_STOP
        "A: Seleccionar",              // S_A_SELECT
        "¡La alarma está sonando!",    // S_ALARM_SOUNDING
        "Apágala para desbloquear",    // S_TURN_OFF_UNLOCK
        "el boton Home",               // S_HOME_BUTTON
        "Menu",                        // S_MENU
        "Alarmas",                     // S_ALARMS
        "Ajustes",                     // S_SETTINGS
        "Créditos",                    // S_CREDITS
        "D-Pad + A  o  toca",          // S_DPAD_A_TOUCH
        "A: Config  Toca toggle: on/off", // S_A_CONFIG_TOUCH
        "Encendida",                   // S_ALARM_ON
        "Apagada",                     // S_ALARM_OFF
        "Alarma %d",                   // S_ALARM_N
        "Suena en %dh %dm %ds",        // S_RINGS_IN
        "Hora",                        // S_TIME
        "Volumen",                     // S_VOLUME
        "¡Ya hay una alarma a esa hora!", // S_DUPLICATE_ERR
        "Arr/Abj: navegar  A: editar  B: volver", // S_NAV_HINT
        "Arr/Abj: valor  B: cancelar", // S_EDIT_HINT
        "Izq/Der: campo  A: confirmar",// S_LR_FIELD
        "A: Confirmar  B: Cancelar",   // S_CONFIRM_CANCEL
        "A: Editar  B: Volver",        // S_EDIT_BACK
        "Ajustes",                     // S_SETTINGS_TITLE
        "Formato de horas",            // S_TIME_FORMAT
        "12 horas",                    // S_FORMAT_AMPM
        "24 horas",                    // S_FORMAT_24H
        "Idioma",                      // S_LANGUAGE
        "D-Pad + A  o  toca  B: volver", // S_DPAD_A_TOUCH_BACK
        "Selecciona el idioma",        // S_LANG_SELECT_HINT
        "Ajustes > Idioma",            // S_LANG_SETTINGS_TITLE
        "Configuraciones varias",      // S_SETTINGS_SUB
        "¡Muchas gracias!",            // S_CREDITS_SUB
        "Créditos",                    // S_CREDITS_TITLE
        "3DS Alarming Clock",          // S_CREDITS_LINE1
    },
    // ===== ENGLISH =====
    {
        "3DS Alarming Clock",
        "Today is %s %02d, %04d",
        "Next alarm rings in",
        "%02d hours, %02d minutes and %02d seconds",
        "No active alarms",
        "!! ALARM RINGING !!",
        "Press A to stop",
        "A: Select",
        "The alarm is ringing!",
        "Turn it off to unlock",
        "the Home button",
        "Menu",
        "Alarms",
        "Settings",
        "Credits",
        "D-Pad + A  or  tap",
        "A: Config  Tap toggle: on/off",
        "On",
        "Off",
        "Alarm %d",
        "Rings in %dh %dm %ds",
        "Time",
        "Volume",
        "An alarm already exists at that time!",
        "Up/Dn: navigate  A: edit  B: back",
        "Up/Dn: value  B: cancel",
        "L/R: field  A: confirm",
        "A: Confirm  B: Cancel",
        "A: Edit  B: Back",
        "Settings",
        "Time format",
        "12-hour clock",               // S_FORMAT_AMPM
        "24-hour clock",               // S_FORMAT_24H
        "Language",
        "D-Pad + A  or  tap  B: back",
        "Select language",             // S_LANG_SELECT_HINT
        "Settings > Language",         // S_LANG_SETTINGS_TITLE
        "Various settings",            // S_SETTINGS_SUB
        "Thank you so much!",          // S_CREDITS_SUB
        "Credits",
        "3DS Alarming Clock",
    },
};
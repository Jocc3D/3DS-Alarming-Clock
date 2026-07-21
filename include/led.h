#pragma once
#include <3ds.h>

// Orden: Apagado, Rojo, Amarillo, Azul, Morado, Verde
extern const char *LED_COLOR_NAMES_ES[6];
extern const char *LED_COLOR_NAMES_EN[6];
extern const char *LED_PATTERN_NAMES_ES[5];
extern const char *LED_PATTERN_NAMES_EN[5];
extern const u32 LED_COLOR_UI[6];

void led_init(void);
void led_exit(void);
void led_set(int color, int pattern);
void led_set_solid(int color);
void led_clear(void);
#pragma once
#include "globals.h"

void ui_init(void);
void ui_exit(void);

void ui_draw_btn_a(float x, float y, float size);
void ui_draw_btn_b(float x, float y, float size);
void ui_draw_btn_x(float x, float y, float size);
void ui_draw_btn_y(float x, float y, float size);
void ui_draw_dpad_up(float x, float y, float size);
void ui_draw_dpad_down(float x, float y, float size);
void ui_draw_dpad_left(float x, float y, float size);
void ui_draw_dpad_right(float x, float y, float size);
void ui_draw_volume(float x, float y, float size);

void ui_draw_battery(float x, float y, float size, int percent, int tick, bool charging);

float ui_hint_a(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color);
float ui_hint_b(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color);
float ui_hint_dpad_ud(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color);
float ui_hint_dpad_lr(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color);
#include "ui.h"
#include "ui_button_a_t3x.h"
#include "ui_button_b_t3x.h"
#include "ui_button_x_t3x.h"
#include "ui_button_y_t3x.h"
#include "ui_button_dpad_up_t3x.h"
#include "ui_button_dpad_down_t3x.h"
#include "ui_button_dpad_left_t3x.h"
#include "ui_button_dpad_right_t3x.h"
#include "ui_volume_icon_t3x.h"
#include "ui_battery_0_t3x.h"
#include "ui_battery_25_t3x.h"
#include "ui_battery_50_t3x.h"
#include "ui_battery_75_t3x.h"
#include "ui_battery_100_t3x.h"
#include "ui_battery_charging_t3x.h"
#include <citro2d.h>

#define NUM_SHEETS 15
static C2D_SpriteSheet sheets[NUM_SHEETS];
static C2D_Image        imgs[NUM_SHEETS];

#define BTN_A        0
#define BTN_B        1
#define BTN_X        2
#define BTN_Y        3
#define DPAD_UP      4
#define DPAD_DN      5
#define DPAD_L       6
#define DPAD_R       7
#define VOL_ICO      8
#define BAT_0        9
#define BAT_25       10
#define BAT_50       11
#define BAT_75       12
#define BAT_100      13
#define BAT_CHARGING 14

static const struct { const void *data; u32 size; } sheet_data[NUM_SHEETS] = {
    { ui_button_a_t3x,           ui_button_a_t3x_size           },
    { ui_button_b_t3x,           ui_button_b_t3x_size           },
    { ui_button_x_t3x,           ui_button_x_t3x_size           },
    { ui_button_y_t3x,           ui_button_y_t3x_size           },
    { ui_button_dpad_up_t3x,     ui_button_dpad_up_t3x_size     },
    { ui_button_dpad_down_t3x,   ui_button_dpad_down_t3x_size   },
    { ui_button_dpad_left_t3x,   ui_button_dpad_left_t3x_size   },
    { ui_button_dpad_right_t3x,  ui_button_dpad_right_t3x_size  },
    { ui_volume_icon_t3x,        ui_volume_icon_t3x_size        },
    { ui_battery_0_t3x,          ui_battery_0_t3x_size          },
    { ui_battery_25_t3x,         ui_battery_25_t3x_size         },
    { ui_battery_50_t3x,         ui_battery_50_t3x_size         },
    { ui_battery_75_t3x,         ui_battery_75_t3x_size         },
    { ui_battery_100_t3x,        ui_battery_100_t3x_size        },
    { ui_battery_charging_t3x,   ui_battery_charging_t3x_size   },
};

void ui_init(void) {
    for (int i = 0; i < NUM_SHEETS; i++) {
        sheets[i] = C2D_SpriteSheetLoadFromMem(sheet_data[i].data, sheet_data[i].size);
        if (sheets[i]) imgs[i] = C2D_SpriteSheetGetImage(sheets[i], 0);
    }
}

void ui_exit(void) {
    for (int i = 0; i < NUM_SHEETS; i++)
        if (sheets[i]) C2D_SpriteSheetFree(sheets[i]);
}

static void draw_img(int idx, float x, float y, float size) {
    if (!sheets[idx]) return;
    C2D_DrawImageAt(imgs[idx], x, y, 0.5f, NULL, size/128.0f, size/128.0f);
}

void ui_draw_btn_a(float x, float y, float size)     { draw_img(BTN_A,   x, y, size); }
void ui_draw_btn_b(float x, float y, float size)     { draw_img(BTN_B,   x, y, size); }
void ui_draw_btn_x(float x, float y, float size)     { draw_img(BTN_X,   x, y, size); }
void ui_draw_btn_y(float x, float y, float size)     { draw_img(BTN_Y,   x, y, size); }
void ui_draw_dpad_up(float x, float y, float size)   { draw_img(DPAD_UP, x, y, size); }
void ui_draw_dpad_down(float x, float y, float size) { draw_img(DPAD_DN, x, y, size); }
void ui_draw_dpad_left(float x, float y, float size) { draw_img(DPAD_L,  x, y, size); }
void ui_draw_dpad_right(float x, float y, float size){ draw_img(DPAD_R,  x, y, size); }
void ui_draw_volume(float x, float y, float size)    { draw_img(VOL_ICO, x, y, size); }

void ui_draw_battery(float x, float y, float size, int percent, int tick, bool charging) {
    if (charging) {
        // Parpadeo 0.75s visible / 0.75s invisible
        // 0.75s * 60fps = 45 frames por estado
        // El primer estado es visible, por lo que ocultamos en el segundo slot
        if ((tick / 45) % 2 == 1) return;
        draw_img(BAT_CHARGING, x, y, size);
        return;
    }
    // Parpadeo batería baja: visible/invisible cada 12 frames
    if (percent <= 9 && (tick / 12) % 2 == 1) return;
    int idx;
    if      (percent >= 76) idx = BAT_100;
    else if (percent >= 51) idx = BAT_75;
    else if (percent >= 26) idx = BAT_50;
    else if (percent >= 10) idx = BAT_25;
    else                    idx = BAT_0;
    draw_img(idx, x, y, size);
}

static float draw_text_at(const char *str, float x, float y, float sx, float sy, u32 color) {
    extern C2D_TextBuf render_tbuf;
    extern C2D_Font    render_font;
    C2D_Text txt;
    C2D_TextBufClear(render_tbuf);
    C2D_TextFontParse(&txt, render_font, render_tbuf, str);
    C2D_TextOptimize(&txt);
    float w, h;
    C2D_TextGetDimensions(&txt, sx, sy, &w, &h);
    C2D_DrawText(&txt, C2D_WithColor, x, y, 0.5f, sx, sy, color);
    return w;
}

static float hint_icon(int idx, float x, float y, float size, float gap,
                        const char *text, float tsx, float tsy, u32 color) {
    draw_img(idx, x, y, size);
    float th = tsy * 30.0f;
    float ty = y + (size - th) / 2.0f;
    float tw = draw_text_at(text, x + size + gap, ty, tsx, tsy, color);
    return size + gap + tw;
}

float ui_hint_a(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color) {
    return hint_icon(BTN_A, x, y, size, gap, text, tsx, tsy, color);
}
float ui_hint_b(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color) {
    return hint_icon(BTN_B, x, y, size, gap, text, tsx, tsy, color);
}
float ui_hint_dpad_ud(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color) {
    draw_img(DPAD_UP, x, y, size);
    draw_img(DPAD_DN, x + size + 1, y, size);
    float th = tsy * 30.0f;
    float ty = y + (size - th) / 2.0f;
    float tw = draw_text_at(text, x + size*2 + 2 + gap, ty, tsx, tsy, color);
    return size*2 + 2 + gap + tw;
}
float ui_hint_dpad_lr(float x, float y, float size, float gap, const char *text, float tsx, float tsy, u32 color) {
    draw_img(DPAD_L, x, y, size);
    draw_img(DPAD_R, x + size + 1, y, size);
    float th = tsy * 30.0f;
    float ty = y + (size - th) / 2.0f;
    float tw = draw_text_at(text, x + size*2 + 2 + gap, ty, tsx, tsy, color);
    return size*2 + 2 + gap + tw;
}
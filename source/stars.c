#include "stars.h"
#include <stdlib.h>

Star stars_top[NUM_STARS_TOP];
Star stars_bot[NUM_STARS_BOT];

static void init_set(Star *stars, int count, float screen_w) {
    for (int i = 0; i < count; i++) {
        stars[i].x     = (float)(rand() % (int)screen_w);
        stars[i].y     = (float)(rand() % SCREEN_H);
        stars[i].phase = rand() % STAR_CYCLE;
    }
}

static void draw_set(Star *stars, int count) {
    for (int i = 0; i < count; i++) {
        stars[i].phase = (stars[i].phase + 1) % STAR_CYCLE;
        float t      = (float)stars[i].phase / STAR_CYCLE;
        float bright = (t < 0.5f) ? (t * 2.0f) : (2.0f - t * 2.0f);
        u8 alpha = (u8)(bright * 255.0f);
        if (alpha < 5) continue;
        C2D_DrawRectSolid(stars[i].x, stars[i].y, 0.2f, 2, 2,
            C2D_Color32(255, 255, 255, alpha));
    }
}

void stars_init(void) {
    init_set(stars_top, NUM_STARS_TOP, SCREEN_W_TOP);
    init_set(stars_bot, NUM_STARS_BOT, SCREEN_W_BOT);
}

void stars_draw_top(void) { draw_set(stars_top, NUM_STARS_TOP); }
void stars_draw_bot(void) { draw_set(stars_bot, NUM_STARS_BOT); }
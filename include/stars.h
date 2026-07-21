#pragma once
#include "globals.h"

#define NUM_STARS_TOP 50
#define NUM_STARS_BOT 35
#define STAR_CYCLE    120

typedef struct { float x, y; int phase; } Star;

extern Star stars_top[NUM_STARS_TOP];
extern Star stars_bot[NUM_STARS_BOT];

void stars_init(void);
void stars_draw_top(void);
void stars_draw_bot(void);

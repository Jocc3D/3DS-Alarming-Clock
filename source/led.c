#include "led.h"
#include <string.h>

// Orden: 0=Apagado, 1=Rojo, 2=Amarillo, 3=Azul, 4=Morado, 5=Verde
const char *LED_COLOR_NAMES_ES[6] = {"Apagado","Rojo","Amarillo","Azul","Morado","Verde"};
const char *LED_COLOR_NAMES_EN[6] = {"Off","Red","Yellow","Blue","Purple","Green"};
const char *LED_PATTERN_NAMES_ES[5] = {"Pulso suave","Lento","Ondulante","Intermitente","Respiracion"};
const char *LED_PATTERN_NAMES_EN[5] = {"Soft pulse","Slow","Undulating","Intermittent","Breathing"};

const u32 LED_COLOR_UI[6] = {
    0xFF101010, // apagado
    0xFF2020FF, // rojo
    0xFF00FFFF, // amarillo
    0xFFFF2000, // azul
    0xFFFF20C0, // morado
    0xFF20C020, // verde
};

void led_init(void)  { mcuHwcInit(); }
void led_exit(void)  { led_clear(); mcuHwcExit(); }

static void get_rgb(int color, u8 *r, u8 *g, u8 *b) {
    switch(color) {
        case 0: *r=0x00; *g=0x00; *b=0x00; break; // apagado
        case 1: *r=0xFF; *g=0x00; *b=0x00; break; // rojo
        case 2: *r=0xFF; *g=0xFF; *b=0x00; break; // amarillo
        case 3: *r=0x00; *g=0x80; *b=0xFF; break; // azul
        case 4: *r=0xCC; *g=0x00; *b=0xFF; break; // morado
        case 5: *r=0x00; *g=0xFF; *b=0x00; break; // verde
        default:*r=0xFF; *g=0xFF; *b=0xFF; break;
    }
}

static u8 scale(u8 ch, u8 bright) { return (u8)((ch * bright) / 255); }

void led_set(int color, int pattern) {
    // Si es apagado, simplemente limpiar
    if (color == 0) { led_clear(); return; }
    InfoLedPattern p;
    memset(&p, 0, sizeof(p));
    u8 r, g, b;
    get_rgb(color, &r, &g, &b);

    switch(pattern) {
        case 0: // Pulso suave
            p.delay=0x08; p.smoothing=0xFF; p.loopDelay=0x00;
            for(int i=0;i<8;i++){u8 br=(i<4)?(u8)(i*64+32):(u8)((7-i)*64+32);p.redPattern[i]=scale(r,br);p.greenPattern[i]=scale(g,br);p.bluePattern[i]=scale(b,br);}
            break;
        case 1: // Lento
            p.delay=0x18; p.smoothing=0xC0; p.loopDelay=0x10;
            for(int i=0;i<8;i++){u8 br=(i<4)?0xFF:0x00;p.redPattern[i]=scale(r,br);p.greenPattern[i]=scale(g,br);p.bluePattern[i]=scale(b,br);}
            break;
        case 2: // Ondulante
            p.delay=0x0A; p.smoothing=0xFF; p.loopDelay=0x00;
            {u8 wave[8]={0x40,0x80,0xC0,0xFF,0xC0,0x80,0x40,0x20};for(int i=0;i<8;i++){p.redPattern[i]=scale(r,wave[i]);p.greenPattern[i]=scale(g,wave[i]);p.bluePattern[i]=scale(b,wave[i]);}}
            break;
        case 3: // Intermitente
            p.delay=0x08; p.smoothing=0x40; p.loopDelay=0x04;
            {u8 bl[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00};for(int i=0;i<8;i++){p.redPattern[i]=scale(r,bl[i]);p.greenPattern[i]=scale(g,bl[i]);p.bluePattern[i]=scale(b,bl[i]);}}
            break;
        case 4: // Respiracion
            p.delay=0x10; p.smoothing=0xFF; p.loopDelay=0x18;
            {u8 br[8]={0x00,0x20,0x60,0xB0,0xFF,0xB0,0x60,0x10};for(int i=0;i<8;i++){p.redPattern[i]=scale(r,br[i]);p.greenPattern[i]=scale(g,br[i]);p.bluePattern[i]=scale(b,br[i]);}}
            break;
    }
    MCUHWC_SetInfoLedPattern(&p);
}

void led_set_solid(int color) {
    if (color == 0) { led_clear(); return; }
    InfoLedPattern p;
    memset(&p, 0, sizeof(p));
    u8 r, g, b;
    get_rgb(color, &r, &g, &b);
    p.delay=0xFF; p.smoothing=0x00; p.loopDelay=0x00;
    for(int i=0;i<8;i++){p.redPattern[i]=r;p.greenPattern[i]=g;p.bluePattern[i]=b;}
    MCUHWC_SetInfoLedPattern(&p);
}

void led_clear(void) {
    InfoLedPattern p;
    memset(&p, 0, sizeof(p));
    MCUHWC_SetInfoLedPattern(&p);
}
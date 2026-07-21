#include "render.h"
#include "stars.h"
#include "alarm.h"
#include "strings.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

C3D_RenderTarget *render_top;
C3D_RenderTarget *render_bot;
C2D_TextBuf       render_tbuf;
C2D_Font render_font;

void render_init(void) {
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    render_top  = C2D_CreateScreenTarget(GFX_TOP,    GFX_LEFT);
    render_bot  = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    render_tbuf = C2D_TextBufNew(512);
    render_font = C2D_FontLoadSystem(CFG_REGION_EUR);
}

void render_exit(void) {
    C2D_TextBufDelete(render_tbuf);
    C2D_FontFree(render_font);
    C2D_Fini();
    C3D_Fini();
}

static void draw_centered(const char *str, float y, float sx, float sy, u32 color, float sw) {
    C2D_Text txt;
    C2D_TextBufClear(render_tbuf);
    C2D_TextFontParse(&txt, render_font, render_tbuf, str);
    C2D_TextOptimize(&txt);
    float w, h;
    C2D_TextGetDimensions(&txt, sx, sy, &w, &h);
    C2D_DrawText(&txt, C2D_WithColor, (sw-w)/2.0f, y, 0.5f, sx, sy, color);
}

static void draw_top(const char *str, float y, float sx, float sy, u32 color) {
    draw_centered(str, y, sx, sy, color, SCREEN_W_TOP);
}

static void draw_bot(const char *str, float y, float sx, float sy, u32 color) {
    draw_centered(str, y, sx, sy, color, SCREEN_W_BOT);
}

static void draw_in_card(const char *str, float x, float y, float cw, float sx, float sy, u32 color) {
    C2D_Text txt;
    C2D_TextBufClear(render_tbuf);
    C2D_TextFontParse(&txt, render_font, render_tbuf, str);
    C2D_TextOptimize(&txt);
    float w, h;
    C2D_TextGetDimensions(&txt, sx, sy, &w, &h);
    float tx = x + (cw-w)/2.0f;
    if (tx < x) tx = x;
    C2D_DrawText(&txt, C2D_WithColor, tx, y, 0.5f, sx, sy, color);
}

static const char *month_es[] = {"enero","febrero","marzo","abril","mayo","junio","julio","agosto","septiembre","octubre","noviembre","diciembre"};
static const char *month_en[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

// Cuadros centrados: 3*100 + 2*5 = 310, mx=(320-310)/2=5
static void draw_alarm_card(const AppState *state, int idx, int col, int row, bool selected) {
    char buf[32];
    int cw=100, ch=86, mx=5, my=26;
    int x=mx+col*(cw+5), y=my+row*(ch+4);
    int lang=state->language;
    u32 border = selected ? COL_ACCENT : COL_STRIPE;
    C2D_DrawRectSolid(x, y, 0.4f, cw, ch, border);
    C2D_DrawRectSolid(x+2, y+2, 0.45f, cw-4, ch-4, COL_BG);
    const Alarm *a = &state->alarms[idx];
    alarm_format_short(buf, sizeof(buf), a->hour, a->minute, state->ampm_mode, a->is_pm);
    draw_in_card(buf, x, y+10, cw, 0.52f, 0.52f, COL_WHITE);
    snprintf(buf, sizeof(buf), "Vol: %d%%", (int)roundf(a->volume*100));
    draw_in_card(buf, x, y+34, cw, 0.38f, 0.38f, COL_TEXT);
    u32 tog_col = a->enabled ? COL_GREEN : C2D_Color32(100,100,120,255);
    C2D_DrawRectSolid(x+10, y+ch-24, 0.46f, cw-20, 18, tog_col);
    draw_in_card(a->enabled ? s(lang,S_ALARM_ON) : s(lang,S_ALARM_OFF),
                 x, y+ch-21, cw, 0.36f, 0.36f, COL_WHITE);
}

void render_frame(AppScreen screen, const AppState *state,
                  int selected_opt,
                  int config_opt, bool config_editing, int edit_subfield,
                  int edit_hour, int edit_minute, bool edit_is_pm,
                  float edit_volume,
                  int fade_tick, int fade_frames,
                  int selected_alarm_idx,
                  bool duplicate_error,
                  int tick,
                  int edit_led_color, int edit_led_pattern, bool edit_challenge,
                  bool challenge_active, u32 challenge_seq[4], int challenge_step, bool challenge_fail) {
    char buf[128];
    time_t now_t = time(NULL);
    struct tm *t = localtime(&now_t);
    int lang = state->language;
    int alarm_h24=-1, alarm_m=0;
    alarm_find_next(state, &alarm_h24, &alarm_m);

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    // ===== PANTALLA SUPERIOR =====
    C2D_TargetClear(render_top, COL_BG);
    C2D_SceneBegin(render_top);
    stars_draw_top();
    C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_W_TOP, 30, COL_STRIPE);
    draw_top(s(lang,S_APP_TITLE), 5, 0.65f, 0.65f, COL_WHITE);

    // Batería: icono 30px pegado al borde derecho, porcentaje a su izquierda
    {
        u8 bat_pct = 0;
        MCUHWC_GetBatteryLevel(&bat_pct);
        int pct = (int)bat_pct;
        if(pct < 0) pct = 0;
        if(pct > 100) pct = 100;
        bool charging = false;
        PTMU_GetAdapterState(&charging);
        snprintf(buf, sizeof(buf), "%d%%", pct);
        float bx = SCREEN_W_TOP - 30 - 2;
        ui_draw_battery(bx, 0, 30, pct, tick, charging);
        C2D_Text btxt; C2D_TextBufClear(render_tbuf);
        C2D_TextFontParse(&btxt, render_font, render_tbuf, buf); C2D_TextOptimize(&btxt);
        float tw, th; C2D_TextGetDimensions(&btxt, 0.38f, 0.38f, &tw, &th);
        C2D_DrawText(&btxt, C2D_WithColor, bx - tw - 4, 7, 0.5f, 0.38f, 0.38f, COL_TEXT);
    }

    // Fecha y hora en pantalla superior
    if (state->display_mode == 0) {
        if (lang==0)
            snprintf(buf, sizeof(buf), "Hoy es %02d de %s de %04d", t->tm_mday, month_es[t->tm_mon], t->tm_year+1900);
        else
            snprintf(buf, sizeof(buf), "Today is %s %02d, %04d", month_en[t->tm_mon], t->tm_mday, t->tm_year+1900);
        draw_top(buf, 38, 0.48f, 0.48f, COL_TEXT);
        alarm_format_time(buf, sizeof(buf), t->tm_hour, t->tm_min, t->tm_sec, state->ampm_mode);
        draw_top(buf, 60, state->ampm_mode?1.0f:1.3f, state->ampm_mode?1.0f:1.3f,
                 state->alarm_active ? COL_RED : COL_TEXT);
        if (!state->alarm_active) {
            if (alarm_h24>=0) {
                int dh,dm,ds;
                alarm_time_until(alarm_h24,alarm_m,t->tm_hour,t->tm_min,t->tm_sec,&dh,&dm,&ds);
                draw_top(s(lang,S_NEXT_ALARM_IN), 155, 0.46f, 0.46f, COL_TEXT);
                snprintf(buf, sizeof(buf), s(lang,S_NEXT_ALARM_TIME), dh, dm, ds);
                draw_top(buf, 172, 0.46f, 0.46f, COL_TEXT);
            } else {
                draw_top(s(lang,S_NO_ALARMS), 163, 0.46f, 0.46f, COL_TEXT);
            }
        } else {
            draw_top(s(lang,S_ALARM_RINGING), 155, 0.58f, 0.58f, COL_RED);
            draw_top(s(lang,S_PRESS_A_STOP),  174, 0.46f, 0.46f, COL_RED);
        }
    } else {
        if (lang==0)
            snprintf(buf,sizeof(buf),"Hoy es %02d de %s de %04d", t->tm_mday, month_es[t->tm_mon], t->tm_year+1900);
        else
            snprintf(buf,sizeof(buf),"Today is %s %02d, %04d", month_en[t->tm_mon], t->tm_mday, t->tm_year+1900);
        draw_centered(buf, 33, 0.42f, 0.42f, COL_TEXT, 200);
        alarm_format_time(buf, sizeof(buf), t->tm_hour, t->tm_min, t->tm_sec, state->ampm_mode);
        draw_centered(buf, 48, state->ampm_mode?0.85f:1.05f, state->ampm_mode?0.85f:1.05f,
                      state->alarm_active?COL_RED:COL_TEXT, 200);
        C2D_DrawRectSolid(200, 30, 0.5f, 1, 140, COL_STRIPE);
        draw_centered(lang==0?"Próxima alarma:":"Next alarm:", 33, 0.40f, 0.40f, COL_TEXT, 200+400);
        if (alarm_h24>=0 && !state->alarm_active) {
            int dh,dm,ds;
            alarm_time_until(alarm_h24,alarm_m,t->tm_hour,t->tm_min,t->tm_sec,&dh,&dm,&ds);
            char alarm_time_str[16];
            alarm_format_short(alarm_time_str,sizeof(alarm_time_str),alarm_h24,alarm_m,state->ampm_mode,false);
            draw_centered(alarm_time_str, 48, 0.65f, 0.65f, COL_ACCENT, 200+400);
            snprintf(buf,sizeof(buf),"%02d %s",dh,lang==0?"horas":"hours");
            draw_centered(buf, 70, 0.42f, 0.42f, COL_TEXT, 200+400);
            snprintf(buf,sizeof(buf),"%02d %s",dm,lang==0?"minutos":"minutes");
            draw_centered(buf, 85, 0.42f, 0.42f, COL_TEXT, 200+400);
            snprintf(buf,sizeof(buf),"%02d %s",ds,lang==0?"segundos":"seconds");
            draw_centered(buf, 100, 0.42f, 0.42f, COL_TEXT, 200+400);
        } else if (state->alarm_active) {
            draw_centered(s(lang,S_ALARM_RINGING), 55, 0.46f, 0.46f, COL_RED, 200+400);
            draw_centered(s(lang,S_PRESS_A_STOP),  75, 0.38f, 0.38f, COL_RED, 200+400);
        } else {
            draw_centered(s(lang,S_NO_ALARMS), 65, 0.40f, 0.40f, COL_TEXT, 200+400);
        }
    }

    // Franja inferior pantalla superior: hint contextual
    C2D_DrawRectSolid(0, 210, 0.5f, SCREEN_W_TOP, 30, COL_STRIPE);
    {
        const char *hint = "";
        if (state->alarm_active) {
            hint = lang==0 ? "Presiona A para detener la alarma" : "Press A to stop the alarm";
        } else if (screen == SCREEN_MAIN) {
            const char *hints_es[] = {
                "Configura la hora, volumen y luz LED de tus alarmas",
                "Formato de hora, idioma y otras configuraciones",
                "¿Quién habrá hecho esta maravillosa app?"
            };
            const char *hints_en[] = {
                "Set the time, volume and LED light for your alarms",
                "Time format, language and other settings",
                "Who could have made this wonderful app?"
            };
            if (selected_opt>=0 && selected_opt<3)
                hint = lang==0 ? hints_es[selected_opt] : hints_en[selected_opt];
        } else if (screen == SCREEN_ALARM_LIST) {
            hint = lang==0 ? "Elige una alarma para configurar" : "Choose an alarm to configure";
        } else if (screen == SCREEN_ALARM_CONFIG) {
            if (!config_editing) {
                const char *hints_es[] = {
                    "Edita la hora a la que sonara la alarma",
                    "Ajusta el volumen. iSonara incluso con la consola cerrada o silenciada!",
                    "Activa un simple desafio para apagar la alarma",
                    "Elige el color del LED al sonar la alarma",
                    "Elige el patrón de luces para la alarma"
                };
                const char *hints_en[] = {
                    "Edit the time the alarm will ring",
                    "Adjust volume. It will ring even with the console closed or muted!",
                    "Enable a simple challenge to turn off the alarm",
                    "Choose the LED color when the alarm rings",
                    "Choose the LED pattern for the alarm"
                };
                if (config_opt>=0 && config_opt<5)
                    hint = lang==0 ? hints_es[config_opt] : hints_en[config_opt];
            } else {
                const char *hints_es[] = {
                    "Arr/Abj: valor  Izq/Der: campo  A: confirmar",
                    "Izq/Der: subir/bajar volumen  A: confirmar",
                    "",
                    "Izq/Der: cambiar color  A: confirmar",
                    "Izq/Der: cambiar patron  A: confirmar"
                };
                const char *hints_en[] = {
                    "Up/Dn: value  L/R: field  A: confirm",
                    "L/R: raise/lower volume  A: confirm",
                    "",
                    "L/R: change color  A: confirm",
                    "L/R: change pattern  A: confirm"
                };
                if (config_opt>=0 && config_opt<5)
                    hint = lang==0 ? hints_es[config_opt] : hints_en[config_opt];
            }
        } else if (screen == SCREEN_SETTINGS) {
            const char *hints_es[] = {
                "Cambia entre formato 12 horas y 24 horas",
                "Selecciona el idioma de la aplicacion",
                "Trabajo en proceso. Paciencia por favor =P"
            };
            const char *hints_en[] = {
                "Switch between 12-hour and 24-hour format",
                "Select the application language",
                "Work in progress. Please be patient =P"
            };
            if (selected_opt>=0 && selected_opt<3)
                hint = lang==0 ? hints_es[selected_opt] : hints_en[selected_opt];
        } else if (screen == SCREEN_SETTINGS_LANGUAGE) {
            hint = s(lang, S_LANG_SELECT_HINT);
        } else if (screen == SCREEN_CREDITS) {
            hint = lang==0 ? "iCreado con mucho sueño!" : "Made with lots of sleep!";
        }
        draw_top(hint, 216, 0.36f, 0.36f, COL_WHITE);
    }

    // ===== PANTALLA INFERIOR =====
    C2D_TargetClear(render_bot, COL_BG);
    C2D_SceneBegin(render_bot);
    stars_draw_bot();

    if (state->alarm_active) {
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_W_BOT, SCREEN_H, COL_DARKRED);
        if (!challenge_active) {
            draw_bot(s(lang,S_ALARM_SOUNDING), 80, 0.55f, 0.55f, COL_WHITE);
            draw_bot(s(lang,S_TURN_OFF_UNLOCK),105, 0.48f, 0.48f, COL_WHITE);
            draw_bot(s(lang,S_HOME_BUTTON),    125, 0.48f, 0.48f, COL_WHITE);
        } else {
            draw_bot(lang==0?"iPresiona la combinacion!":"Press the combination!", 30, 0.48f, 0.48f, COL_WHITE);
            {
                u32 btn_keys[8] = {KEY_A,KEY_B,KEY_X,KEY_Y,KEY_DUP,KEY_DDOWN,KEY_DLEFT,KEY_DRIGHT};
                float iy=68, bsz=26, gap=10;
                float total_w = 4*bsz + 3*(gap+8);
                float bx0 = (SCREEN_W_BOT - total_w)/2.0f;
                for(int bi=0;bi<4;bi++){
                    float bx = bx0 + bi*(bsz+gap+8);
                    int bidx=0;
                    for(int k=0;k<8;k++) if(btn_keys[k]==challenge_seq[bi]){bidx=k;break;}
                    u32 bg = (bi<challenge_step)?C2D_Color32(0,180,0,100)
                           : (bi==challenge_step)?C2D_Color32(255,200,0,100)
                           : C2D_Color32(50,50,100,100);
                    C2D_DrawRectSolid(bx-2, iy-2, 0.44f, bsz+4, bsz+4, bg);
                    switch(bidx){
                        case 0: ui_draw_btn_a(bx,iy,bsz); break;
                        case 1: ui_draw_btn_b(bx,iy,bsz); break;
                        case 2: ui_draw_btn_x(bx,iy,bsz); break;
                        case 3: ui_draw_btn_y(bx,iy,bsz); break;
                        case 4: ui_draw_dpad_up(bx,iy,bsz); break;
                        case 5: ui_draw_dpad_down(bx,iy,bsz); break;
                        case 6: ui_draw_dpad_left(bx,iy,bsz); break;
                        case 7: ui_draw_dpad_right(bx,iy,bsz); break;
                    }
                    if(bi<3){
                        C2D_Text ptxt; C2D_TextBufClear(render_tbuf);
                        C2D_TextFontParse(&ptxt, render_font, render_tbuf,"+"); C2D_TextOptimize(&ptxt);
                        C2D_DrawText(&ptxt,C2D_WithColor,bx+bsz+2,iy+6,0.5f,0.50f,0.50f,COL_WHITE);
                    }
                }
            }
            if(challenge_fail)
                draw_bot(lang==0?"iIncorrecto! Vuelve a intentarlo":"Wrong! Try again", 108, 0.44f, 0.44f, COL_RED);
            else {
                snprintf(buf,sizeof(buf),lang==0?"%d / 4 correctos":"%d / 4 correct",challenge_step);
                draw_bot(buf, 108, 0.44f, 0.44f, COL_TEXT);
            }
        }

    } else if (screen == SCREEN_ALARM_LIST) {
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_W_BOT, 22, COL_STRIPE);
        draw_bot(s(lang,S_ALARMS), 3, 0.55f, 0.55f, COL_WHITE);
        for (int i=0; i<MAX_ALARMS; i++)
            draw_alarm_card(state, i, i%3, i/3, i==selected_alarm_idx);
        C2D_DrawRectSolid(0, 210, 0.5f, SCREEN_W_BOT, 30, COL_STRIPE);
        { float _x=10;
          _x += ui_hint_a(_x, 216, 18, 3, "Config", 0.38f, 0.38f, COL_WHITE)+8;
          (void)_x; }

    } else if (screen == SCREEN_ALARM_CONFIG) {
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_W_BOT, 20, COL_STRIPE);
        snprintf(buf, sizeof(buf), s(lang,S_ALARM_N), selected_alarm_idx+1);
        draw_bot(buf, 3, 0.44f, 0.44f, COL_WHITE);
        int dh,dm,ds;
        alarm_time_until(edit_hour,edit_minute,t->tm_hour,t->tm_min,t->tm_sec,&dh,&dm,&ds);
        snprintf(buf, sizeof(buf), s(lang,S_RINGS_IN), dh, dm, ds);
        draw_bot(buf, 22, 0.38f, 0.38f, COL_TEXT);
        C2D_DrawRectSolid(158, 20, 0.5f, 1, 188, C2D_Color32(30,50,120,255));
        float rx = 160.0f, rw = 160.0f;

        bool hora_nav = (config_opt==0 && !config_editing);
        bool hora_edit= (config_opt==0 &&  config_editing);
        u32 hora_lbl = (hora_nav||hora_edit) ? COL_ACCENT : COL_TEXT;
        u32 hora_val = hora_edit ? COL_ACCENT : COL_WHITE;
        draw_centered(hora_nav?(lang==0?">> Hora <<":">> Time <<"):(lang==0?"Hora":"Time"),
                      38, 0.44f, 0.44f, hora_lbl, 160);
        if (hora_edit) {
            int disp_h=edit_hour;
            const char *disp_ampm=(edit_hour<12)?"AM":"PM";
            if(state->ampm_mode){disp_h=edit_hour%12;if(disp_h==0)disp_h=12;}
            if(!state->ampm_mode)
                snprintf(buf,sizeof(buf),"%s%02d%s:%s%02d%s",
                    edit_subfield==0?"[":"-",disp_h,edit_subfield==0?"]":"-",
                    edit_subfield==1?"[":"-",edit_minute,edit_subfield==1?"]":"-");
            else
                snprintf(buf,sizeof(buf),"%s%02d%s:%s%02d%s %s%s%s",
                    edit_subfield==0?"[":"-",disp_h,edit_subfield==0?"]":"-",
                    edit_subfield==1?"[":"-",edit_minute,edit_subfield==1?"]":"-",
                    edit_subfield==2?"[":"-",disp_ampm,edit_subfield==2?"]":"-");
            draw_centered(buf, 52, 0.78f, 0.78f, COL_ACCENT, 160);
        } else {
            alarm_format_short(buf,sizeof(buf),edit_hour,edit_minute,state->ampm_mode,edit_is_pm);
            draw_centered(buf, 52, 0.85f, 0.85f, hora_val, 160);
        }

        bool vol_nav  = (config_opt==1 && !config_editing);
        bool vol_edit = (config_opt==1 &&  config_editing);
        u32 vol_lbl = (vol_nav||vol_edit) ? COL_ACCENT : COL_TEXT;
        u32 vol_val = vol_edit ? COL_ACCENT : COL_WHITE;
        draw_centered(vol_nav?(lang==0?">> Volumen <<":">> Volume <<"):s(lang,S_VOLUME),
                      96, 0.44f, 0.44f, vol_lbl, 160);
        snprintf(buf,sizeof(buf),"%d%%",(int)roundf(edit_volume*100));
        {
            C2D_Text vtxt; C2D_TextBufClear(render_tbuf);
            C2D_TextFontParse(&vtxt, render_font, render_tbuf,buf); C2D_TextOptimize(&vtxt);
            float vw,vh; C2D_TextGetDimensions(&vtxt,0.60f,0.60f,&vw,&vh);
            float total=28+6+vw;
            float startx=(160-total)/2.0f;
            ui_draw_volume(startx, 106, 28);
            C2D_DrawText(&vtxt,C2D_WithColor,startx+28+6,110,0.5f,0.60f,0.60f,vol_val);
        }

        bool ch_nav2  = (config_opt==2 && !config_editing);
        u32 ch_lbl2 = ch_nav2 ? COL_ACCENT : COL_TEXT;
        draw_centered(ch_nav2?(lang==0?">> Desafío <<":">> Challenge <<"):(lang==0?"Desafío":"Challenge"),
                      150, 0.40f, 0.40f, ch_lbl2, 160);
        u32 ch_tog2 = edit_challenge ? COL_GREEN : C2D_Color32(100,100,120,255);
        C2D_DrawRectSolid(20, 162, 0.5f, 118, 16, ch_tog2);
        draw_centered(edit_challenge?(lang==0?"Activado":"On"):(lang==0?"Desactivado":"Off"),
                      164, 0.38f, 0.38f, COL_WHITE, 160);

        bool col_nav  = (config_opt==3 && !config_editing);
        bool col_edit = (config_opt==3 &&  config_editing);
        u32 clbl = (col_nav||col_edit) ? COL_ACCENT : COL_TEXT;
        u32 cval = col_edit ? COL_ACCENT : COL_WHITE;
        draw_centered(col_nav?(lang==0?">> Color LED <<":">> LED Color <<"):(lang==0?"Color LED":"LED Color"),
                      38, 0.40f, 0.40f, clbl, rx*2+rw);
        int cbox=26, cgap=3;
        float crow_x = rx + (rw-(3*cbox+2*cgap))/2.0f;
        for(int ci=0;ci<3;ci++){
            float bx=crow_x+ci*(cbox+cgap);
            bool sel=(edit_led_color==ci);
            u32 border=sel?(col_edit?COL_ACCENT:COL_WHITE):COL_STRIPE;
            C2D_DrawRectSolid(bx, 54, 0.5f, cbox, 22, border);
            C2D_DrawRectSolid(bx+2, 56, 0.5f, cbox-4, 18, LED_COLOR_UI[ci]);
        }
        for(int ci=3;ci<6;ci++){
            float bx=crow_x+(ci-3)*(cbox+cgap);
            bool sel=(edit_led_color==ci);
            u32 border=sel?(col_edit?COL_ACCENT:COL_WHITE):COL_STRIPE;
            C2D_DrawRectSolid(bx, 80, 0.5f, cbox, 22, border);
            C2D_DrawRectSolid(bx+2, 82, 0.5f, cbox-4, 18, LED_COLOR_UI[ci]);
        }
        draw_centered((lang==0?LED_COLOR_NAMES_ES:LED_COLOR_NAMES_EN)[edit_led_color],
                      105, 0.34f, 0.34f, cval, rx*2+rw);

        bool pat_nav  = (config_opt==4 && !config_editing);
        bool pat_edit = (config_opt==4 &&  config_editing);
        u32 plbl = (pat_nav||pat_edit) ? COL_ACCENT : COL_TEXT;
        u32 pval = pat_edit ? COL_ACCENT : COL_WHITE;
        draw_centered(pat_nav?(lang==0?">> Patrón LED <<":">> LED Pattern <<"):(lang==0?"Patrón LED":"LED Pattern"),
                      142, 0.38f, 0.38f, plbl, rx*2+rw);
        draw_centered((lang==0?LED_PATTERN_NAMES_ES:LED_PATTERN_NAMES_EN)[edit_led_pattern],
                      158, 0.58f, 0.58f, pval, rx*2+rw);

        if(duplicate_error)
            draw_bot(s(lang,S_DUPLICATE_ERR), 192, 0.34f, 0.34f, COL_RED);
        C2D_DrawRectSolid(0, 210, 0.5f, SCREEN_W_BOT, 30, COL_STRIPE);
        {float _x=10;
         _x+=ui_hint_a(_x,216,18,3,config_editing?(lang==0?"Confirmar":"Confirm"):(lang==0?"Editar":"Edit"),0.36f,0.36f,COL_WHITE)+6;
         _x+=ui_hint_b(_x,216,18,3,config_editing?(lang==0?"Cancelar":"Cancel"):(lang==0?"Volver":"Back"),0.36f,0.36f,COL_WHITE)+6;
         if(!config_editing) draw_bot(lang==0?"Start: Listo":"Start: Done",216,0.36f,0.36f,COL_TEXT);}

    } else if (screen == SCREEN_SETTINGS) {
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_W_BOT, 30, COL_STRIPE);
        draw_bot(s(lang,S_SETTINGS_TITLE), 5, 0.6f, 0.6f, COL_WHITE);
        C2D_DrawRectSolid(10,50,0.5f,300,47,selected_opt==0?COL_STRIPE:COL_BG);
        draw_bot(s(lang,S_TIME_FORMAT),55,0.50f,0.50f,COL_WHITE);
        draw_bot(state->ampm_mode?s(lang,S_FORMAT_AMPM):s(lang,S_FORMAT_24H),72,0.50f,0.50f,COL_TEXT);
        C2D_DrawRectSolid(10,102,0.5f,300,47,selected_opt==1?COL_STRIPE:COL_BG);
        draw_bot(s(lang,S_LANGUAGE),107,0.50f,0.50f,COL_WHITE);
        // Nombre del idioma siempre en su propio idioma
        draw_bot(state->language==0?"Español":"English",124,0.50f,0.50f,COL_TEXT);
        C2D_DrawRectSolid(10,154,0.5f,300,47,selected_opt==2?COL_STRIPE:COL_BG);
        draw_bot(lang==0?"Diseño de pantalla":"Screen layout",159,0.50f,0.50f,COL_WHITE);
        draw_bot(lang==0?"Trabajando en ello...":"Work in progress...",176,0.50f,0.50f,COL_TEXT);
        C2D_DrawRectSolid(0,210,0.5f,SCREEN_W_BOT,30,COL_STRIPE);
        {float _x=10;_x+=ui_hint_dpad_ud(_x,216,18,3,"",0.38f,0.38f,COL_WHITE)+4;_x+=ui_hint_a(_x,216,18,3,lang==0?"Selec.":"Select",0.38f,0.38f,COL_WHITE)+10;ui_hint_b(_x,216,18,3,lang==0?"Volver":"Back",0.38f,0.38f,COL_WHITE);}

    } else if (screen == SCREEN_SETTINGS_LANGUAGE) {
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_W_BOT, 30, COL_STRIPE);
        draw_bot(s(lang, S_LANG_SETTINGS_TITLE), 5, 0.52f, 0.52f, COL_WHITE);
        // "Español" y "English" siempre fijos en su propio idioma
        const char *lang_names[2] = {"Español", "English"};
        for (int i=0; i<2; i++) {
            float ly = 60.0f + i * 60.0f;
            bool sel = (i == selected_opt);
            C2D_DrawRectSolid(20, ly, 0.5f, 280, 46, sel ? COL_STRIPE : COL_BG);
            if (sel) C2D_DrawRectSolid(20, ly, 0.5f, 4, 46, COL_ACCENT);
            draw_bot(lang_names[i], ly+12, 0.58f, 0.58f, sel ? COL_ACCENT : COL_WHITE);
        }
        C2D_DrawRectSolid(0,210,0.5f,SCREEN_W_BOT,30,COL_STRIPE);
        {float _x=10;_x+=ui_hint_a(_x,216,18,3,lang==0?"Seleccionar":"Select",0.38f,0.38f,COL_WHITE)+8;ui_hint_b(_x,216,18,3,lang==0?"Volver":"Back",0.38f,0.38f,COL_WHITE);}

    } else if (screen == SCREEN_CREDITS) {
        C2D_DrawRectSolid(0,0,0.5f,SCREEN_W_BOT,30,COL_STRIPE);
        draw_bot(s(lang,S_CREDITS_TITLE),5,0.6f,0.6f,COL_WHITE);
        draw_bot("3DS Alarming Clock",40,0.56f,0.56f,COL_WHITE);
        draw_bot(lang==0?"Creado por: Joc3D":"Created by: Joc3D",58,0.50f,0.50f,COL_ACCENT);
        draw_bot(lang==0?"Desarrollado con DevKitPro":"Developed with DevKitPro",78,0.44f,0.44f,COL_TEXT);
        draw_bot(lang==0?"Gracias especiales a PabloMK7":"Special thanks to PabloMK7",98,0.44f,0.44f,COL_TEXT);
        draw_bot(lang==0?"por libncsnd por permitir sonido con la pantalla cerrada =D":"for libncsnd for allowing sound with the screen closed =D",116,0.38f,0.38f,COL_WHITE);
        C2D_DrawRectSolid(0,210,0.5f,SCREEN_W_BOT,30,COL_STRIPE);
        {float _x=10;ui_hint_b(_x,216,18,3,lang==0?"Volver":"Back",0.38f,0.38f,COL_WHITE);}

    } else {
        // SCREEN_MAIN: mismo tamaño que Ajustes (47px, y=50/102/154)
        C2D_DrawRectSolid(0,0,0.5f,SCREEN_W_BOT,30,COL_STRIPE);
        draw_bot(s(lang,S_MENU),5,0.6f,0.6f,COL_WHITE);
        int opts_y[3]={50,102,154};
        const char *opts_title[3]={s(lang,S_ALARMS),s(lang,S_SETTINGS),s(lang,S_CREDITS)};
        int ena=0;
        for(int i=0;i<MAX_ALARMS;i++) if(state->alarms[i].enabled) ena++;
        char alarms_sub[32];
        if(ena==0) snprintf(alarms_sub,sizeof(alarms_sub),"%s",lang==0?"Sin alarmas activas":"No active alarms");
        else if(lang==0) snprintf(alarms_sub,sizeof(alarms_sub),"%d activa%s",ena,ena==1?"":"s");
        else snprintf(alarms_sub,sizeof(alarms_sub),"%d active alarm%s",ena,ena==1?"":"s");
        const char *opts_sub[3]={alarms_sub, s(lang,S_SETTINGS_SUB), s(lang,S_CREDITS_SUB)};
        for(int i=0;i<3;i++){
            C2D_DrawRectSolid(10,opts_y[i],0.5f,300,47,selected_opt==i?COL_STRIPE:COL_BG);
            draw_bot(opts_title[i],opts_y[i]+5,0.50f,0.50f,COL_WHITE);
            draw_bot(opts_sub[i],opts_y[i]+22,0.42f,0.42f,COL_TEXT);
        }
        C2D_DrawRectSolid(0,210,0.5f,SCREEN_W_BOT,30,COL_STRIPE);
        {float _x=10;_x+=ui_hint_dpad_ud(_x,216,18,3,"",0.38f,0.38f,COL_WHITE)+4;ui_hint_a(_x,216,18,3,lang==0?"Selec. / Toca":"Select / Tap",0.38f,0.38f,COL_WHITE);}
    }

    if (fade_tick < fade_frames) {
        float alpha=1.0f-(float)fade_tick/fade_frames;
        u8 a=(u8)(alpha*255.0f);
        u32 col=C2D_Color32(0,0,0,a);
        C2D_SceneBegin(render_top);
        C2D_DrawRectSolid(0,0,0.9f,SCREEN_W_TOP,SCREEN_H,col);
        C2D_SceneBegin(render_bot);
        C2D_DrawRectSolid(0,0,0.9f,SCREEN_W_BOT,SCREEN_H,col);
    }
    C3D_FrameEnd(0);
}
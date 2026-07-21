#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "audio.h"
#include "render.h"
#include "stars.h"
#include "alarm.h"
#include "config.h"
#include "ui.h"
#include "led.h"

#define FADE_FRAMES 72

int main(void) {
    srand(42);
    gfxInitDefault();
    render_init();
    stars_init();

    aptSetSleepAllowed(false);
    ndmuInit();
    ptmuInit();
    NDMU_EnterExclusiveState(NDM_EXCLUSIVE_STATE_INFRASTRUCTURE);
    NDMU_LockState();
    osSetSpeedupEnable(false);
    audio_init();
    ui_init();
    led_init();

    AppState state;
    alarm_init(&state);
    config_load(&state);

    AppScreen screen      = SCREEN_MAIN;
    int  selected_opt     = 0;
    int  selected_alarm   = 0;
    int  config_opt       = 0;
    bool config_editing   = false;
    int  edit_subfield    = 0;
    int  edit_hour        = 7, edit_minute = 0;
    bool edit_is_pm       = false;
    float edit_volume     = 1.0f;
    int   edit_led_color   = 0;
    int   edit_led_pattern = 0;
    bool  edit_challenge   = false;
    bool duplicate_error  = false;

    #define CHALLENGE_LEN 4
    u32   challenge_seq[CHALLENGE_LEN];
    int   challenge_step  = 0;
    bool  challenge_active = false;
    bool  challenge_fail   = false;
    int   challenge_fail_timer = 0;

    bool running      = true;
    int  tick         = 0;
    int  fade_tick    = 0;
    bool fading_in    = true;
    bool jingle_played = false;

    int repeat_key=0, repeat_timer=0;
    #define REPEAT_DELAY    15
    #define REPEAT_INTERVAL  5

    touchPosition touch_prev = {0,0};
    bool touching = false;

    while (running && aptMainLoop()) {
        hidScanInput();
        u32 keys      = hidKeysDown();
        u32 keys_held = hidKeysHeld();
        touchPosition touch;
        hidTouchRead(&touch);

        if (!aptIsActive()) audio_alarm_stop();

        if (fading_in) {
            if (!jingle_played) { sfx_play_jingle(); jingle_played = true; }
            fade_tick++;
            if (fade_tick >= FADE_FRAMES) fading_in = false;
        } else {
            int touch_dy = 0;
            if (keys_held & KEY_TOUCH) {
                if (touching) touch_dy = (int)touch_prev.py - (int)touch.py;
                touch_prev = touch; touching = true;
            } else { touching = false; }

            if (state.alarm_active) {
                if (!challenge_active && (keys & KEY_A)) {
                    sfx_play_select();
                    audio_alarm_stop();
                    alarm_stop_active(&state);
                    led_clear();
                    aptSetHomeAllowed(true);
                }
            } else if (screen == SCREEN_MAIN) {
                if (keys & KEY_DUP)   { selected_opt=(selected_opt+2)%3; sfx_play_move(); }
                if (keys & KEY_DDOWN) { selected_opt=(selected_opt+1)%3; sfx_play_move(); }
                if (keys & KEY_TOUCH) {
                    int opts_y[3]={50,102,154};
                    for (int i=0;i<3;i++) {
                        if (touch.py>=opts_y[i]&&touch.py<opts_y[i]+47) {
                            sfx_play_select(); selected_opt=i;
                            if      (i==0){screen=SCREEN_ALARM_LIST;selected_alarm=0;}
                            else if (i==1){screen=SCREEN_SETTINGS;selected_opt=0;}
                            else if (i==2){screen=SCREEN_CREDITS;}
                            break;
                        }
                    }
                }
                if (keys & KEY_A) {
                    sfx_play_select();
                    if      (selected_opt==0){screen=SCREEN_ALARM_LIST;selected_alarm=0;}
                    else if (selected_opt==1){screen=SCREEN_SETTINGS;selected_opt=0;}
                    else if (selected_opt==2){screen=SCREEN_CREDITS;}
                }

            } else if (screen == SCREEN_ALARM_LIST) {
                if (keys & KEY_DLEFT)  {selected_alarm=(selected_alarm+MAX_ALARMS-1)%MAX_ALARMS;sfx_play_move();}
                if (keys & KEY_DRIGHT) {selected_alarm=(selected_alarm+1)%MAX_ALARMS;sfx_play_move();}
                if (keys & KEY_DUP)    {selected_alarm=(selected_alarm+MAX_ALARMS-3)%MAX_ALARMS;sfx_play_move();}
                if (keys & KEY_DDOWN)  {selected_alarm=(selected_alarm+3)%MAX_ALARMS;sfx_play_move();}
                if (keys & KEY_TOUCH) {
                    int cw=100,ch=86,mx=5,my=26;
                    for (int i=0;i<MAX_ALARMS;i++){
                        int col=i%3,row=i/3;
                        int x=mx+col*(cw+5),y=my+row*(ch+4);
                        int tog_y=y+ch-24;
                        if(touch.px>=x+10&&touch.px<=x+cw-10&&touch.py>=tog_y&&touch.py<=tog_y+18){
                            state.alarms[i].enabled=!state.alarms[i].enabled;sfx_play_select();config_save(&state);
                        } else if(touch.px>=x&&touch.px<=x+cw&&touch.py>=y&&touch.py<=tog_y){
                            selected_alarm=i;
                            Alarm *a=&state.alarms[i];
                            edit_hour=alarm_to_24h(a->hour,a->is_pm,state.ampm_mode);
                            edit_minute=a->minute; edit_is_pm=a->is_pm; edit_volume=a->volume;
                            edit_led_color=a->led_color; edit_led_pattern=a->led_pattern;
                            edit_challenge=a->challenge_enabled;
                            config_opt=0; config_editing=false; edit_subfield=0; duplicate_error=false;
                            sfx_play_select();
                            screen=SCREEN_ALARM_CONFIG;
                        }
                    }
                }
                if (keys & KEY_A) {
                    sfx_play_select();
                    Alarm *a=&state.alarms[selected_alarm];
                    edit_hour=alarm_to_24h(a->hour,a->is_pm,state.ampm_mode);
                    edit_minute=a->minute; edit_is_pm=a->is_pm; edit_volume=a->volume;
                    edit_led_color=a->led_color; edit_led_pattern=a->led_pattern;
                    edit_challenge=a->challenge_enabled;
                    config_opt=0; config_editing=false; edit_subfield=0; duplicate_error=false;
                    screen=SCREEN_ALARM_CONFIG;
                }
                if (keys & KEY_B){sfx_play_back();screen=SCREEN_MAIN;}

            } else if (screen == SCREEN_ALARM_CONFIG) {
                if (!config_editing) {
                    if(keys&KEY_DUP){
                        sfx_play_move(); duplicate_error=false;
                        if(config_opt==0)      config_opt=2;
                        else if(config_opt==1) config_opt=0;
                        else if(config_opt==2) config_opt=1;
                        else if(config_opt==3) config_opt=4;
                        else if(config_opt==4) config_opt=3;
                    }
                    if(keys&KEY_DDOWN){
                        sfx_play_move(); duplicate_error=false;
                        if(config_opt==0)      config_opt=1;
                        else if(config_opt==1) config_opt=2;
                        else if(config_opt==2) config_opt=0;
                        else if(config_opt==3) config_opt=4;
                        else if(config_opt==4) config_opt=3;
                    }
                    if(keys&KEY_DLEFT||keys&KEY_DRIGHT){
                        sfx_play_move(); duplicate_error=false;
                        if(config_opt<=2) config_opt=3;
                        else              config_opt=0;
                    }
                    if(keys&KEY_TOUCH){
                        if(touch.px<158){
                            if(touch.py>=38&&touch.py<96){config_opt=0;sfx_play_move();}
                            else if(touch.py>=96&&touch.py<150){config_opt=1;sfx_play_move();}
                            else if(touch.py>=150&&touch.py<182){
                                config_opt=2;
                                edit_challenge=!edit_challenge;
                                state.alarms[selected_alarm].challenge_enabled=edit_challenge;
                                config_save(&state); sfx_play_select();
                            }
                        } else {
                            if(touch.py>=38&&touch.py<110){
                                config_opt=3; sfx_play_move();
                                float crow_x2=158+(160-(3*26+2*3))/2.0f;
                                int ci=-1;
                                for(int c=0;c<3;c++){
                                    float bx=crow_x2+c*(26+3);
                                    if(touch.px>=bx&&touch.px<bx+26){
                                        if(touch.py>=54&&touch.py<76) ci=c;
                                        else if(touch.py>=80&&touch.py<102) ci=c+3;
                                    }
                                }
                                if(ci>=0&&ci<6){edit_led_color=ci;led_set_solid(edit_led_color);sfx_play_adjust();}
                            } else if(touch.py>=140&&touch.py<182){
                                config_opt=4; sfx_play_move();
                            }
                        }
                    }
                    if(keys&KEY_A){
                        sfx_play_select();duplicate_error=false;
                        if(config_opt==2){
                            edit_challenge=!edit_challenge;
                            state.alarms[selected_alarm].challenge_enabled=edit_challenge;
                            config_save(&state);
                        } else {
                            config_editing=true; edit_subfield=0;
                            if(config_opt==1) audio_alarm_start(edit_volume);
                            else if(config_opt==3) led_set_solid(edit_led_color);
                            else if(config_opt==4) led_set(edit_led_color, edit_led_pattern);
                        }
                    }
                    if(keys&KEY_B){sfx_play_back();screen=SCREEN_ALARM_LIST;}
                } else {
                    if(config_opt==0) {
                        int max_sub=state.ampm_mode?3:2;
                        if(keys&KEY_DLEFT) {edit_subfield=(edit_subfield+max_sub-1)%max_sub;sfx_play_move();}
                        if(keys&KEY_DRIGHT){edit_subfield=(edit_subfield+1)%max_sub;sfx_play_move();}
                        if(keys_held&(KEY_DUP|KEY_DDOWN)){int cur=(keys_held&KEY_DUP)?KEY_DUP:KEY_DDOWN;if(repeat_key!=cur){repeat_key=cur;repeat_timer=0;}else repeat_timer++;}else{repeat_key=0;repeat_timer=0;}
                        int delta=0;
                        if(keys&KEY_DUP)delta=1; else if(keys&KEY_DDOWN)delta=-1;
                        if(delta==0&&repeat_key&&repeat_timer>=REPEAT_DELAY&&(repeat_timer-REPEAT_DELAY)%REPEAT_INTERVAL==0)delta=(repeat_key==KEY_DUP)?1:-1;
                        if(touch_dy>10)delta=1; else if(touch_dy<-10)delta=-1;
                        if(delta!=0){sfx_play_adjust();if(edit_subfield==0){edit_hour=(edit_hour+delta+24)%24;}else if(edit_subfield==1){edit_minute=(edit_minute+delta+60)%60;}else{if(edit_hour<12)edit_hour+=12;else edit_hour-=12;}}
                        if(keys&KEY_A){int eh24=edit_hour;if(alarm_has_duplicate(&state,selected_alarm,eh24,edit_minute)){duplicate_error=true;sfx_play_back();}else{sfx_play_select();Alarm*a=&state.alarms[selected_alarm];a->hour=edit_hour;a->minute=edit_minute;a->is_pm=(edit_hour>=12);a->used=false;config_save(&state);config_editing=false;duplicate_error=false;}}
                    } else if(config_opt==1) {
                        if(keys_held&(KEY_DLEFT|KEY_DRIGHT)){int cur=(keys_held&KEY_DRIGHT)?KEY_DRIGHT:KEY_DLEFT;if(repeat_key!=cur){repeat_key=cur;repeat_timer=0;}else repeat_timer++;}else{repeat_key=0;repeat_timer=0;}
                        int delta=0;
                        if(keys&KEY_DRIGHT)delta=1; else if(keys&KEY_DLEFT)delta=-1;
                        if(delta==0&&repeat_key&&repeat_timer>=REPEAT_DELAY&&(repeat_timer-REPEAT_DELAY)%REPEAT_INTERVAL==0)delta=(repeat_key==KEY_DRIGHT)?1:-1;
                        if(touch_dy>10)delta=1; else if(touch_dy<-10)delta=-1;
                        if(delta!=0){sfx_play_adjust();int vi=(int)roundf(edit_volume*100.0f);vi=((vi/5)+delta)*5;if(vi>100)vi=100;if(vi<0)vi=0;edit_volume=vi/100.0f;audio_alarm_stop();audio_alarm_start(edit_volume);}
                        if(keys&KEY_A){sfx_play_select();audio_alarm_stop();state.alarms[selected_alarm].volume=edit_volume;config_save(&state);config_editing=false;}
                    } else if(config_opt==3) {
                        if(keys&KEY_DRIGHT){ sfx_play_adjust(); edit_led_color=(edit_led_color%3==2)?edit_led_color-2:edit_led_color+1; led_set_solid(edit_led_color); }
                        if(keys&KEY_DLEFT){  sfx_play_adjust(); edit_led_color=(edit_led_color%3==0)?edit_led_color+2:edit_led_color-1; led_set_solid(edit_led_color); }
                        if(keys&KEY_DUP||keys&KEY_DDOWN){ sfx_play_adjust(); edit_led_color=(edit_led_color<3)?edit_led_color+3:edit_led_color-3; led_set_solid(edit_led_color); }
                        if(keys&KEY_A){sfx_play_select();led_clear();state.alarms[selected_alarm].led_color=edit_led_color;config_save(&state);config_editing=false;}
                    } else if(config_opt==4) {
                        int delta=0;
                        if(keys&KEY_DRIGHT)delta=1; else if(keys&KEY_DLEFT)delta=-1;
                        if(delta!=0){sfx_play_adjust();edit_led_pattern=(edit_led_pattern+delta+LED_PATTERN_COUNT)%LED_PATTERN_COUNT;led_set(edit_led_color,edit_led_pattern);}
                        if(keys&KEY_A){sfx_play_select();led_clear();state.alarms[selected_alarm].led_pattern=edit_led_pattern;config_save(&state);config_editing=false;}
                    }
                    if(keys&KEY_B){
                        sfx_play_back();audio_alarm_stop();led_clear();repeat_key=0;repeat_timer=0;
                        Alarm*a=&state.alarms[selected_alarm];
                        edit_hour=alarm_to_24h(a->hour,a->is_pm,state.ampm_mode);
                        edit_minute=a->minute;edit_is_pm=a->is_pm;edit_volume=a->volume;
                        edit_led_color=a->led_color;edit_led_pattern=a->led_pattern;
                        edit_challenge=a->challenge_enabled;
                        config_editing=false;duplicate_error=false;
                    }
                }
                if(keys&KEY_START && !config_editing){
                    sfx_play_select();
                    Alarm*a=&state.alarms[selected_alarm];
                    a->hour=edit_hour; a->minute=edit_minute; a->is_pm=(edit_hour>=12);
                    a->volume=edit_volume; a->led_color=edit_led_color;
                    a->led_pattern=edit_led_pattern; a->challenge_enabled=edit_challenge;
                    a->used=false;
                    config_save(&state);
                    screen=SCREEN_ALARM_LIST;
                }

            } else if (screen == SCREEN_SETTINGS) {
                #define SET_OPTS 3
                if(keys&KEY_DUP)  {selected_opt=(selected_opt+SET_OPTS-1)%SET_OPTS;sfx_play_move();}
                if(keys&KEY_DDOWN){selected_opt=(selected_opt+1)%SET_OPTS;sfx_play_move();}
                if(keys&KEY_TOUCH){
                    if(touch.py>=50&&touch.py<=97){state.ampm_mode=!state.ampm_mode;config_save(&state);sfx_play_select();}
                    else if(touch.py>=102&&touch.py<=149){sfx_play_select();screen=SCREEN_SETTINGS_LANGUAGE;selected_opt=state.language;}
                    // touch.py>=154: diseño de pantalla bloqueado, no hace nada
                }
                if(keys&KEY_A){
                    if(selected_opt==0){sfx_play_select();state.ampm_mode=!state.ampm_mode;config_save(&state);}
                    else if(selected_opt==1){sfx_play_select();screen=SCREEN_SETTINGS_LANGUAGE;selected_opt=state.language;}
                    // selected_opt==2: bloqueado, no hace nada
                }
                if(keys&KEY_B){sfx_play_back();screen=SCREEN_MAIN;selected_opt=1;}

            } else if (screen == SCREEN_SETTINGS_LANGUAGE) {
                if(keys&KEY_DUP||keys&KEY_DDOWN){selected_opt=(selected_opt+1)%2;sfx_play_move();}
                if(keys&KEY_TOUCH){
                    if(touch.py>=60&&touch.py<=106){selected_opt=0;sfx_play_move();}
                    else if(touch.py>=120&&touch.py<=166){selected_opt=1;sfx_play_move();}
                }
                if(keys&KEY_A){sfx_play_select();state.language=selected_opt;config_save(&state);screen=SCREEN_SETTINGS;selected_opt=1;}
                if(keys&KEY_B){sfx_play_back();screen=SCREEN_SETTINGS;selected_opt=1;}

            } else if (screen == SCREEN_CREDITS) {
                if(keys&KEY_B){sfx_play_back();screen=SCREEN_MAIN;selected_opt=2;}
            }

            if(tick%60==0) alarm_check(&state);

            if(state.alarm_active){
                aptSetHomeAllowed(false);
                if(state.active_alarm_idx>=0){
                    Alarm*a=&state.alarms[state.active_alarm_idx];
                    audio_alarm_start(a->volume);
                    if(a->challenge_enabled && !challenge_active){
                        challenge_active=true; challenge_step=0; challenge_fail=false;
                        u32 btns[]={KEY_A,KEY_B,KEY_X,KEY_Y,KEY_DUP,KEY_DDOWN,KEY_DLEFT,KEY_DRIGHT};
                        for(int ci=0;ci<CHALLENGE_LEN;ci++) challenge_seq[ci]=btns[rand()%8];
                    }
                }
                if(tick%300==1){ Alarm*la=&state.alarms[state.active_alarm_idx]; led_set(la->led_color,la->led_pattern); }
                if(challenge_active){
                    if(challenge_fail_timer>0){
                        challenge_fail_timer--;
                        if(challenge_fail_timer==0){challenge_fail=false;challenge_step=0;}
                    } else if(keys){
                        if(keys & challenge_seq[challenge_step]){
                            sfx_play_move(); challenge_step++;
                            if(challenge_step>=CHALLENGE_LEN){
                                challenge_active=false; sfx_play_select();
                                audio_alarm_stop(); alarm_stop_active(&state);
                                led_clear(); aptSetHomeAllowed(true);
                            }
                        } else if(keys != 0){
                            sfx_play_back(); challenge_fail=true; challenge_fail_timer=45;
                        }
                    }
                }
            } else {
                challenge_active=false; challenge_step=0; challenge_fail=false;
            }
        }

        render_frame(screen,&state,selected_opt,
                     config_opt,config_editing,edit_subfield,
                     edit_hour,edit_minute,edit_is_pm,edit_volume,
                     fade_tick,FADE_FRAMES,
                     selected_alarm,duplicate_error,
                     tick,
                     edit_led_color,edit_led_pattern,edit_challenge,
                     challenge_active,challenge_seq,challenge_step,challenge_fail);
        tick++;
    }

    config_save(&state);
    led_exit();
    ui_exit();
    NDMU_UnlockState();
    NDMU_LeaveExclusiveState();
    ndmuExit();
    ptmuExit();
    aptSetSleepAllowed(true);
    aptSetHomeAllowed(true);
    osSetSpeedupEnable(true);
    audio_exit();
    render_exit();
    gfxExit();
    return 0;
}
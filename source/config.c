#include "config.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#define CONFIG_PATH "sdmc:/3ds/3DSAlarmingClock/config.bin"
#define CONFIG_MAGIC 0xAC111

typedef struct {
    u32      magic;
    AppState state;
} ConfigFile;

void config_load(AppState *state) {
    FILE *f = fopen(CONFIG_PATH, "rb");
    if (!f) return;
    ConfigFile cf;
    if (fread(&cf, sizeof(ConfigFile), 1, f) == 1 && cf.magic == CONFIG_MAGIC)
        memcpy(state, &cf.state, sizeof(AppState));
    fclose(f);
}

void config_save(const AppState *state) {
    mkdir("sdmc:/3ds/3DSAlarmingClock", 0777);
    FILE *f = fopen(CONFIG_PATH, "wb");
    if (!f) return;
    ConfigFile cf;
    cf.magic = CONFIG_MAGIC;
    memcpy(&cf.state, state, sizeof(AppState));
    fwrite(&cf, sizeof(ConfigFile), 1, f);
    fclose(f);
}
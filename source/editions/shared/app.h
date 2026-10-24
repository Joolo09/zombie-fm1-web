/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#include "edition.h"
#define APP_SECOND_CORE
#include "plat.h"
#define APP_NAME Z_EDITION_TITLE
#define APP_BLOCK 256u
#define APP_SPLASH_RGB RGB(154,224,86)
#define APP_TOUCH
#define APP_SYSEX
#define APP_SPLASH
enum {OBJ_SETTINGS,OBJ_PANEL,OBJ_BLE,OBJ_SYSTEM,OBJ_SAMPLER,OBJ_NOBJ};
/* Own project at D8000; retain platform calibration/Bluetooth sector pairs. */
#define APP_STORE_MAP {0xD8000u,0xDC000u,0xDE000u,0xDA000u,0xD0000u}
void app_init(void);
void app_render(int32_t *,uint32_t,uint32_t);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *,const char *);
void app_splash(void);
int app_sysex(const uint8_t *,uint32_t);
#ifdef PLAT_HOST
#define APP_HOST_CMD
#define APP_HOST_EXPECT
int app_host_cmd(const char *,const char *,const char *,const char *,const char *);
int app_expect(const char *,int *);
#endif

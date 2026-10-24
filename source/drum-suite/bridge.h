/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
uint32_t h_platform_ms(void);
uint32_t h_platform_lock(void);
void h_platform_unlock(uint32_t);
uint16_t *h_platform_canvas(void);
void h_platform_fill(uint32_t,uint32_t,uint32_t,uint32_t,uint16_t);
void h_platform_blit(uint32_t,uint32_t,uint32_t,uint32_t,const uint16_t *);
void h_platform_sync(void);
const uint8_t *h_platform_data(uint32_t);
int h_platform_read(uint32_t,void *,uint32_t);
int h_platform_erase(uint32_t);
int h_platform_write(uint32_t,const void *,uint32_t);
void h_platform_midi(uint32_t);
void h_init(void);
void h_render(float *,uint32_t);
void h_input(uint32_t,uint32_t,const int32_t *);
void h_draw(void);
void h_transport(uint32_t);
void h_tempo(uint32_t);
uint32_t h_bpm(void);
void h_midi(uint32_t,uint32_t);
uint32_t h_leds(void);
uint32_t h_key_leds(void);
void h_unfocus(void);
uint32_t h_models(void);
uint32_t h_presets(void);
uint32_t h_requested_playing(void);
void h_select(int32_t);
int h_save(void);
uint32_t h_has_project(void);
void h_legacy_begin(uint32_t);
void h_legacy_step(uint32_t,uint32_t,uint32_t);
void h_channel(uint32_t);
int h_remote(uint32_t,uint32_t,int32_t);
int h_set(uint32_t,uint32_t,int32_t);
int h_get(uint32_t,uint32_t);
int h_set_global(uint32_t,int32_t);
int h_set_step(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,int32_t);
int h_load_preset(uint32_t,uint32_t);
void h_hit(uint32_t,uint32_t);
uint32_t h_pack(uint8_t *);
int h_unpack(const uint8_t *,uint32_t);

void h_remote_release(void);

#ifdef PLAT_HOST
int h_host_check(void);
#endif

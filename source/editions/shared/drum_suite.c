/* SPDX-License-Identifier: GPL-3.0-only */
#include "model.h"
#if !Z_SAMPLER_EDITION
#ifdef Z_WASM
#include "../../drum-suite/bridge.h"
static uint8_t h_wasm_flash[0x100000];
static uint16_t h_wasm_canvas[240*144],h_wasm_frame[240*240];
static uint32_t h_wasm_ready,h_wasm_frames,h_wasm_dirty;
static uint32_t h_wasm_buttons,h_wasm_keys;
uint32_t h_platform_ms(void){return (uint32_t)((uint64_t)h_wasm_frames*1000u/44100u);}
uint32_t h_platform_lock(void){return 0;}
void h_platform_unlock(uint32_t f){(void)f;}
uint16_t *h_platform_canvas(void){return h_wasm_canvas;}
static void h_wasm_flash_init(void){if(!h_wasm_ready){for(uint32_t i=0;i<sizeof h_wasm_flash;i++)h_wasm_flash[i]=255;h_wasm_ready=1;}}
void h_platform_fill(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint16_t c){for(uint32_t j=y;j<y+h&&j<240;j++)for(uint32_t i=x;i<x+w&&i<240;i++)h_wasm_frame[j*240+i]=c;}
void h_platform_blit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t *p){for(uint32_t j=0;j<h&&y+j<240;j++)for(uint32_t i=0;i<w&&x+i<240;i++){uint16_t c=p[j*w+i];h_wasm_frame[(y+j)*240+x+i]=(uint16_t)((c>>8)|(c<<8));}}
void h_platform_sync(void){}
const uint8_t *h_platform_data(uint32_t off){h_wasm_flash_init();return off<sizeof h_wasm_flash?h_wasm_flash+off:0;}
int h_platform_read(uint32_t off,void *dst,uint32_t n){h_wasm_flash_init();if(off+n<off||off+n>sizeof h_wasm_flash)return -1;for(uint32_t i=0;i<n;i++)((uint8_t*)dst)[i]=h_wasm_flash[off+i];return 0;}
static int h_wasm_range(uint32_t off,uint32_t n){return off+n>=off&&((off>=0x97000u&&off+n<=0xD0000u)||(off>=0xD2000u&&off+n<=0xD8000u));}
int h_platform_erase(uint32_t off){if(!h_wasm_range(off,4096)||off%4096)return -1;h_wasm_flash_init();for(uint32_t i=0;i<4096;i++)h_wasm_flash[off+i]=255;h_wasm_dirty=1;return 0;}
int h_platform_write(uint32_t off,const void *p,uint32_t n){if(!h_wasm_range(off,n))return -1;h_wasm_flash_init();for(uint32_t i=0;i<n;i++)h_wasm_flash[off+i]&=((const uint8_t*)p)[i];h_wasm_dirty=1;return 0;}
void h_platform_midi(uint32_t p){(void)p;}
#endif
#include "../../drum-suite/runtime.c"
#ifdef Z_WASM
uint32_t h_frame(void){return (uint32_t)(uintptr_t)h_wasm_frame;}
uint32_t h_flash(void){h_wasm_flash_init();return (uint32_t)(uintptr_t)h_wasm_flash;}
uint32_t h_flash_dirty(void){uint32_t d=h_wasm_dirty;h_wasm_dirty=0;return d;}
void h_ui_tick(void){int32_t enc[7]={0};h_input(0,0,enc);h_draw();}
void h_button(uint32_t b,uint32_t on){h_remote(0,b,(int32_t)on);h_draw();}
void h_key(uint32_t k,uint32_t on){h_remote(1,k,(int32_t)on);h_draw();}
void h_knob(uint32_t role,int32_t value){h_remote(2,role,value);h_draw();}
void h_time(uint32_t ms){h_wasm_frames=(uint32_t)((uint64_t)ms*44100u/1000u);}

#endif
#endif

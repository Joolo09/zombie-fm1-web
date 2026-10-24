/* SPDX-License-Identifier: GPL-3.0-only
 * Zombie FM-1 bridge around Hortator by DEADACTIVE / Huegelton.
 * DSP/UI code keeps its upstream notices. Hardware access stays in fm1-base.
 */
#include "bridge.h"
#define FELUCCA_FLASH 1
#define FELUCCA_OTA 0
#define FELUCCA_CDC 0
#define bootguard h_bootguard
#define panel h_panel
#define proj_slot h_proj_slot
#define SMP_USER_XIP(k) h_platform_data(0xA0000u+(k)*0x10000u)
static void lcd_sync(void){h_platform_sync();}
static void lcd_init(void){}
static void lcd_fill(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint16_t c){h_platform_fill(x,y,w,h,c);}
static void lcd_blit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t *p){h_platform_blit(x,y,w,h,p);}
static void fm1_delay_ms(uint32_t ms){(void)ms;}
/* The same DSP, UI and sequencer are compiled for the device and WASM. */

#include <stdint.h>







#define memset felucca_memset
#define memcpy felucca_memcpy
#define memcmp felucca_memcmp
/* Production engine: no upstream host instrumentation. */
#include "gen/felucca_tables.h"
#include "../third_party/hortator/firmware/src/libc.c"



static struct { volatile uint32_t notes, buttons; } fm1_in;
static uint32_t h_irq_flags;
static void fm1_irq_off(void) {h_irq_flags=h_platform_lock();}                    /* (the main loop's guard: nothing to guard on the host) */
static uint32_t host_irq_on;                         /* fm1_irq_on calls (boot_test: none before main.c's) */
static void fm1_irq_on(void) { host_irq_on++;h_platform_unlock(h_irq_flags); }
#include "../third_party/hortator/firmware/src/core.h"
#include "../third_party/hortator/firmware/src/dsp.c"
#include "../third_party/hortator/firmware/src/comp.c"
#include "ported/eng_sample.c"
#include "../third_party/hortator/firmware/src/dmodels.c"
#include "ported/presets.c"
#include "../third_party/hortator/firmware/src/params.c"
#include "../third_party/hortator/firmware/src/reson.c"
#include "../third_party/hortator/firmware/src/filter.c"
#include "../third_party/hortator/firmware/src/lfo.c"
#include "../third_party/hortator/firmware/src/drum_core.c"
#include "../third_party/hortator/firmware/src/slicer.c"
#include "ported/fx.c"


#define MQ 64u
static uint32_t midi_in_q[MQ],midi_in_ms[MQ];
static uint8_t midi_in_source[MQ];
static volatile uint32_t mi_w,mi_r;
static volatile uint8_t midi_in_overflow;
static void midi_out_event(uint32_t p){h_platform_midi(p);}
static struct {unsigned config,suspended,up,setups,sof_seen;} usb;


#include "../third_party/hortator/firmware/src/grids.c"
#include "../third_party/hortator/firmware/src/seq.c"


/* ---- input HAL (hal/fm1_input.h API, host state) */
#define FM1_NCOL 11u
/* FM1_KEYMAP: copy the table byte for byte from hal/fm1_input.h (the LED positions depend on it) */
static const int8_t FM1_KEYMAP[6][FM1_NCOL] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},          /* PA0: encoders */
    { 5, 11,  4, 10,  3,  9,  2,  8, -1, -1, -1},          /* PA5 */
    {34, 35, 36, 37, 38, 40, 39, 13,  7,  6, 12},          /* PA6 */
    {23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33},          /* PA7 */
    { 0,  1, 15, 14, 17, 16, 19, 18, 20, 21, 22},          /* PA8 */
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},          /* PB7: encoder 6 */
};
static uint8_t fm1_led[FM1_NCOL];
static uint8_t fm1__tick_col;                         /* (hal/fm1_input.h: the column the scan lit last; led_dim.c) */
static uint32_t host_btn_edges, host_note_edges, host_ticks;
static int32_t host_enc[7];
#define FM1_TICKS_PER_US 24u
static uint32_t fm1_ticks(void) { return h_platform_ms()*24000u; }   /* 1 ms per call */
static int32_t host_enc_late[7];                      /* detents that come in after a pass's first read of the encoder
                                                       * (the scan ISR runs while ui_input does: the next read has them) */
static void enc_frame(void) {}                         /* (enc.c: the scan's decoder) */
static int32_t enc_take(uint32_t e)                   /* (enc.c) */
{
    int32_t s = host_enc[e % 7u];
    host_enc[e % 7u] = host_enc_late[e % 7u];
    host_enc_late[e % 7u] = 0;
    return s;
}
static uint32_t fm1_input_edges(uint32_t *released) { uint32_t p = host_btn_edges; (void)released; host_btn_edges = 0; return p; }
static uint32_t fm1_input_note_edges(void) { uint32_t p = host_note_edges; host_note_edges = 0; return p; }
static void fm1_led_key(uint32_t id, int on) { (void)id; (void)on; }
static void fm1_wdt_feed(void) {}
#define FM1_DEB_PRESS 2u               /* as hal/fm1_input.h (upstream 1.0) */
#define FM1_DEB_RELEASE 8u
static uint32_t host_scans;                          /* the boot's polled scans (tests: SEQ held = fm1_in.buttons) */
static void fm1_input_scan(void) { host_scans++; }
#define SCOPE_N 512u
static int16_t scope_buf[SCOPE_N];
static uint32_t scope_w;
static struct { uint32_t stage, page, home, ui_frames; } felucca_dbg;
#include "ported/gfx.c"
#include "../third_party/hortator/firmware/src/panel.c"
#include "../third_party/hortator/firmware/src/pages.c"
#define FELUCCA_VERSION "H 0.23.3 / Z 0.9.0"
#include "../third_party/hortator/firmware/src/ui.c"
#ifndef UI_NO_DRAW
#include "ported/ui_draw.c"
#else
static void ui_draw(void) {}
#endif
#include "ported/ui_menu.c"
#include "../third_party/hortator/firmware/src/ui_input.c"
#if FELUCCA_FLASH
                               /* tests/boot_test.c: the boot path with flash */
#endif
#ifdef UI_NO_PROJECT                                  /* until Task 4 rewrites project.c */
static void project_save(uint32_t s) { (void)s; }
static void project_load(uint32_t s) { (void)s; }
static int project_used(uint32_t s) { (void)s; return 0; }
static void settings_save(void) {}
#else
static uint8_t flash_ok=1;
#define FL_FAR(fn) (fn)
static uint32_t fl_jedec_ram(void){return 0x856014u;}
static void fl_plain_window_init(void){}
static uint32_t irq_save(void){return 0;}
static void irq_restore(uint32_t f){(void)f;}
static int st_read(uint32_t off,void *dst,uint32_t n){return h_platform_read(off,dst,n);}
static uint8_t h_store_error;
static int st_erase(uint32_t off){int rc=h_platform_erase(off);if(rc)h_store_error=1;return rc;}
static int st_prog(uint32_t off,const void *p,uint32_t n){int rc=h_platform_write(off,p,n);if(rc)h_store_error=1;return rc;}
#include "ported/storage.c"
#include "ported/project.c"
#endif


void h_init(void){
 fm1_ms=h_platform_ms();usb.config=usb.up=0;panel=PANEL_DEFAULT;
 h_unfocus();transport_req=0;host_enc[0]=host_enc[1]=host_enc[2]=host_enc[3]=host_enc[4]=host_enc[5]=host_enc[6]=0;
 memset(dly_buf,0,sizeof dly_buf);memset(cho_buf,0,sizeof cho_buf);memset(rev_comb,0,sizeof rev_comb);memset(&rev_u,0,sizeof rev_u);
 memset(rs_buf,0,sizeof rs_buf);memset(sl_buf,0,sizeof sl_buf);memset(sl,0,sizeof sl);memset(&fx,0,sizeof fx);memset(&mo,0,sizeof mo);memset(&chain,0,sizeof chain);memset(&song,0,sizeof song);
 memset(metal_ph,0,sizeof metal_ph);metal_blk=0xffffffffu;dvage=0;mi_w=mi_r=0;comp_reset();
 persist_boot();settings_init();panel_init();drum_tracks_init();
 if(lastp.set&&proj_ok(&proj_slot[lastp.slot&3]))project_load(lastp.slot);
 memset(&ui,0,sizeof ui);ui.home=1;ui.hmode=1;ui.force=1;
 song.master_q12=4096;safe_start=0;
}
void h_render(float *out,uint32_t n){
 int32_t b[CTL*2];
#ifdef Z_WASM
 h_wasm_frames+=n;
#endif
 fm1_ms=h_platform_ms();
 for(uint32_t at=0;at<n;at+=CTL){uint32_t count=n-at;if(count>CTL)count=CTL;mix_block(b,count);
  for(uint32_t i=0;i<count;i++){out[(at+i)*2]=b[i*2]/32768.0f;out[(at+i)*2+1]=b[i*2+1]/32768.0f;if(i&1)scope_buf[scope_w++&(SCOPE_N-1)]=(int16_t)b[i*2];}
 }
}
static uint32_t h_remote_buttons,h_remote_keys,h_physical_buttons,h_physical_keys;
void h_input(uint32_t buttons,uint32_t keys,const int32_t *enc){
 h_physical_buttons=buttons;h_physical_keys=keys;buttons|=h_remote_buttons;keys|=h_remote_keys;
 fm1_ms=h_platform_ms();uint32_t raw=0;
 for(uint32_t k=0;k<NB;k++)if(buttons>>k&1)raw|=1u<<panel.btn[k];
 host_btn_edges|=raw&~fm1_in.buttons;fm1_in.buttons=raw;
 host_note_edges|=keys&~fm1_in.notes;fm1_in.notes=keys;
 for(uint32_t k=0;k<NE;k++)host_enc[panel.enc[k]]+=enc[k]*panel.dir[k];
 ui_input();ui_leds();
 if(led_dim_on){uint32_t ms=h_platform_ms();led_br_lit=(ms%20u)<led_br_level(ms%1130u*1024u/1130u)*20u/65536u;for(uint32_t c=0;c<FM1_NCOL;c++)fm1_led[c]=(uint8_t)led_dim_compose(led_lay[c],ms%20u);}
}
void h_draw(void){fm1_ms=h_platform_ms();felucca_dbg.ui_frames++;ui_draw();}
void h_transport(uint32_t on){transport_req=on?1:2;}
void h_tempo(uint32_t bpm){song.g[G_BPM]=(int16_t)clamp(bpm,30,300);}
uint32_t h_bpm(void){return song.g[G_BPM];}
void h_midi(uint32_t p,uint32_t port){
 if(mi_w-mi_r>=MQ){midi_in_overflow=1;return;}
 uint32_t at=mi_w%MQ;midi_in_q[at]=p;midi_in_ms[at]=h_platform_ms();midi_in_source[at]=(uint8_t)(port?2:1);mi_w++;
}
static int h_lit(uint32_t id){for(uint32_t p=0;p<FM1_NCOL;p++)for(uint32_t row=1;row<5;row++)if(FM1_KEYMAP[row][p]==(int8_t)id)return(fm1_led[p]>>row)&1;return 0;}
uint32_t h_leds(void){uint32_t b=0;for(uint32_t k=0;k<NB;k++)if(h_lit(panel.btn[k]))b|=1u<<k;return b;}
uint32_t h_key_leds(void){uint32_t b=0;for(uint32_t k=0;k<27;k++)if(h_lit(14+k))b|=1u<<k;return b;}
void h_unfocus(void){h_remote_buttons=h_remote_keys=h_physical_buttons=h_physical_keys=0;fm1_in.notes=fm1_in.buttons=host_btn_edges=host_note_edges=0;mute_mask=0;perf_mask=0;perf_kill=1;kb_prev=0;ui.force=1;}
uint32_t h_models(void){return NMODELS;}
uint32_t h_presets(void){return PRE_NFACT;}

uint32_t h_requested_playing(void){return transport_req==1?1:transport_req==2?0:song.playing;}
void h_select(int32_t d){song.sel=(uint8_t)((song.sel+NTRK+d%(int32_t)NTRK)%NTRK);ui.force=1;}
int h_save(void){transport_req=0;seq_stop();h_store_error=0;project_save(song.g[G_SLOT]>=1&&song.g[G_SLOT]<=4?(uint32_t)song.g[G_SLOT]-1:lastp.set?lastp.slot:0);settings_save();return h_store_error?-1:0;}
uint32_t h_has_project(void){return lastp.set;}
void h_legacy_begin(uint32_t length){if(lastp.set)return;for(uint32_t k=0;k<NTRK;k++){for(uint32_t i=0;i<NSTEP;i++)memset(&trk[k].step[i],0,sizeof(step_t));trk[k].p[P_SLEN]=(int16_t)clamp(length,1,64);}}
void h_legacy_step(uint32_t at,uint32_t note,uint32_t velocity){if(lastp.set||at>=64||!velocity)return;for(uint32_t k=0;k<NTRK;k++)if((uint32_t)trk[k].p[P_NOTE]==note){trk[k].step[at].on=1;trk[k].step[at].acc=velocity>104;}}
void h_channel(uint32_t ch){song.g[G_DRCH]=(int16_t)clamp(ch,1,16);}

int h_remote(uint32_t kind,uint32_t index,int32_t value){
 int32_t e[7]={0};
 if(kind==0){if(index>=NB||value<0||value>1)return -1;if(index==B_SAVE&&value)return h_save();if(value)h_remote_buttons|=1u<<index;else h_remote_buttons&=~(1u<<index);}
 else if(kind==1){if(index>=27||value<0||value>1)return -1;if(value)h_remote_keys|=1u<<index;else h_remote_keys&=~(1u<<index);}
 else if(kind==2){if(index>=NE||value < -63||value>63)return -1;e[index]=value;}
 else if(kind==3){if(index>=8||value<0||value>127)return -1;h_hit(index,(uint32_t)value);return 0;}
 else return -1;
 h_input(h_physical_buttons,h_physical_keys,e);return 0;
}
int h_set(uint32_t lane,uint32_t param,int32_t value){
 if(lane>=8||param>=P_COUNT)return -1;track_t *t=&trk[lane];
 const param_desc_t *d=track_desc(t,param);value=clamp(value,d->min,d->max);
 if(param==P_MODEL){if(value==DM_ACID&&acid_tracks(t)>=ACID_MAXTRK)return -1;drum_set_model(t,(uint32_t)value);motion_rebase(lane);}
 else {if(param==P_RMODEL&&reson_refused(t,t->p[param],value))return -1;t->p[param]=(int16_t)value;if(param>=P_E0&&param<=P_E7)preset_touch(t);motion_capture(t,param);}
 ui.force=1;return 0;
}
int h_get(uint32_t lane,uint32_t param){return lane<8&&param<P_COUNT?trk[lane].p[param]:-9999;}
int h_set_global(uint32_t param,int32_t value){if(param>=G_COUNT)return -1;song.g[param]=(int16_t)clamp(value,GP[param].min,GP[param].max);ui.force=1;return 0;}
int h_set_step(uint32_t lane,uint32_t at,uint32_t on,uint32_t accent,uint32_t condition,uint32_t ratchet,int32_t note){
 if(lane>=8||at>=64||on>1||accent>1||condition>COND_MAX||ratchet>3||note<NT_MIN||note>NT_MAX)return -1;
 step_t *p=&trk[lane].step[at];p->on=on;p->acc=accent;p->cond=condition;p->rat=ratchet;p->nt=(int8_t)note;p->tie=p->slide=p->rsv=0;ui.force=1;return 0;
}
int h_load_preset(uint32_t lane,uint32_t id){if(lane>=8)return -1;int rc=preset_load(&trk[lane],id);ui.force=1;return rc;}
void h_hit(uint32_t lane,uint32_t velocity){if(lane<8&&velocity&&velocity<=127)input_hit(&trk[lane],velocity);}
uint32_t h_pack(uint8_t *dst){
 project_t *p=(project_t*)dst;memset(p,0,sizeof *p);p->magic=PROJ_MAGIC;p->size=sizeof *p;
 for(uint32_t i=0;i<G_COUNT;i++)p->g[i]=song.g[i];p->sel=song.sel;
 for(uint32_t k=0;k<NTRK;k++){for(uint32_t i=0;i<P_NSTORED;i++)p->t[k].p[i]=(int8_t)trk[k].p[i];memcpy(p->t[k].step,trk[k].step,sizeof trk[k].step);proj_xp_put(&p->t[k],trk[k].p);p->pre[k]=(uint16_t)preset_rec_enc(trk[k].pre);p->pre_ed[k]=trk[k].pre_ed;}
 p->motion=mo.s;p->song=chain.cfg;memcpy(p->name,chain.name,NAME_LEN);p->sum=proj_sum(p);return sizeof *p;
}
int h_unpack(const uint8_t *src,uint32_t n){
 if(n!=sizeof(project_t))return -1;const project_t *p=(const project_t*)src;if(!proj_ok(p)||p->sel>=8||!motion_valid(&p->motion)||!chain_valid(&p->song))return -1;
 for(uint32_t i=0;i<G_COUNT;i++)if(p->g[i]<GP[i].min||p->g[i]>GP[i].max)return -1;
 uint32_t acid=0,reson=0,chord=0,modal=0;
 for(uint32_t k=0;k<8;k++){uint32_t m=(uint32_t)p->t[k].p[P_MODEL];if(m>=NMODELS)return -1;
  int rm=p->t[k].p[P_RMODEL];if((m==DM_ACID&&++acid>ACID_MAXTRK)||(rm!=RS_OFF&&++reson>RS_MAXTRK)||(rm==RS_CHORD&&++chord>2)||(rm==RS_MODAL&&++modal>RS_MAXMODAL))return -1;
  for(uint32_t i=0;i<P_NSTORED;i++){const param_desc_t *d=i>=P_E0&&i<=P_E7?&DMODELS[m].edit[i-P_E0]:&TP[i];if(p->t[k].p[i]<d->min||p->t[k].p[i]>d->max)return -1;}
  for(uint32_t at=0;at<64;at++)if(p->t[k].step[at].cond>COND_MAX||p->t[k].step[at].nt<NT_MIN||p->t[k].step[at].nt>NT_MAX)return -1;
 }
 seq_stop();for(uint32_t i=0;i<G_COUNT;i++)song.g[i]=p->g[i];song.sel=p->sel;
 for(uint32_t k=0;k<8;k++){track_t *t=&trk[k];drum_set_model(t,(uint32_t)p->t[k].p[P_MODEL]);for(uint32_t i=0;i<P_NSTORED;i++)t->p[i]=p->t[k].p[i];for(uint32_t i=P_NSTORED;i<P_COUNT;i++)t->p[i]=(int16_t)clamp(proj_xp_get(&p->t[k],i),TP[i].min,TP[i].max);memcpy(t->step,p->t[k].step,sizeof t->step);t->pre=(uint16_t)preset_ref(preset_rec_dec(p->pre[k]),t->model);t->pre_ed=p->pre_ed[k]?1:0;}
 mo.s=p->motion;chain.cfg=p->song;memcpy(chain.name,p->name,NAME_LEN);chain.name[NAME_LEN]=0;motion_forget();ui.force=1;return 0;
}

void h_remote_release(void){h_remote_buttons=h_remote_keys=0;}

#ifdef PLAT_HOST
#include "host_checks.inc"
#endif

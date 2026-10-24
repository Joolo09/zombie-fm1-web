/* SPDX-License-Identifier: MIT
 * Original Zombie Core synthesizer. Same C source runs on AC79 and browser WASM.
 * Floating-point oscillators with polyBLEP, two-pole feedback filter, ADSR,
 * two-operator FM, phase distortion, percussion and linear PCM sampler.
 * Tables are generated from mathematical definitions by tools/build-core.py. */
#include "dsp.h"
#include "z_tables.h"
#if Z_SAMPLER_EDITION
#include "z_samples.h"
#endif
#include "z_presets.h"
#ifndef Z_POOL
#ifdef Z_WASM
#define Z_POOL
#else
#define Z_POOL __attribute__((section(".pool")))
#endif
#endif
const char *const z_models[9]={"ANALOG","FM-2","PHASE","WAVETABLE","NOISE","KICK","SNARE","HAT","SAMPLE"};
const z_descriptor z_desc[Z_PARAMS]={
 {"model",0,8,0},{"wave",0,3,1},{"mix",0,127,64},{"detune",0,127,18},
 {"cutoff",0,127,100},{"resonance",0,127,24},{"attack",0,127,2},{"decay",0,127,45},
 {"sustain",0,127,90},{"release",0,127,35},{"fm",0,127,35},{"ratio",1,16,2},
 {"drive",0,127,0},{"delay",0,127,0},{"reverb",0,127,15},{"level",0,127,95},
 {"pan",-64,63,0},{"lfoRate",0,127,30},{"lfoDepth",0,127,0},{"sample",0,13,0},
 {"root",0,127,60},{"mute",0,1,0},{"channel",1,16,1},{"length",1,64,16},
 {"gate",1,127,80},{"transpose",-24,24,0},{"probability",0,127,127}};
z_project z_song;
typedef struct { float phase,phase2,env,lp1,lp2,velocity,age,pos,inc; uint32_t stamp,gate;
                 uint8_t track,note,stage,active,released,arp,pad,drum;float drum_decay;z_pad settings;uint32_t sample_start,sample_end; } voice;
static voice voices[Z_VOICES];
static Z_POOL float delay_l[8192],delay_r[8192],comb_a[1499],comb_b[1877];
static Z_POOL uint8_t transfer[Z_SLOT_BYTES];
#ifdef Z_WASM
static Z_POOL int16_t preview_pcm[4][Z_PCM_MAX];
#endif
static Z_POOL float render_buffer[512];
static const int16_t *pcm[14];
static uint32_t pcm_n[14],pcm_rate[14];
#include "sampler.inc"
static uint32_t rng_state,stamp,delay_i,comb_i,playing,pos,external_clock,clock_ticks;
static float step_time,lfo_phase[Z_TRACKS];
static float bend[Z_TRACKS];
static uint8_t sustain[Z_TRACKS];
static float dc_xl,dc_xr,dc_yl,dc_yr;
static uint8_t arp_notes[6][128],arp_physical[6][16];
static float arp_time[6];
static uint32_t arp_index[6],revision;
static void (*midi_out)(uint32_t,uint32_t,uint32_t,uint32_t);
static uint32_t audio_frame,last_step_frame,live_effect,live_requested,click_left,clock_beat_frame,clock_pulses,clock_last_frame;
static float click_phase,live_phase,live_blend;
static uint8_t arp_count[6],rec_at[6][128],rec_active[6][128],applied_mode[6];
static uint32_t rec_start[6][128];
static uint32_t random32(void);
/* Undo stores musical patterns only. Transport, language and live settings stay put. */
typedef struct {z_step steps[6][64];uint8_t chord[6][64][3],duration[6][64];int16_t length[6];} z_pattern_snapshot;
static Z_POOL z_pattern_snapshot undo_bank[4],redo_bank[4];
static uint32_t undo_count,redo_count,solo_track;
static void pattern_capture(z_pattern_snapshot *p){uint32_t t,i,j;for(t=0;t<6;t++){
 p->length[t]=z_song.track[t].p[Z_LENGTH];for(i=0;i<64;i++){p->steps[t][i]=z_song.track[t].steps[i];p->duration[t][i]=z_song.duration[t][i];for(j=0;j<3;j++)p->chord[t][i][j]=z_song.chord[t][i][j];}
}}
static void pattern_restore(const z_pattern_snapshot *p){uint32_t t,i,j;for(t=0;t<6;t++){
 z_song.track[t].p[Z_LENGTH]=p->length[t];for(i=0;i<64;i++){z_song.track[t].steps[i]=p->steps[t][i];z_song.duration[t][i]=p->duration[t][i];for(j=0;j<3;j++)z_song.chord[t][i][j]=p->chord[t][i][j];}
}z_touch();}
static void history_push(z_pattern_snapshot *bank,uint32_t *count){uint32_t i;if(*count==4){for(i=0;i<3;i++)bank[i]=bank[i+1];*count=3;}pattern_capture(&bank[(*count)++]);}
void z_checkpoint(void){history_push(undo_bank,&undo_count);redo_count=0;}
uint32_t z_history_state(void){return undo_count|(redo_count<<8);}
void z_history_reset(void){undo_count=redo_count=0;}
uint32_t z_history(uint32_t redo){
 if(redo>1)return 0;
 if(redo){if(!redo_count)return 0;z_record(0);history_push(undo_bank,&undo_count);pattern_restore(&redo_bank[--redo_count]);}
 else {if(!undo_count)return 0;z_record(0);history_push(redo_bank,&redo_count);pattern_restore(&undo_bank[--undo_count]);}
 return 1;
}
uint32_t z_solo_state(void){return solo_track;}
void z_solo(uint32_t t){uint32_t i;if(t>6)return;solo_track=t;if(t&&midi_out)for(i=0;i<6;i++)if(i+1!=t)midi_out(i,128,0,0);}
int z_pattern(uint32_t t,uint32_t action,uint32_t target){
 uint32_t i,j,len;z_step step;uint8_t chord[3],duration;
 if(t>=6||action>6)return -1;len=z_song.track[t].p[Z_LENGTH];
 if(action==1&&len>32)return -1;if(action==4&&(target>=6||target==t))return -1;
 z_record(0);z_checkpoint();
 if(action==0){for(i=0;i<64;i++){z_song.track[t].steps[i].velocity=0;z_song.duration[t][i]=0;for(j=0;j<3;j++)z_song.chord[t][i][j]=0;}}
 else if(action==1||action==4){uint32_t dst=action==4?target:t,offset=action==4?0:len;
  for(i=0;i<(action==4?64:len);i++){z_song.track[dst].steps[offset+i]=z_song.track[t].steps[i];z_song.duration[dst][offset+i]=z_song.duration[t][i];for(j=0;j<3;j++)z_song.chord[dst][offset+i][j]=z_song.chord[t][i][j];}
  z_song.track[dst].p[Z_LENGTH]=(int16_t)(action==4?len:len*2);
 }else if(action==6){int model=z_song.track[t].p[Z_MODEL];for(i=0;i<len;i++){static const uint8_t notes[4]={48,48,55,58};z_step *s=&z_song.track[t].steps[i];s->note=model>=5&&model<=7?60:notes[random32()&3];s->velocity=model==5?(i%4==0?110:0):model==6?(i%8==4?110:0):model==7?(i%2==0?85:0):((random32()&3)<2?100:0);s->chance=127;s->gate=70;z_song.duration[t][i]=0;for(j=0;j<3;j++)z_song.chord[t][i][j]=0;}}
 else if(action==5){for(i=0;i<len/2;i++){uint32_t b=len-1-i;step=z_song.track[t].steps[i];duration=z_song.duration[t][i];for(j=0;j<3;j++)chord[j]=z_song.chord[t][i][j];z_song.track[t].steps[i]=z_song.track[t].steps[b];z_song.duration[t][i]=z_song.duration[t][b];z_song.track[t].steps[b]=step;z_song.duration[t][b]=duration;for(j=0;j<3;j++){z_song.chord[t][i][j]=z_song.chord[t][b][j];z_song.chord[t][b][j]=chord[j];}}}
 else {
  uint32_t first=action==2?0:len-1;step=z_song.track[t].steps[first];duration=z_song.duration[t][first];for(j=0;j<3;j++)chord[j]=z_song.chord[t][first][j];
  for(i=0;i<len-1;i++){uint32_t dst=action==2?i:len-1-i,src=action==2?i+1:len-2-i;z_song.track[t].steps[dst]=z_song.track[t].steps[src];z_song.duration[t][dst]=z_song.duration[t][src];for(j=0;j<3;j++)z_song.chord[t][dst][j]=z_song.chord[t][src][j];}
  first=action==2?len-1:0;z_song.track[t].steps[first]=step;z_song.duration[t][first]=duration;for(j=0;j<3;j++)z_song.chord[t][first][j]=chord[j];
 }
 if(action==6&&z_drum_track(t))for(i=0;i<len;i++){z_step *s=&z_song.track[t].steps[i];s->note=i%4==0?36:i%8==7?46:42;s->velocity=i%4==0?112:i%2==0?80:0;if(i%4==0)z_song.chord[t][i][0]=43;if(i%8==4)z_song.chord[t][i][1]=39;}
 z_touch();return 0;
}
static float step_frames(void){return 661500.0f/z_song.bpm;}
static void record_finish(uint32_t t,uint32_t n){
 if(rec_active[t][n]){uint32_t at=rec_at[t][n];float len=(audio_frame-rec_start[t][n])/step_frames();
  uint32_t whole,gate;if(len<0.04f)len=0.04f;if(len>63.99f)len=63.99f;
  whole=(uint32_t)len;gate=(uint32_t)((len-whole)*127);if(gate<1)gate=1;
  if(whole>z_song.duration[t][at]||(whole==z_song.duration[t][at]&&gate>z_song.track[t].steps[at].gate)){
   z_song.duration[t][at]=(uint8_t)whole;z_song.track[t].steps[at].gate=(uint8_t)gate;
  }
  rec_active[t][n]=0;z_touch();
 }
}
void z_record(uint32_t on){uint32_t t,n;on=!!on;if(on&&!z_song.recording)z_checkpoint();
 if(!on)for(t=0;t<6;t++)for(n=0;n<128;n++)record_finish(t,n);
 if(z_song.recording!=on){z_song.recording=(uint8_t)on;z_touch();}
}
void z_live(uint32_t effect){live_requested=effect<=16?effect:0;if(live_requested)live_effect=live_requested;}
uint32_t z_live_state(void){return live_requested;}
void z_clear_track(uint32_t t){(void)z_pattern(t,0,0);}
/* Reconcile control changes without copying transient key/transport state. */
void z_apply_state(void){uint32_t t,n;
 for(t=0;t<6;t++){if(Z_SAMPLER_EDITION&&z_song.track[t].p[Z_MODEL]!=8){z_song.track[t].p[Z_MODEL]=8;z_song.track[t].p[Z_ATTACK]=0;z_song.track[t].p[Z_SUSTAIN]=127;z_song.track[t].p[Z_RELEASE]=8;z_song.preset[t]=255;}else if(!Z_SAMPLER_EDITION&&z_song.track[t].p[Z_MODEL]>7){z_song.track[t].p[Z_MODEL]=0;z_song.preset[t]=255;}}
 z_sampler.tracks=Z_SAMPLER_EDITION?63:0;
 if(!Z_SAMPLER_EDITION){z_song.track[5].p[Z_MODEL]=7;z_song.preset[5]=255;}
 for(t=0;t<6;t++){if(applied_mode[t]!=z_song.arp_mode[t]){z_alloff(t);applied_mode[t]=z_song.arp_mode[t];}arp_count[t]=0;for(n=0;n<128;n++){
   if(!z_song.arp_hold[t]&&!(arp_physical[t][n/8]>>(n&7)&1))arp_notes[t][n]=0;
   if(arp_notes[t][n])arp_count[t]++;
  }
  if(!z_song.arp_mode[t]||!arp_count[t]){arp_time[t]=0;arp_index[t]=0;}
  if(z_song.arp_mode[t]&&!arp_count[t]){for(n=0;n<8;n++)if(voices[n].active&&voices[n].track==t&&voices[n].arp){voices[n].stage=4;voices[n].gate=0;}if(midi_out)midi_out(t,128,1,0);}
 }
}
static uint32_t random32(void) { uint32_t x=rng_state; x^=x<<13; x^=x>>17; x^=x<<5; return rng_state=x; }
static float absf(float x) { return x<0?-x:x; }
static float clampf(float x,float a,float b) { return x<a?a:x>b?b:x; }
static float sat(float x) { return x/(1.0f+absf(x)); }
static float sine(float p) { int i; float f; p-=(int)p; if(p<0)p+=1; p*=512; i=(int)p; f=p-i; return z_sine[i]+(z_sine[(i+1)&511]-z_sine[i])*f; }
static float blep(float p,float dt) { if(p<dt) { p/=dt; return p+p-p*p-1; } if(p>1-dt) { p=(p-1)/dt; return p*p+p+p+1; } return 0; }
static float osc(float p,float dt,int shape) {
 if(shape==0)return sine(p);
 if(shape==1)return p*2-1-blep(p,dt);
 if(shape==2){float q=p+0.5f; if(q>=1)q-=1; return (p<0.5f?1:-1)+blep(p,dt)-blep(q,dt);}
 return 1-4*absf(p-0.5f);
}
#include "drums.inc"
void z_set_midi_out(void (*cb)(uint32_t,uint32_t,uint32_t,uint32_t)) { midi_out=cb; }
void z_touch(void) { if(++revision==0)revision=1; }
uint32_t z_revision(void) { return revision; }
uint32_t z_state_size(void) { return sizeof z_song; }
const char *z_preset_name(uint32_t p) { return p<Z_PRESETS?Z_PRESET_NAMES[p]:"EDITED"; }
int z_set(uint32_t t,uint32_t p,int v) {
 if(t>=Z_TRACKS||p>=Z_PARAMS)return -9999;
 if(p==Z_MODEL)v=Z_SAMPLER_EDITION?8:z_drum_track(t)?7:(v>7?7:v);
 if(v<z_desc[p].min)v=z_desc[p].min;if(v>z_desc[p].max)v=z_desc[p].max;
 if(z_song.track[t].p[p]!=v){if(p==Z_CHANNEL)z_alloff(t);z_song.track[t].p[p]=(int16_t)v;
  if(p==Z_MUTE&&v&&midi_out)midi_out(t,128,0,0);
  if(p!=Z_LEVEL&&p!=Z_PAN&&p!=Z_MUTE&&p!=Z_CHANNEL&&p!=Z_LENGTH&&p!=Z_GATE&&p!=Z_PROBABILITY)z_song.preset[t]=255;
  z_touch();}return v;
}
int z_get(uint32_t t,uint32_t p) { return t<Z_TRACKS&&p<Z_PARAMS?z_song.track[t].p[p]:-9999; }
void z_reset_audio(void) {
 dc_xl=dc_xr=dc_yl=dc_yr=0;live_effect=live_requested=0;live_blend=0;click_left=0;
 uint32_t i,t; for(i=0;i<Z_VOICES;i++)voices[i].active=0;
 for(t=0;t<6;t++){arp_count[t]=0;sustain[t]=0;applied_mode[t]=z_song.arp_mode[t];if(midi_out)midi_out(t,128,0,0);for(i=0;i<128;i++){arp_notes[t][i]=0;rec_active[t][i]=0;}for(i=0;i<16;i++)arp_physical[t][i]=0;arp_time[t]=0;arp_index[t]=0;}
 for(i=0;i<8192;i++)delay_l[i]=delay_r[i]=0;
 for(i=0;i<1499;i++)comb_a[i]=0; for(i=0;i<1877;i++)comb_b[i]=0;
}
/* One catalog generates the native bank and the browser names. */
void z_preset(uint32_t t,uint32_t preset) {
 uint32_t i;int16_t *p;int level,pan,channel,mute,len,gate,prob;
 if(t>=6||preset>=Z_PRESETS)return;p=z_song.track[t].p;
 if(z_drum_track(t)){p[Z_MODEL]=7;p[Z_CUTOFF]=121;p[Z_RESONANCE]=8;p[Z_DRIVE]=16;p[Z_DELAY]=0;p[Z_REVERB]=6;z_song.preset[t]=255;z_touch();return;}
 level=p[Z_LEVEL];pan=p[Z_PAN];channel=p[Z_CHANNEL];mute=p[Z_MUTE];len=p[Z_LENGTH];gate=p[Z_GATE];prob=p[Z_PROBABILITY];
 for(i=0;i<Z_PARAMS;i++)p[i]=Z_PRESET_VALUES[preset][i];
 if(Z_SAMPLER_EDITION){p[Z_MODEL]=8;p[Z_ATTACK]=0;p[Z_SUSTAIN]=127;p[Z_RELEASE]=8;}
 p[Z_LEVEL]=level;p[Z_PAN]=pan;p[Z_CHANNEL]=channel;p[Z_MUTE]=mute;p[Z_LENGTH]=len;p[Z_GATE]=gate;p[Z_PROBABILITY]=prob;
 z_song.preset[t]=(uint8_t)preset;z_touch();
}
void z_sample(uint32_t s,const int16_t *data,uint32_t n,uint32_t rate) { if(s<14&&rate>=4000&&rate<=44100) {pcm[s]=data;pcm_n[s]=n;pcm_rate[s]=rate;} }
#include "edition_dsp.inc"
void z_init(void) {
 uint32_t t,i;undo_count=redo_count=solo_track=0;for(i=0;i<sizeof z_song;i++)((uint8_t *)&z_song)[i]=0;revision=1; rng_state=0x51f15e37;stamp=0;playing=0;pos=63;step_time=0;external_clock=0;clock_ticks=0;
 z_song.magic=0x31434f5a;z_song.version=3;z_song.fx_amount=90;audio_frame=last_step_frame=0;live_phase=0;z_song.bpm=120;z_song.swing=0;z_song.selected=0;z_song.language=0;z_song.reserved=0;
 for(t=0;t<Z_TRACKS;t++){
  for(i=0;i<Z_PARAMS;i++)z_song.track[t].p[i]=z_desc[i].def;
  z_song.track[t].p[Z_CHANNEL]=(int16_t)(t+1);
  for(i=0;i<64;i++){z_step *s=&z_song.track[t].steps[i];s->note=60;s->velocity=0;s->chance=127;s->gate=80;}
  z_song.arp_oct[t]=1;z_song.arp_rate[t]=1;lfo_phase[t]=0;bend[t]=1;sustain[t]=0;
  {static const uint8_t defaults[6]={48,51,54,16,24,32};z_preset(t,Z_SAMPLER_EDITION?t:defaults[t]);}
 }
 for(i=0;i<14;i++){pcm[i]=0;pcm_n[i]=0;pcm_rate[i]=22050;}
#if Z_SAMPLER_EDITION
 for(i=0;i<6;i++)z_sample(i,z_factory_pcm[i],z_factory_n[i],11025);
 for(i=0;i<4;i++)z_sample(10+i,z_drum_pcm[i],z_drum_n[i],11025);
#endif
 sampler_init();z_sampler.tracks=Z_SAMPLER_EDITION?63:0;z_reset_audio();edition_defaults();
}
void z_note(uint32_t t,uint32_t note,uint32_t vel,uint32_t on) {
 uint32_t i,best=0,oldest=0xffffffffu;voice *v;
 if(t>=Z_TRACKS||note>127)return;
 if(!on||!vel){if(z_drum_track(t))return;for(i=0;i<Z_VOICES;i++)if(voices[i].active&&voices[i].track==t&&voices[i].note==note){voices[i].released=1;if(!sustain[t]&&(voices[i].pad==255||voices[i].settings.mode))voices[i].stage=3;}return;}
 for(i=0;i<Z_VOICES;i++) { if(!voices[i].active){best=i;break;} if(voices[i].stamp<oldest){oldest=voices[i].stamp;best=i;} }
 v=&voices[best];
 if(z_drum_track(t)){uint32_t d=z_drum_index(note);if(d==2||d==3)for(i=0;i<Z_VOICES;i++)if(voices[i].active&&voices[i].track==t&&(voices[i].drum==2||voices[i].drum==3))voices[i].stage=4;
  v->drum=(uint8_t)d;v->drum_decay=1.0f-6.90776f/(drum_ms[d]*44.1f);
 }else v->drum=255;
 if(z_song.track[t].p[Z_MODEL]==8&&(z_sampler.tracks>>t&1)){
  uint32_t start,end,slot;z_pad pad;if(note<36||note>=68)return;pad=z_sampler.pad[note-36];slot=pad.source;
  if(!pcm[slot]||pcm_n[slot]<2)return;
  start=pcm_n[slot]*(uint32_t)pad.start/65535u;end=(pcm_n[slot]*(uint32_t)pad.end+65534u)/65535u;
  if(end>pcm_n[slot])end=pcm_n[slot];if(end-start<2)return;
  if(pad.choke)for(i=0;i<8;i++)if(voices[i].active&&voices[i].pad!=255&&voices[i].settings.choke==pad.choke){voices[i].stage=4;voices[i].gate=0;}
  v->pad=(uint8_t)(note-36);v->settings=pad;v->sample_start=start;v->sample_end=end;
 }else v->pad=255;
 v->track=(uint8_t)t;v->note=(uint8_t)note;v->stage=0;v->active=1;v->released=0;v->arp=0;v->stamp=++stamp;
 v->phase=0;v->phase2=0;v->env=0;v->lp1=v->lp2=v->inc=0;v->age=v->pos=0;v->velocity=(vel>127?127:vel)/127.0f;v->gate=0;
}
/* Panel, browser and external MIDI all enter through this same key path. */
void z_key(uint32_t t,uint32_t n,uint32_t v,uint32_t on) {
 uint32_t i,physical=0,was;if(t>=6||n>127)return;on=on&&v;was=arp_physical[t][n/8]>>(n&7)&1;
 for(i=0;i<16;i++)physical|=arp_physical[t][i];
 if(on){
  if(was)return; /* duplicate MIDI note-on must not advance or multiply a take */
  if(!physical&&z_song.arp_hold[t]){for(i=0;i<128;i++)arp_notes[t][i]=0;arp_count[t]=0;}
  arp_physical[t][n/8]|=1u<<(n&7);if(!arp_notes[t][n])arp_count[t]++;arp_notes[t][n]=(uint8_t)(v>127?127:v);
  if(!physical){arp_time[t]=0;arp_index[t]=0;}
  if(z_song.recording){
   uint32_t len=z_song.track[t].p[Z_LENGTH],at=z_song.step_edit;
   if(playing){at=pos==63&&step_time==0?0:pos%len;if(!z_song.rec_quant&&(audio_frame-last_step_frame)>step_frames()*0.5f)at=(at+1)%len;}
   z_step *s=&z_song.track[t].steps[at];
   if(!s->velocity||(z_song.rec_mode&&!physical)){
    s->note=(uint8_t)n;s->velocity=(uint8_t)v;s->chance=127;s->gate=1;z_song.duration[t][at]=0;
    for(i=0;i<3;i++)z_song.chord[t][at][i]=0;
   }else if(s->note!=n){for(i=0;i<3;i++)if(z_song.chord[t][at][i]==n+1)break;else if(!z_song.chord[t][at][i]){z_song.chord[t][at][i]=(uint8_t)(n+1);break;}}
   rec_active[t][n]=1;rec_at[t][n]=(uint8_t)at;rec_start[t][n]=audio_frame;
   z_song.step_edit=(uint8_t)at;z_song.step_bank=(uint8_t)(at/16);z_touch();
  }
 }else{
  if(!was)return;
  record_finish(t,n);arp_physical[t][n/8]&=(uint8_t)~(1u<<(n&7));
  if(!z_song.arp_hold[t]||!z_song.arp_mode[t]){if(arp_notes[t][n])arp_count[t]--;arp_notes[t][n]=0;}
  physical=0;for(i=0;i<16;i++)physical|=arp_physical[t][i];
  if(z_song.recording&&!playing&&!physical){z_song.step_edit=(uint8_t)((z_song.step_edit+1)%z_song.track[t].p[Z_LENGTH]);z_song.step_bank=z_song.step_edit/16;z_touch();}
  if(z_song.arp_mode[t]&&!arp_count[t]){
   arp_time[t]=0;arp_index[t]=0;
   /* Terminate generated gates now. Keep a short click-free release, not seconds of sustained arp. */
   for(i=0;i<8;i++)if(voices[i].active&&voices[i].track==t&&voices[i].arp){voices[i].released=1;voices[i].stage=4;voices[i].gate=0;}
   if(midi_out)midi_out(t,128,1,0); /* cancel pending MIDI gates for this track */
  }
 }
 if(!z_song.arp_mode[t])z_note(t,n,v,on);
}
static void arp_tick(void) {
 uint32_t t;
 for(t=0;t<6;t++)if(z_song.arp_mode[t]){
  uint32_t i,count=0,total,index,nth,n=0,v=0;float frames;
  count=arp_count[t];if(!count)continue;
  if(arp_time[t]>0){arp_time[t]-=1;continue;}total=count*z_song.arp_oct[t];index=arp_index[t]++;
  if(z_song.arp_mode[t]==2)index=total-1-index%total;
  else if(z_song.arp_mode[t]==3){uint32_t period=total>1?total*2-2:1;index%=period;if(index>=total)index=period-index;}
  else if(z_song.arp_mode[t]==4)index=random32()%total;
  else index%=total;
  nth=index%count;
  for(i=0;i<128;i++)if(arp_notes[t][i]){if(!nth){n=i+12*(index/count);v=arp_notes[t][i];break;}nth--;}
  frames=44100.0f*15/z_song.bpm;
  if(z_song.arp_rate[t]==0)frames*=2;else if(z_song.arp_rate[t]==2)frames*=0.5f;else if(z_song.arp_rate[t]==3)frames*=1.3333333f;
  arp_time[t]+=frames;
  if(n<=127&&!z_song.track[t].p[Z_MUTE]&&(!solo_track||solo_track==t+1)){z_note(t,n,v,1);for(i=0;i<8;i++)if(voices[i].active&&voices[i].track==t&&voices[i].stamp==stamp){voices[i].arp=1;voices[i].gate=(uint32_t)(frames*z_song.track[t].p[Z_GATE]/127.0f)+1;}
   if(midi_out)midi_out(t,n,v,0x80000000u|((uint32_t)(frames/44.1f*z_song.track[t].p[Z_GATE]/127.0f)+1));
  }
 }
}

void z_bend(uint32_t t,uint32_t value){float pos,f;uint32_t i;if(t>=6)return;if(value>16383)value=16383;pos=69.0f+((int)value-8192)/4096.0f;i=(uint32_t)pos;f=pos-i;bend[t]=(z_frequency[i]*(1-f)+z_frequency[i+1]*f)/440.0f;}
void z_sustain(uint32_t t,uint32_t on){uint32_t i;if(t>=6)return;sustain[t]=!!on;if(!on)for(i=0;i<8;i++)if(voices[i].active&&voices[i].track==t&&voices[i].released&&voices[i].stage!=4&&(voices[i].pad==255||voices[i].settings.mode))voices[i].stage=3;}
void z_alloff(uint32_t t){uint32_t i;if(t>=6)return;for(i=0;i<128;i++)record_finish(t,i);arp_count[t]=0;if(midi_out)midi_out(t,128,0,0);sustain[t]=0;for(i=0;i<8;i++)if(voices[i].track==t)voices[i].active=0;for(i=0;i<128;i++)arp_notes[t][i]=0;for(i=0;i<16;i++)arp_physical[t][i]=0;arp_index[t]=0;arp_time[t]=0;}
int z_arp_set(uint32_t t,uint32_t field,uint32_t value){uint32_t i;if(t>=6||field>3)return -1;
 if(field==0){if(value>4)return -1;if(z_song.arp_mode[t]!=value)z_alloff(t);z_song.arp_mode[t]=(uint8_t)value;applied_mode[t]=(uint8_t)value;}
 else if(field==1){if(value>3)return -1;z_song.arp_rate[t]=(uint8_t)value;}
 else if(field==2){if(value<1||value>3)return -1;z_song.arp_oct[t]=(uint8_t)value;}
 else {if(value>1)return -1;z_song.arp_hold[t]=(uint8_t)value;if(!value)for(i=0;i<128;i++)if(!(arp_physical[t][i/8]>>(i&7)&1))arp_notes[t][i]=0;}
 z_apply_state();if(!arp_count[t]){for(i=0;i<8;i++)if(voices[i].active&&voices[i].track==t&&voices[i].arp){voices[i].stage=4;voices[i].gate=0;}if(midi_out)midi_out(t,128,1,0);}
 z_touch();return 0;
}
static void seq_step(void) {
 uint32_t t;pos=(pos+1)&63u;last_step_frame=audio_frame;if(z_song.metronome&&!(pos&3)){click_left=1100;click_phase=0;}
 for(t=0;t<Z_TRACKS;t++) {
  z_track *tr=&z_song.track[t];z_step *s=&tr->steps[pos%tr->p[Z_LENGTH]];
  if((!solo_track||solo_track==t+1)&&!tr->p[Z_MUTE]&&s->velocity&&(s->chance==127||(random32()&127)<(uint32_t)s->chance)&&(tr->p[Z_PROBABILITY]==127||(random32()&127)<(uint32_t)tr->p[Z_PROBABILITY])){
   uint32_t i,j,at=pos%tr->p[Z_LENGTH];
   float duration=step_frames()*(z_song.duration[t][at]+s->gate/127.0f);
   for(j=0;j<4;j++){
    uint32_t note=j?z_song.chord[t][at][j-1]:s->note+1,held=0;if(!note)continue;note--;
    /* Do not double the note under the player's finger while overdubbing. */
    for(i=0;i<128;i++)if(rec_active[t][i]&&rec_at[t][i]==at){held=1;break;}if(held)continue;
    z_note(t,note,s->velocity,1);
    for(i=0;i<Z_VOICES;i++)if(voices[i].active&&voices[i].track==t&&voices[i].stamp==stamp)voices[i].gate=(uint32_t)duration+1;
    if(midi_out)midi_out(t,note,s->velocity,(uint32_t)(duration/44.1f)+1);
   }
  }
 }
}
void z_transport(uint32_t on) { if(!on){z_record(0);z_reset_audio();solo_track=0;}playing=!!on;pos=63;step_time=0;clock_ticks=0;last_step_frame=audio_frame; }
uint32_t z_external(uint32_t on) {external_clock=!!on;if(z_song.reserved!=(uint8_t)external_clock){z_song.reserved=(uint8_t)external_clock;z_touch();}return external_clock;}
void z_clock(uint32_t status) {
 if(!external_clock)return;
 if(status==0xfa){clock_pulses=0;clock_beat_frame=audio_frame;clock_last_frame=audio_frame;z_transport(1);}
 else if(status==0xfb){playing=1;clock_last_frame=audio_frame;}
 else if(status==0xfc){uint32_t at=pos,ticks=clock_ticks;z_transport(0);pos=at;clock_ticks=ticks;}
 else if(status==0xf8){
  clock_last_frame=audio_frame;
  if(!clock_pulses)clock_beat_frame=audio_frame;
  if(++clock_pulses==25){uint32_t frames=audio_frame-clock_beat_frame;
   if(frames>=8820&&frames<=88200){uint32_t bpm=(2646000u+frames/2)/frames;if(bpm!=z_song.bpm)z_tempo(bpm,z_song.swing);}
   clock_pulses=1;clock_beat_frame=audio_frame;
  }
  if(playing&&clock_ticks++%6==0)seq_step();
 }
}
void z_songpos(uint32_t sixteenths){if(!external_clock)return;pos=(sixteenths+63)&63;clock_ticks=0;last_step_frame=audio_frame;}
void z_tempo(uint32_t bpm,uint32_t swing) {z_song.bpm=(uint16_t)(bpm<30?30:bpm>300?300:bpm);z_song.swing=(uint8_t)(swing>60?60:swing);z_touch();}
void z_step_set(uint32_t t,uint32_t i,uint32_t n,uint32_t v,uint32_t c,uint32_t g) {
 z_step *s;if(t>=Z_TRACKS||i>=64)return;z_checkpoint();s=&z_song.track[t].steps[i];s->note=(uint8_t)(n>127?127:n);s->velocity=(uint8_t)(v>127?127:v);s->chance=(uint8_t)(c>127?127:c);s->gate=(uint8_t)(g<1?1:g>127?127:g);z_song.duration[t][i]=0;{uint32_t j;for(j=0;j<3;j++)z_song.chord[t][i][j]=0;}z_touch();
}
void z_render(float *out,uint32_t n) {
 uint32_t frame,k,t;float attack[6],decay[6],release[6],cut[6],lfo[6];
 for(t=0;t<6;t++){int16_t *p=z_song.track[t].p;
  attack[t]=1.0f/(1+p[Z_ATTACK]*p[Z_ATTACK]*9.0f);decay[t]=1.0f/(20+p[Z_DECAY]*p[Z_DECAY]*5.0f);release[t]=1.0f/(20+p[Z_RELEASE]*p[Z_RELEASE]*6.0f);
  /* Hats use a single audible decay instead of a sub-ms ADSR multiplied by another fade. */
  if(p[Z_MODEL]==7){decay[t]=1.0f/(441+p[Z_DECAY]*90.0f);release[t]=1.0f/(220+p[Z_RELEASE]*60.0f);}
  lfo_phase[t]+=(0.05f+p[Z_LFO_RATE]*0.12f)*n/44100.0f; if(lfo_phase[t]>=1)lfo_phase[t]-=(int)lfo_phase[t];
  lfo[t]=sine(lfo_phase[t])*p[Z_LFO_DEPTH]/127.0f;
  cut[t]=clampf(z_filter[p[Z_CUTOFF]]*(1+lfo[t]*0.8f),0.001f,0.78f);
 }
 for(frame=0;frame<n;frame++) {
  audio_frame++;
  if(external_clock&&playing&&audio_frame-clock_last_frame>110250)z_transport(0);
  arp_tick();
  float l=0,r=0,sendd=0,sendr=0;
  if(playing&&!external_clock){if(step_time<=0){seq_step();step_time+=44100.0f*15/z_song.bpm*(1+((pos&1)?-1:1)*z_song.swing/200.0f);}step_time-=1;}
  for(k=0;k<8;k++){
   voice *v=&voices[k];int16_t *p;float x=0,dt,dt2,e,a,pan;int model,note,ss;
   if(!v->active)continue;t=v->track;p=z_song.track[t].p;model=Z_SAMPLER_EDITION?8:p[Z_MODEL];
   /* A held one-shot must still end if its track changes to a synth preset. */
   if(v->pad!=255&&(model!=8||!(z_sampler.tracks>>t&1))){v->stage=4;model=8;}
   if(v->drum==255&&v->gate&&--v->gate==0&&(v->pad==255||v->settings.mode))v->stage=3;
   if(v->drum!=255){if(v->stage==0){v->env=1;v->stage=1;}else v->env*=v->stage==4?0.97f:v->drum_decay;if(v->env<0.00008f){v->active=0;continue;}}
   else if(v->stage==0){v->env+=attack[t];if(v->env>=1){v->env=1;v->stage=1;}}
   else if(v->stage==1){v->env+=(p[Z_SUSTAIN]/127.0f-v->env)*decay[t];if(absf(v->env-p[Z_SUSTAIN]/127.0f)<0.0001f)v->stage=2;}
   else if(v->stage==4){v->env*=0.985f;if(v->env<0.00008f){v->active=0;continue;}}
   else if(v->stage==3){v->env-=v->env*release[t];if(v->env<0.00008f){v->active=0;continue;}}
   if(v->drum==255&&model==7&&v->stage==2&&!p[Z_SUSTAIN]){v->active=0;continue;}
   note=(int)v->note+p[Z_TRANSPOSE];if(note<0)note=0;if(note>127)note=127;
   dt=z_frequency[note]/44100.0f*bend[t];dt2=dt*(1+p[Z_DETUNE]*0.00015f);e=v->env;
   v->age+=1.0f/44100;if(v->drum==255){v->phase+=dt;v->phase2+=dt2;if(v->phase>=1)v->phase-=1;if(v->phase2>=1)v->phase2-=1;}
   if(v->drum!=255)x=drum_signal(v);
   else if(model==0)x=osc(v->phase,dt,p[Z_WAVE])*(1-p[Z_MIX]/254.0f)+osc(v->phase2,dt2,p[Z_WAVE])*p[Z_MIX]/254.0f;
   else if(model==1)x=sine(v->phase+sine(v->phase*p[Z_RATIO])*p[Z_FM]/45.0f*e);
   else if(model==2){float pivot=0.08f+p[Z_FM]/151.0f;float ph=v->phase<pivot?v->phase*0.5f/pivot:0.5f+(v->phase-pivot)*0.5f/(1-pivot);x=sine(ph);}
   else if(model==3){float m=clampf(p[Z_MIX]/127.0f+lfo[t]*0.3f,0,1);x=sine(v->phase)*(1-m)+0.5f*(sine(v->phase*3)+sine(v->phase*5))*m;}
   else if(model==4)x=((int32_t)(random32()>>9)-4194304)/4194304.0f;
   else if(model==5){float bend=1+p[Z_FM]/4.0f/(1+v->age*90);v->pos+=dt*0.21f*bend;x=sine(v->pos)/(1+v->age*v->age*(15000.0f/(p[Z_DECAY]+3)));if(v->age>2)v->stage=3;}
   else if(model==6){x=(((int32_t)(random32()>>9)-4194304)/4194304.0f*0.7f+sine(v->phase)*0.3f)/(1+v->age*v->age*10000/(p[Z_DECAY]+1));if(v->age>1)v->stage=3;}
   else if(model==7){float noise=((int32_t)(random32()>>9)-4194304)/4194304.0f;
    /* Remove the low thud; a small inharmonic ring gives the noise a metallic edge. */
    v->inc+=(noise-v->inc)*0.35f;x=(noise-v->inc)*1.2f+sine(v->phase*17)*sine(v->phase2*23)*0.12f;
   }
   else if(v->pad!=255){
    const z_pad *pad=&v->settings;uint32_t span=v->sample_end-v->sample_start,j,next;float f,step;
    ss=pad->source;if(!pcm[ss]||v->sample_end>pcm_n[ss]){v->active=0;continue;}
    if(v->pos>=span){if(pad->mode==2)v->pos-=span*(uint32_t)(v->pos/span);else{v->active=0;continue;}}
    j=(uint32_t)v->pos;f=v->pos-j;next=j+1<span?j+1:pad->mode==2?0:j;
    if(pad->reverse){j=v->sample_end-1-j;next=v->sample_end-1-next;}else{j+=v->sample_start;next+=v->sample_start;}
    x=(pcm[ss][j]*(1-f)+pcm[ss][next]*f)/32768.0f;
    if(pad->quality){int scale=pad->quality==1?2048:128;x=(int)(x*scale)/(float)scale;}
    step=pcm_rate[ss]/44100.0f*z_frequency[60+pad->tune]/z_frequency[60]*bend[t];v->pos+=step;
    /* A short edge ramp avoids clicks at trim boundaries without editing source PCM. */
    {float edge=v->pos<32?v->pos/32:1;if(pad->mode!=2&&span-v->pos<32)edge=clampf((span-v->pos)/32,0,edge);x*=edge;}
    x*=pad->gain/100.0f;
   }
   else {ss=p[Z_SAMPLE];if(!pcm[ss]||v->pos>=pcm_n[ss]-1){v->active=0;continue;}uint32_t j=(uint32_t)v->pos;float f=v->pos-j;x=(pcm[ss][j]*(1-f)+pcm[ss][j+1]*f)/32768.0f;v->pos+=pcm_rate[ss]/44100.0f*z_frequency[note]/z_frequency[p[Z_ROOT]]*bend[t];}
   x=clampf(x-v->lp2*p[Z_RESONANCE]/100.0f,-3,3);v->lp1+=(x-v->lp1)*cut[t];v->lp2+=(v->lp1-v->lp2)*cut[t];x=v->lp2;
   a=1+p[Z_DRIVE]/20.0f;x=sat(x*a)*1.4f*e*v->velocity*p[Z_LEVEL]/127.0f*0.5f;
   if(p[Z_MUTE]||(solo_track&&solo_track!=t+1))x=0;pan=clampf(p[Z_PAN]/128.0f+(v->pad==255?0:v->settings.pan/128.0f),-0.5f,0.5f);l+=x*(0.5f-pan);r+=x*(0.5f+pan);sendd+=x*p[Z_DELAY]/127.0f;sendr+=x*p[Z_REVERB]/127.0f;
  }
  {uint32_t di=(delay_i+8192-6615)&8191;float dl=delay_l[di],dr=delay_r[di];delay_l[delay_i]=sendd+dr*0.36f;delay_r[delay_i]=sendd+dl*0.31f;l+=dl*0.5f;r+=dr*0.5f;delay_i=(delay_i+1)&8191;}
  {uint32_t a=comb_i%1499,b=comb_i%1877;float ra=comb_a[a],rb=comb_b[b];comb_a[a]=sendr+ra*0.69f;comb_b[b]=sendr+rb*0.72f;l+=ra*0.24f;r+=rb*0.24f;comb_i++;}
  if(click_left){float c=sine(click_phase)*0.13f*click_left/1100.0f;click_phase+=((pos&15)?1200:1800)/44100.0f;l+=c;r+=c;click_left--;}
  live_phase+=z_song.bpm/2646000.0f*(1u<<z_song.fx_rate);if(live_phase>=1)live_phase-=1;
  live_blend+=((live_requested?1.0f:0.0f)-live_blend)*0.008f;
  if(live_effect){float dry_l=l,dry_r=r,a=z_song.fx_amount/127.0f,x=(l+r)*0.5f;
   switch(live_effect){
    case 1:l*=1-a;r*=1-a;break;
    case 2:l*=live_phase<0.5f?1:1-a;r*=live_phase<0.5f?1:1-a;break;
    case 3:l*=1-a*(0.5f+0.5f*sine(live_phase));r*=1-a*(0.5f+0.5f*sine(live_phase));break;
    case 4:l=(int)(l*(8+120*(1-a)))/(8+120*(1-a));r=(int)(r*(8+120*(1-a)))/(8+120*(1-a));break;
    case 5:l=sat(l*(1+a*12));r=sat(r*(1+a*12));break;
    case 6:l+=(x-l)*a;r+=(x-r)*a;break;
    case 7:{float q=l;l=r;r=q;break;}
    case 8:l*=1-a*(1-live_phase);r*=1-a*(1-live_phase);break;
    case 9:l*=sine(live_phase*64);r*=sine(live_phase*64);break;
    case 10:l*=1+a;r*=1+a;break;
    case 11:l*=1-a;r*=1+a*0.3f;break;
    case 12:r*=1-a;l*=1+a*0.3f;break;
    case 13:l*=live_phase<0.25f?1:1-a;r*=live_phase<0.25f?1:1-a;break;
    case 14:l=sat(l*(1+a*30));r=sat(r*(1+a*30));break;
    case 15:l*=1-a*0.8f;r=-r*(1-a*0.8f);break;
    case 16:l=0;r=0;break;
   }
   l=dry_l+(l-dry_l)*live_blend;r=dry_r+(r-dry_r)*live_blend;
   if(!live_requested&&live_blend<0.0001f){live_effect=0;live_blend=0;}
  }
  {float yl=l-dc_xl+0.99715f*dc_yl,yr=r-dc_xr+0.99715f*dc_yr;dc_xl=l;dc_xr=r;dc_yl=yl;dc_yr=yr;
   out[2*frame]=clampf(yl,-0.98f,0.98f);out[2*frame+1]=clampf(yr,-0.98f,0.98f);}
 }
}
uint32_t z_position(void){return pos;}
uint32_t z_playing(void){return playing;}
uint32_t z_voice_count(void){uint32_t i,n=0;for(i=0;i<8;i++)n+=voices[i].active;return n;}
float *z_buffer(void){return render_buffer;}
uint8_t *z_transfer(void){return transfer;}
uint32_t z_snapshot(void){return (uint32_t)(uintptr_t)&z_song;}
#ifdef Z_WASM
void z_preview_sample(uint32_t slot,uint32_t length,uint32_t rate){uint32_t i;if(slot>=4||length>Z_PCM_MAX)return;for(i=0;i<length;i++)preview_pcm[slot][i]=(int16_t)(transfer[2*i]|transfer[2*i+1]<<8);z_sample(6+slot,preview_pcm[slot],length,rate);}
#endif

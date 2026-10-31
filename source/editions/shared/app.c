/* SPDX-License-Identifier: MIT
 * Original Zombie Core device application and protocol. Hardware/boot/update:
 * fm1-base (MPL-2.0), not the SLOOP or Skiff applications. */
#include "dsp.h"
#include "output.h"
#include "system.h"
static void midi_mode_changed(uint32_t mode);
static void edition_note(uint32_t track,uint32_t note,uint32_t velocity,uint32_t on);
#include "system.c"
#include "z_cyr.h"
#include "z_logo240.h"
static uint8_t drum_choice=36;
static uint32_t panel_keys_before,held_buttons,redraw_at;
static uint8_t key_action[27],held_raw[27],held_local[27],held_channels[27];
static uint8_t midi_local,daw_playing,daw_recording,daw_values[6][6];
static uint16_t daw_bpm;
static uint8_t daw_host;
static uint32_t daw_seen,daw_touch_at;
static char daw_name[48];
static const uint8_t daw_params[4]={Z_CUTOFF,Z_RESONANCE,Z_ATTACK,Z_RELEASE};
static const uint8_t daw_cc[4]={74,71,73,72};
static void midi_cc(uint32_t ch,uint32_t cc,uint32_t value){plat_midi_out(11|((0xb0|ch)<<8)|(cc<<16)|(value<<24));}
static void raw_note(uint32_t ch,uint32_t note,uint32_t vel,uint32_t on){plat_midi_out((on?9:8)|((on?0x90:0x80)|ch)<<8|note<<16|vel<<24);}
static int daw_live(void){return daw_seen&&plat_ms()-daw_seen<3500;}
static void daw_transport(uint32_t record){if(!z_sys.transport)return;if(record){daw_recording=!daw_recording;midi_cc(15,115,daw_recording?127:0);}else{daw_playing=!daw_playing;midi_cc(15,114,daw_playing?127:0);}}
static uint32_t pulse_until[NB],message_until,saved_revision;
static uint8_t held_notes[27],held_tracks[27];
static volatile uint8_t flash_busy;
static uint32_t off_deadline[6][128];
static uint8_t off_arp[6][128];
static void panel_reset(void);
static uint8_t upload_slot=255;
static uint32_t upload_size,upload_at,upload_crc,upload_rate;
static char message[96];
static uint8_t save_requested,save_after_stop; /* SAVE while playing waits for STOP: a flash write would stop the music */
static uint32_t editor_seen;
static uint8_t editor_effect;
/* Explicit HOME+PLAY output check: 0.5 s, quiet ramped triangle, still at MASTER/trim.
 * Bypasses patches/ARP/mutes only; never started by MIDI or on boot. */
static uint32_t audio_test_left;
static float audio_test_phase;
static uint8_t editor_notes[6][16];

static uint32_t crc32(const uint8_t *p,uint32_t n){uint32_t c=0xffffffffu,i,b;for(i=0;i<n;i++){c^=p[i];for(b=0;b<8;b++)c=(c>>1)^((c&1)?0xedb88320u:0);}return ~c;}
#include "project_store.inc"
static int project_save_all(void){
 int rc;
#if Z_SAMPLER_EDITION
 rc=plat_store_save(OBJ_SETTINGS,&z_song,sizeof z_song);if(!rc)rc=system_save();if(!rc)rc=plat_store_save(OBJ_SAMPLER,&z_sampler,sizeof z_sampler);
#else
 rc=system_save();if(!rc)rc=plat_store_save(OBJ_SAMPLER,&z_sampler,sizeof z_sampler);if(!rc)rc=project_pair_save(&z_song,&z_dm);
#endif
 return rc;
}
static uint32_t rd32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void wr32(uint8_t *p,uint32_t x){uint32_t i;for(i=0;i<4;i++)p[i]=(uint8_t)(x>>(8*i));}
static void note_packet(uint32_t t,uint32_t note,uint32_t vel,uint32_t on){if(on&&(z_get(t,Z_MUTE)||(z_solo_state()&&z_solo_state()!=t+1)))return;uint32_t status=(on?0x90:0x80)|(z_get(t,Z_CHANNEL)-1);plat_midi_out((on?9:8)|status<<8|note<<16|vel<<24);}
static void seq_out(uint32_t t,uint32_t n,uint32_t v,uint32_t ms){uint32_t i;if(z_sys.midi_mode)return;if(n==128){for(i=0;i<128;i++)if(off_deadline[t][i]&&(!v||off_arp[t][i])){note_packet(t,i,0,0);off_deadline[t][i]=0;}if(!v){uint32_t ch=z_get(t,Z_CHANNEL)-1;plat_midi_out(11|((0xb0|ch)<<8)|(120u<<16));}return;}off_arp[t][n]=ms>>31;ms&=0x7fffffffu;if(off_deadline[t][n])note_packet(t,n,0,0);note_packet(t,n,v,1);off_deadline[t][n]=plat_ms()+ms+1;}
static void all_off(void){uint32_t t,n;for(t=0;t<27;t++)if(key_action[t]==1){if(held_raw[t])raw_note(held_channels[t],held_notes[t],0,0);key_action[t]=held_local[t]=0;}for(t=0;t<6;t++)for(n=0;n<128;n++)if(off_deadline[t][n]){note_packet(t,n,0,0);off_deadline[t][n]=0;}for(t=0;t<6;t++){z_alloff(t);uint32_t ch=z_get(t,Z_CHANNEL)-1;plat_midi_out(11|((0xb0|ch)<<8)|(64u<<16));plat_midi_out(11|((0xb0|ch)<<8)|(123u<<16));for(n=0;n<16;n++)editor_notes[t][n]=0;}}
/* MIDI clock out, SETTINGS > CLOCK OUT (padding[0] bit 3, default off): standalone mode only. */
static void clock_out_cb(uint32_t status){if(z_sys.midi_mode==0&&(z_sys.padding[0]&8u))plat_midi_out(0x0fu|status<<8);}
static void midi_mode_changed(uint32_t mode){app_panic();midi_local=mode==0;daw_bpm=z_song.bpm;daw_playing=daw_recording=0;daw_seen=0;daw_host=0;daw_name[0]=0;}
/* External MIDI Note On is an attack, including repeated percussion hits without Note Off.
 * Keep physical/editor key deduplication in z_key unchanged. */
static void midi_key(uint32_t t,uint32_t n,uint32_t v,uint32_t on){if(on&&v)z_key(t,n,0,0);z_key(t,n,v,on);}
static void midi_receive(void){uint32_t pkt;int port;while(plat_midi_in(&pkt,&port)){
 uint32_t status=(pkt>>8)&255,n=(pkt>>16)&127,v=(pkt>>24)&127,t;
 if(z_ext.thru&&port==PORT_TRS)plat_midi_out(pkt); /* SYSTEM > TRS THRU: a DIN keyboard on the jack reaches the computer */
 if(status>=0xf8){if(z_sys.midi_mode!=2)z_clock(status);continue;}
 if(status==0xf2){if(z_sys.midi_mode!=2)z_songpos(n|(v<<7));continue;}
 for(t=0;t<6;t++)if(z_ext.omni&&z_sys.midi_mode!=2?t==z_song.selected:(status&15)+1==(uint32_t)z_get(t,Z_CHANNEL)){ /* omni: every channel plays the selected track */
  if(z_sys.midi_mode==2){if((status&0xf0)==0xb0){uint32_t k;for(k=0;k<4;k++)if(n==daw_cc[k])daw_values[t][k]=v;if(n==7)daw_values[t][4]=v;else if(n==10)daw_values[t][5]=v;else if(n==120||n==123)z_alloff(t);}continue;}
  if((status&0xf0)==0x90){edition_note(t,n,v,v!=0);midi_key(t,n,v,v!=0);}
  else if((status&0xf0)==0x80)z_key(t,n,0,0);
  else if((status&0xf0)==0xb0){if(n==123||n==120)z_alloff(t);else if(z_ext.no_cc){}else if(n==121){z_bend(t,8192);z_sustain(t,0);}else if(n==122&&z_sys.midi_mode==0)midi_local=v>=64;else if(n==73)z_set(t,Z_ATTACK,v);else if(n==72)z_set(t,Z_RELEASE,v);else if(n==64)z_sustain(t,v>=64);else if(n==74)z_set(t,Z_CUTOFF,v);else if(n==71)z_set(t,Z_RESONANCE,v);else if(n==1)z_set(t,Z_FM,v);else if(n==7)z_set(t,Z_LEVEL,v);else if(n==10)z_set(t,Z_PAN,(int)v-64);}
  else if((status&0xf0)==0xe0)z_bend(t,n|(v<<7));
  else if((status&0xf0)==0xc0){if(!z_ext.no_pc)z_preset(t,n%Z_PRESETS);}
 }
}}
void app_render(int32_t *lr,uint32_t n,uint32_t master){uint32_t i;float *buf=z_buffer();if(flash_busy){for(i=0;i<n*2;i++)lr[i]=0;return;}midi_receive();z_render(buf,n);if(z_sys.midi_mode==2)for(i=0;i<n*2;i++)buf[i]=0;
 if(audio_test_left)for(i=0;i<n;i++){float x=0;if(audio_test_left){uint32_t age=22050u-audio_test_left;float ramp=age<441?age/441.0f:audio_test_left<441?audio_test_left/441.0f:1.0f;audio_test_phase+=440.0f/44100.0f;if(audio_test_phase>=1)audio_test_phase-=1;x=(audio_test_phase<0.5f?audio_test_phase*4-1:3-audio_test_phase*4)*0.06f*ramp;audio_test_left--;}buf[i*2]=buf[i*2+1]=x;}
 system_output(buf,lr,n,master);}
void app_panic(void){audio_test_left=0;z_transport(0);all_off();}
static void load_samples(void){uint32_t k;
 for(k=0;k<4;k++){const uint8_t *p=plat_data(0xA0000u+k*0xC000u);uint32_t n=p?rd32(p+4):0,rate=p?rd32(p+8):0;
  if(p&&rd32(p)==0x3150435au&&n>1&&n<=Z_PCM_MAX&&rate>=4000&&rate<=44100&&crc32(p+256,n*2)==rd32(p+12))z_sample(k+6,(const int16_t *)(p+256),n,rate);
 }}
static int valid_project(const z_project *s){uint32_t t,p,i;
 if(s->magic!=0x31434f5au||s->version!=3||s->bpm<30||s->bpm>300||s->swing>60||s->selected>=6||s->language>1||s->reserved>1||s->octave < -3||s->octave>3||s->section>=Z_SECTIONS||s->subpage>(s->section==Z_GLO?5:3)||s->step_bank>3||s->recording>1||s->step_edit>63)return 0;
 if(s->rec_mode>1||s->rec_quant>1||s->metronome>1||s->fx_amount>127||s->fx_rate>3)return 0;
 for(t=0;t<6;t++){
  if(s->arp_mode[t]>4||s->arp_rate[t]>3||s->arp_oct[t]<1||s->arp_oct[t]>3||s->arp_hold[t]>1||(s->preset[t]>=Z_PRESETS&&s->preset[t]!=255))return 0;
  for(p=0;p<Z_PARAMS;p++)if(s->track[t].p[p]<z_desc[p].min||s->track[t].p[p]>z_desc[p].max)return 0;
  for(i=0;i<64;i++){const z_step *v=&s->track[t].steps[i];uint32_t j;if(s->duration[t][i]>63)return 0;for(j=0;j<3;j++)if(s->chord[t][i][j]>128)return 0;if(v->note>127||v->velocity>127||v->chance>127||v->gate<1||v->gate>127)return 0;}
 }return 1;
}
void app_init(void){project_pair_boot();z_project saved;audio_test_left=0;audio_test_phase=0;system_init();int n,project_loaded=0,drum_root_matches=0;uint32_t i,t;z_init();saved=z_song;
 n=project_store_load(OBJ_SETTINGS,&saved,sizeof saved);
 if(n==1872&&saved.version!=1)n=-1;
 if(n==1872&&saved.version==1){saved.version=3;saved.padding[1]=0;for(t=0;t<6;t++)saved.preset[t]=255;}
 if(n==1908&&saved.version==2){saved.version=3;saved.padding[1]=0;}
 if((n==sizeof saved||n==1872||n==1908)&&valid_project(&saved)){project_loaded=1;drum_root_matches=saved.padding[0]==1;z_song=saved;if(z_song.padding[0]!=(Z_SAMPLER_EDITION?2:1))for(t=0;t<6;t++)z_song.preset[t]=255;}
 z_song.padding[0]=Z_SAMPLER_EDITION?2:1;
 {z_sampler_state kit;int bytes=plat_store_load(OBJ_SAMPLER,&kit,sizeof kit);if(bytes==sizeof kit&&z_sampler_valid(&kit))z_sampler=kit;}
 if(!Z_SAMPLER_EDITION){z_dm_state kit;int bytes=project_store_load(OBJ_DRUM,&kit,sizeof kit);if(project_loaded&&drum_root_matches&&bytes==sizeof kit&&z_dm_valid(&kit))z_dm=kit;else if(project_loaded&&drum_root_matches&&z_song.padding[1]!=0xD1)z_dm_migrate();else z_dm_init();z_dm_authoritative();}
 z_apply_state();z_song.recording=0;for(t=0;t<6;t++)z_song.arp_hold[t]=0;z_external(z_song.reserved);load_samples();z_set_midi_out(seq_out);z_set_clock_out(clock_out_cb);
 panel_keys_before=held_buttons=0;editor_effect=0;message[0]=0;message_until=0;redraw_at=0;save_requested=save_after_stop=0;upload_slot=255;
 for(i=0;i<27;i++)key_action[i]=0;for(i=0;i<NB;i++)pulse_until[i]=0;
 for(t=0;t<6;t++){for(i=0;i<128;i++)off_deadline[t][i]=0;for(i=0;i<16;i++)editor_notes[t][i]=0;}
 for(t=0;t<6;t++){for(i=0;i<4;i++)daw_values[t][i]=(uint8_t)z_get(t,daw_params[i]);daw_values[t][4]=(uint8_t)z_get(t,Z_LEVEL);daw_values[t][5]=(uint8_t)(z_get(t,Z_PAN)+64);}daw_bpm=z_song.bpm;daw_host=0;
 midi_local=z_sys.midi_mode==0;daw_seen=0;daw_playing=daw_recording=0;daw_name[0]=0;panel_reset();z_song.section=z_sys.start==1?Z_SEQ:z_sys.start==2?Z_SEL:Z_HOME;z_song.subpage=0;saved_revision=z_revision();
#if !Z_SAMPLER_EDITION
 if(pair_boot_status){if(pair_boot_status==2)saved_revision=0;app_say(pair_boot_status==1?(z_song.language?"PREVIOUS PROJECT RECOVERED":"ВОССТАНОВЛЕН ПРЕЖНИЙ ПРОЕКТ"):(z_song.language?"PROJECT DAMAGED / EMPTY START":"ПРОЕКТ ПОВРЕЖДЕН / ПУСТОЙ СТАРТ"),0);}
#endif
}
void app_say(const char *a,const char *b){uint32_t i;(void)b;for(i=0;i<95&&a&&a[i];i++)message[i]=a[i];message[i]=0;message_until=plat_ms()+1800;redraw_at=0;}
#include "panel.c"

void app_input_only(void){uint32_t f=plat_lock();input();plat_unlock(f);}
void app_frame(void){app_input_only();system_service();
 if(save_requested&&(z_playing()||z_countin_state())){save_requested=0;save_after_stop=1;app_say(word("СОХРАНЮ ПОСЛЕ STOP","SAVES WHEN STOPPED"),0);}
 if(save_after_stop&&!z_playing()&&!z_countin_state()){save_after_stop=0;save_requested=1;}
 if(save_requested){int rc;uint32_t f=plat_lock();save_requested=0;app_panic();flash_busy=1;plat_unlock(f);rc=project_save_all();flash_busy=0;if(!rc)saved_revision=z_revision();else saved_revision=0;app_say(rc?word("СБОЙ СОХРАНЕНИЯ","SAVE FAILED"):word("ПРОЕКТ СОХРАНЕН","PROJECT SAVED"),0);}if(plat_ms()-redraw_at>65||!redraw_at){draw_ui();redraw_at=plat_ms();}}
void app_ui_init(void){palette_set(0);draw_ui();}
void app_splash(void){edition_splash();}
/* Independent ZC protocol: F0 7D 5A 43 request-id command pack7(payload) F7.
 * Replies use command|64 and raw status byte preceding payload. */
static uint8_t reply_frame[4010],raw_reply[3500],state_cache[sizeof(z_project)];
static uint32_t snapshot_token,snapshot_revision;
static void reply(uint32_t seq,uint32_t cmd,uint8_t status,const uint8_t *data,uint32_t n){uint32_t i=0,o=6,j;raw_reply[0]=status;if(n>3499)n=3499;for(j=0;j<n;j++)raw_reply[j+1]=data[j];n++;
 reply_frame[0]=0xf0;reply_frame[1]=0x7d;reply_frame[2]=0x5a;reply_frame[3]=0x43;reply_frame[4]=(uint8_t)seq;reply_frame[5]=(uint8_t)(cmd|64);
 while(i<n){uint32_t h=o++;reply_frame[h]=0;for(j=0;j<7&&i<n;j++,i++){reply_frame[h]|=(raw_reply[i]>>7)<<j;reply_frame[o++]=raw_reply[i]&127;}}
 reply_frame[o++]=0xf7;plat_sysex_send(reply_frame,o);
}
int app_sysex(const uint8_t *wire,uint32_t len){uint8_t a[240],out[220],status=0;uint32_t n=0,i=5,j,k,seq,cmd,count=0,f;
 if(len<5||wire[0]!=0x7d||wire[1]!=0x5a||wire[2]!=0x43)return 0;
 seq=wire[3];cmd=wire[4];while(i<len){uint32_t hi=wire[i++];for(j=0;j<7&&i<len;j++){if(n>=sizeof a){reply(seq,cmd,1,0,0);return 1;}a[n++]=(uint8_t)(wire[i++]|((hi>>j&1)<<7));}}
 /* DAW heartbeat must not keep abandoned browser keys/effects alive. */
 if(cmd!=32&&cmd!=33)editor_seen=plat_ms();
 f=plat_lock();
 if(cmd==1){const char *id=Z_PROTOCOL_ID;while(id[count]){out[count]=(uint8_t)id[count];count++;}}
 else if(cmd==2){for(j=0;j<sizeof z_song;j++)raw_reply[j+1]=((const uint8_t *)&z_song)[j];plat_unlock(f);reply(seq,cmd,0,raw_reply+1,sizeof z_song);return 1;}
 else if(cmd==3&&n==4&&a[0]<6&&a[1]<Z_PARAMS){int v=z_set(a[0],a[1],(int16_t)(a[2]|a[3]<<8));out[0]=(uint8_t)v;out[1]=(uint8_t)(v>>8);count=2;}
 else if(cmd==4&&n==4&&a[0]<6&&a[1]<128&&a[2]<128){edition_note(a[0],a[1],a[2],a[3]);z_key(a[0],a[1],a[2],a[3]);if(!z_song.arp_mode[a[0]]||!a[3])note_packet(a[0],a[1],a[2],a[3]&&a[2]);if(a[3]&&a[2])editor_notes[a[0]][a[1]/8]|=1u<<(a[1]&7);else editor_notes[a[0]][a[1]/8]&=(uint8_t)~(1u<<(a[1]&7));}
 else if(cmd==5&&n==1){all_off();z_transport(a[0]);}
 else if(cmd==6&&n==6&&a[0]<6&&a[1]<64)z_step_set(a[0],a[1],a[2],a[3],a[4],a[5]);
 else if(cmd==7&&n==2&&a[0]<6&&a[1]<Z_PRESETS){z_preset(a[0],a[1]);}
 else if(cmd==8&&n==3)z_tempo(a[0]|a[1]<<8,a[2]);
 else if(cmd==9&&n==0){app_panic();flash_busy=1;plat_unlock(f);status=project_save_all()?5:0;if(!status)saved_revision=z_revision();else saved_revision=0;flash_busy=0;reply(seq,cmd,status,0,0);return 1;}
 else if(cmd==10&&n==3&&a[0]<2&&a[1]<2&&a[2]<6){z_external(a[0]);z_song.language=a[1];z_song.selected=a[2];z_touch();}
 else if(cmd==11&&n==11&&a[0]<4){uint32_t bytes=rd32(a+1),rate=a[5]|a[6]<<8;if(bytes<4||bytes>Z_PCM_MAX*2||bytes%2||rate<4000||rate>44100)status=1;else {app_panic();upload_slot=a[0];upload_size=bytes;upload_rate=rate;upload_crc=rd32(a+7);upload_at=0;}}
 else if(cmd==12&&n>=3&&upload_slot<4){uint32_t at=a[0]|a[1]<<8;if(at!=upload_at||at+n-2>upload_size)status=2;else{for(j=2;j<n;j++)z_transfer()[upload_at++]=a[j];}}
 else if(cmd==13&&n==0&&upload_slot<4){
  uint8_t head[256];uint32_t off=0xA0000u+upload_slot*0xC000u,k;
  if(upload_at!=upload_size||crc32(z_transfer(),upload_size)!=upload_crc)status=3;
  else {app_panic();z_sample(upload_slot+6,0,0,22050);plat_unlock(f);flash_busy=1;
   for(k=0;k<256;k++)head[k]=0;wr32(head,0x3150435au);wr32(head+4,upload_size/2);wr32(head+8,upload_rate);wr32(head+12,upload_crc);
   for(k=0;k<256+upload_size;k+=4096)if(plat_data_erase(off+k)){status=5;break;}
   if(!status)for(k=0;k<upload_size;k+=256){uint32_t amount=upload_size-k;if(amount>256)amount=256;if(plat_data_write(off+256+k,z_transfer()+k,amount)){status=5;break;}}
   if(!status&&crc32(plat_data(off+256),upload_size)!=upload_crc)status=3;
   if(!status&&plat_data_write(off,head,256))status=5;
   if(!status)z_sample(upload_slot+6,(const int16_t *)plat_data(off+256),upload_size/2,upload_rate);
   flash_busy=0;upload_slot=255;reply(seq,cmd,status,0,0);return 1;
  }
 }
 else if(cmd==14&&n==0)upload_slot=255;
 else if(cmd==15&&n==0){for(k=0;k<4;k++){const uint8_t *p=plat_data(0xA0000u+k*0xC000u);wr32(out+8*k,p&&rd32(p)==0x3150435au?rd32(p+4):0);wr32(out+8*k+4,p?rd32(p+8):0);}count=32;}
 else if(cmd==16&&n==4){app_panic();upload_slot=254;upload_size=sizeof z_song;upload_at=0;upload_crc=rd32(a);}
 else if(cmd==17&&n>=3&&(upload_slot==254||upload_slot==253||upload_slot==252)){uint32_t at=a[0]|a[1]<<8;if(at!=upload_at||at+n-2>upload_size)status=2;else for(j=2;j<n;j++)z_transfer()[upload_at++]=a[j];}
 else if(cmd==18&&n==0&&upload_slot==254){if(upload_at!=sizeof z_song||crc32(z_transfer(),upload_at)!=upload_crc||!valid_project((const z_project *)z_transfer()))status=3;else {app_panic();z_song=*(const z_project *)z_transfer();z_song.recording=0;if(!Z_SAMPLER_EDITION){if(z_song.padding[1]==0xD1)z_dm_init();else z_dm_migrate();z_dm_authoritative();}z_history_reset();panel_reset();z_apply_state();z_external(z_song.reserved);z_touch();}upload_slot=255;}
 else if(cmd==19&&n==7){
  uint32_t at=a[0]|a[1]<<8,amount=a[2],token=rd32(a+3);
  if(!amount||amount>192||at+amount>sizeof z_song)status=1;
  else if(at==0&&token==0){for(j=0;j<sizeof z_song;j++)state_cache[j]=((const uint8_t *)&z_song)[j];if(++snapshot_token==0)snapshot_token=1;snapshot_revision=z_revision();}
  else if(!snapshot_token||token!=snapshot_token)status=2;
  if(!status){wr32(out,snapshot_token);wr32(out+4,snapshot_revision);out[8]=(uint8_t)at;out[9]=(uint8_t)(at>>8);for(j=0;j<amount;j++)out[10+j]=state_cache[at+j];count=amount+10;}
 }
 else if(cmd==20&&n==0){out[0]=2;out[1]=(uint8_t)z_playing();out[2]=(uint8_t)z_position();out[3]=z_song.recording;out[4]=(uint8_t)(z_song.octave+3);out[5]=z_song.section;out[6]=z_song.subpage;out[7]=z_song.step_bank;out[8]=z_song.step_edit;out[9]=z_song.selected;out[10]=z_song.preset[z_song.selected];out[11]=saved_revision!=z_revision()||system_dirty;wr32(out+12,z_revision());out[16]=(uint8_t)(active_layer+1);out[17]=(uint8_t)z_live_state();out[18]=(uint8_t)z_song.bpm;out[19]=(uint8_t)(z_song.bpm>>8);out[20]=(uint8_t)z_history_state();out[21]=(uint8_t)(z_history_state()>>8);out[22]=(uint8_t)z_solo_state();count=23;out[0]=4;if(!Z_SAMPLER_EDITION){for(j=0;j<16;j++)out[23+j]=(uint8_t)z_dm_position(j);wr32(out+39,z_position());count=43;out[0]=6;}}
 else if(cmd==21&&n==2){if(!panel_setting(a[0],a[1]))status=1;}
 else if(cmd==23&&n==1&&a[0]<=16){editor_effect=a[0];z_live(a[0]);}
 else if(cmd==25&&n==1&&a[0]<=1){out[0]=(uint8_t)z_history(a[0]);count=1;}
 else if(cmd==26&&n==3){if(z_pattern(a[0],a[1],a[2]))status=1;}
 else if(cmd==27&&n==1&&a[0]<=6)z_solo(a[0]);
 else if(cmd==28&&n==0){const uint8_t *p=(const uint8_t *)&z_sys;for(j=0;j<sizeof z_sys;j++)out[j]=p[j];out[112]=core_status;out[113]=output_peak;wr32(out+114,core_jobs);wr32(out+118,core_timeouts);{const uint8_t *e=(const uint8_t *)&z_ext;for(j=0;j<8;j++)out[122+j]=e[j];}count=130;}
 else if(cmd==33&&n==1&&a[0]<=2){daw_host=a[0];}
 else if(cmd==34&&n==0){plat_unlock(f);reply(seq,cmd,0,(const uint8_t *)&z_sampler,sizeof z_sampler);return 1;}
 else if(cmd==35&&n==4){int value=a[2]|a[3]<<8;if(a[1]!=0&&a[1]!=1)value=(int16_t)value;if(z_sampler_set(a[0],a[1],value))status=1;}
 else if(cmd==36&&n==3&&a[0]<6&&a[1]<=1&&a[2]<32){if(((z_sampler.tracks>>a[0])&1)!=a[1])z_sampler_enable(a[0],a[1]);z_sampler.selected=a[2];z_sampler.bank=a[2]/16;z_touch();}
 else if(cmd==37&&n==2){if(z_sampler_chop(a[0],a[1]))status=1;}
 else if(cmd==38&&n==0){app_panic();plat_unlock(f);flash_busy=1;status=plat_store_save(OBJ_SAMPLER,&z_sampler,sizeof z_sampler)?5:0;flash_busy=0;reply(seq,cmd,status,0,0);return 1;}
 else if(cmd==39&&n==1&&a[0]<14){out[0]=a[0];wr32(out+1,z_sample_frames(a[0]));wr32(out+5,z_sample_rate(a[0]));for(j=0;j<96;j++)out[9+j]=(uint8_t)(z_sample_peak(a[0],j,96)>>8);count=105;}
 else if(cmd==40&&n==4){app_panic();upload_slot=253;upload_size=sizeof z_sampler;upload_at=0;upload_crc=rd32(a);}
 else if(cmd==41&&n==0&&upload_slot==253){if(upload_at!=sizeof z_sampler||crc32(z_transfer(),upload_at)!=upload_crc||!z_sampler_valid((const z_sampler_state *)z_transfer()))status=3;else{app_panic();z_sampler=*(const z_sampler_state *)z_transfer();z_sampler.tracks=Z_SAMPLER_EDITION?63:0;z_touch();}upload_slot=255;}

 else if(!Z_SAMPLER_EDITION&&cmd==42&&n==1&&a[0]<16)z_dm_select(a[0]);
 else if(!Z_SAMPLER_EDITION&&cmd==43&&n==0){plat_unlock(f);reply(seq,cmd,0,(const uint8_t *)&z_dm,sizeof z_dm);return 1;}
 else if(!Z_SAMPLER_EDITION&&cmd==44&&n==4){if(z_dm_param(a[0],a[1],(int16_t)(a[2]|a[3]<<8)))status=1;}
 else if(!Z_SAMPLER_EDITION&&cmd==45&&n==5){if(z_dm_step(a[0],a[1],a[2],a[3],a[4]))status=1;}
 else if(!Z_SAMPLER_EDITION&&cmd==47&&n==4){app_panic();upload_slot=252;upload_size=sizeof z_dm;upload_at=0;upload_crc=rd32(a);}
 else if(!Z_SAMPLER_EDITION&&cmd==48&&n==0&&upload_slot==252){if(upload_at!=sizeof z_dm||crc32(z_transfer(),upload_at)!=upload_crc||!z_dm_valid((const z_dm_state *)z_transfer()))status=3;else{app_panic();z_dm_apply(upload_at);}upload_slot=255;}
 else if(cmd==32&&n>=7&&n<=54&&a[0]<=1&&a[1]<=1&&(a[4]<6||a[4]==255)&&a[5]<=127&&a[6]<=127&&system_text(a+7,n-7)){
  uint32_t bpm=a[2]|a[3]<<8;if(bpm<30||bpm>300)status=1;else{daw_playing=a[0];daw_recording=a[1];daw_seen=plat_ms();for(j=7;j<n;j++)daw_name[j-7]=(char)a[j];daw_name[n-7]=0;if(z_sys.midi_mode==2){if(plat_ms()-daw_touch_at>300)daw_bpm=(uint16_t)bpm;if(a[4]<6&&z_song.selected!=a[4]){z_song.selected=a[4];z_touch();}if(plat_ms()-daw_touch_at>300){daw_values[z_song.selected][4]=a[5];daw_values[z_song.selected][5]=a[6];}}}
 }
 else if(cmd==29&&n==2){if(!system_set(a[0],a[1]))status=1;}
 else if(cmd==30&&n>=1&&a[0]<2){char *dst=a[0]?z_sys.bio:z_sys.name;uint32_t cap=a[0]?64:32;if(n-1>=cap||!system_text(a+1,n-1))status=1;else{system_dirty=1;for(j=1;j<n;j++)dst[j-1]=(char)a[j];dst[n-1]=0;}}
 else if(cmd==31&&n==0){app_panic();plat_unlock(f);flash_busy=1;status=system_save()?5:0;flash_busy=0;reply(seq,cmd,status,0,0);return 1;}

 else if(cmd==24&&n==10&&a[0]<6&&!z_drum_track(a[0])&&a[1]<64&&a[2]<=63&&a[3]<=128&&a[4]<=128&&a[5]<=128&&a[6]<=127&&a[7]<=127&&a[8]<=127&&a[9]>=1&&a[9]<=127){
  uint32_t t=a[0],at=a[1],changed=z_song.duration[t][at]!=a[2];z_step *s=&z_song.track[t].steps[at];if(s->note!=a[6]||s->velocity!=a[7]||s->chance!=a[8]||s->gate!=a[9])changed=1;for(j=0;j<3;j++)if(z_song.chord[t][at][j]!=a[3+j])changed=1;if(changed){if(z_song.recording){z_record(0);z_alloff(t);}z_checkpoint();*s=(z_step){a[6],a[7],a[8],a[9]};z_song.duration[t][at]=a[2];for(j=0;j<3;j++)z_song.chord[t][at][j]=a[3+j];z_touch();}
 }
 else if(cmd==49&&n==3){if(z_erase(a[0],a[1],a[2]))status=1;}
 else if(cmd==22&&n==3){if(z_arp_set(a[0],a[1],a[2]))status=1;}
 else status=1;
 plat_unlock(f);reply(seq,cmd,status,out,count);return 1;
}
#ifdef PLAT_HOST
#include "edition_host.inc"
#endif

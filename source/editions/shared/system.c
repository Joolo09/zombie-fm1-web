/* SPDX-License-Identifier: MIT — original settings and bounded CPU1 output worker. */
static z_system_settings z_sys;
static volatile uint32_t core_status,output_peak,core_jobs,core_timeouts;
static uint8_t core_pending,system_dirty;
typedef struct {const float *src;int32_t *dst;uint32_t frames,master,trim,limiter;
 volatile uint32_t request,done,peak;} z_core_job;
static volatile z_core_job core_job;
#ifdef PLAT_HOST
/* Fault injection exercises the same timeout/fallback paths in the simulator. */
static uint32_t core_test_fault;
#else
__attribute__((section(".ram_text"),noreturn,used)) void fm1_cpu1_main(void){
 uint32_t seen=0;PLAT_CORE_FENCE();plat_core_ready_flag=1;PLAT_CORE_FENCE();
 for(;;){uint32_t request=core_job.request;PLAT_CORE_FENCE();if(request!=seen){
  core_job.peak=z_output_kernel(core_job.src,core_job.dst,core_job.frames,core_job.master,core_job.trim,core_job.limiter);
  seen=request;PLAT_CORE_FENCE();core_job.done=request;PLAT_CORE_FENCE();
 }}
}
#endif
static void system_stop(void){
#ifndef PLAT_HOST
 plat_core_stop();
#endif
 core_job.request=core_job.done=0;
}
static int core_run(const float *src,int32_t *dst,uint32_t n,uint32_t master,uint32_t trim,uint32_t limiter){
 uint32_t request=core_job.request+1;core_job.src=src;core_job.dst=dst;core_job.frames=n;
 core_job.master=master;core_job.trim=trim;core_job.limiter=limiter;
 PLAT_CORE_FENCE();core_job.request=request;PLAT_CORE_FENCE();
#ifdef PLAT_HOST
 if(core_test_fault!=3){core_job.peak=z_output_kernel(src,dst,n,master,trim,limiter);if(core_test_fault==2&&n)dst[0]^=1;core_job.done=request;}
#else
 {uint32_t start=plat_core_ticks();while(core_job.done!=request){PLAT_CORE_FENCE();if((uint32_t)(plat_core_ticks()-start)>36000u)return 0;}}
#endif
 PLAT_CORE_FENCE();return core_job.done==request;
}
static void system_output(const float *src,int32_t *dst,uint32_t n,uint32_t master){
 if(core_status==Z_CORE_RUNNING||core_status==Z_CORE_SIMULATED){
  if(core_run(src,dst,n,master,z_sys.trim,z_sys.limiter)){core_jobs++;output_peak=core_job.peak;return;}
  system_stop();core_status=Z_CORE_TIMEOUT;core_timeouts++;
 }
 output_peak=z_output_kernel(src,dst,n,master,z_sys.trim,z_sys.limiter);
}
static int system_text(const uint8_t *p,uint32_t n){uint32_t i=0;
 while(i<n){uint32_t ch=p[i++];if(ch>=32&&ch<127)continue;
  if((ch==0xd0||ch==0xd1)&&i<n&&(p[i]&0xc0)==0x80){ch=(ch&31)*64+(p[i++]&63);if(ch>=0x400&&ch<0x460)continue;}return 0;
 }return 1;
}
static int system_valid(const z_system_settings *s){uint32_t i;
 if(s->magic!=Z_SYSTEM_MAGIC||s->version!=2||s->midi_mode>2||s->transport>1||s->dual>1||s->trim>127||s->limiter>1||s->glow>2||s->velocity>2||s->start>2)return 0;
 for(i=0;i<32&&s->name[i];i++){}if(i==32||!system_text((const uint8_t *)s->name,i))return 0;
 for(i=0;i<64&&s->bio[i];i++){}return i<64&&system_text((const uint8_t *)s->bio,i);
}
/* padding[0] holds the recording options; 0 (every older settings file) means the defaults: count-in 1 bar, click on.
 * bits 0-1: 0 = 1 bar, 1 = off, 2 = 2 bars; bit 2: click while recording off. Older firmware ignores the byte. */
static uint32_t rec_countin_bars(void){uint32_t c=z_sys.padding[0]&3u;return c==0?1:c==1?0:2;}
static uint32_t rec_click_on(void){return !(z_sys.padding[0]&4u);}
/* padding[1]: low nibble the scale lock (0 off, 1 major ... 9 harmonic minor), high nibble its root (0 = C ... 11 = B). */
#define Z_SCALES 10
static const uint16_t scale_mask[Z_SCALES]={0xFFF,0xAB5,0x5AD,0x6AD,0x5AB,0x6B5,0x295,0x4A9,0x4E9,0x9AD};
static uint32_t scale_index(void){uint32_t s=z_sys.padding[1]&15u;return s<Z_SCALES?s:0;}
static uint32_t scale_root(void){uint32_t r=z_sys.padding[1]>>4;return r<12?r:0;}
static uint32_t scale_has(int note){uint32_t s=scale_index();return !s||(scale_mask[s]>>(((note+240)-(int)scale_root())%12)&1u);}
/* a key's note moved to the nearest note of the scale (a tie goes down) */
static int scale_snap_up(int note){int d;if(scale_has(note))return note;for(d=1;d<7;d++){if(note+d<=127&&scale_has(note+d))return note+d;if(note-d>=0&&scale_has(note-d))return note-d;}return note;}
static int scale_snap(int note){int d;if(scale_has(note))return note;for(d=1;d<7;d++){if(note-d>=0&&scale_has(note-d))return note-d;if(note+d<=127&&scale_has(note+d))return note+d;}return note;}
static void system_init(void){
 z_system_settings defaults={Z_SYSTEM_MAGIC,2,0,100,1,1,0,0,"Мой FM-1","",0,1,{0,0}},saved;int n;
 core_status=Z_CORE_OFF;system_stop();core_pending=system_dirty=0;core_jobs=core_timeouts=output_peak=0;z_sys=defaults;
 saved=defaults;n=plat_store_load(OBJ_SYSTEM,&saved,sizeof saved);
 if(n==108&&saved.version==1)saved.version=2;
 if((n==108||n==sizeof saved)&&system_valid(&saved))z_sys=saved;
 z_sys.midi_mode=0; /* Always boot as a local synth, including upgrades with saved DAW settings. */
 z_sys.dual=0; /* Experimental hardware mode always requires an explicit session opt-in. */
 z_sys.padding[0]&=7u;z_rec_click(rec_click_on());if((z_sys.padding[1]&15u)>=Z_SCALES||(z_sys.padding[1]>>4)>=12)z_sys.padding[1]=0;
}
static int system_save(void){z_system_settings saved=z_sys;saved.dual=0;int rc=plat_store_save(OBJ_SYSTEM,&saved,sizeof saved);if(!rc)system_dirty=0;return rc;}
static int system_set(uint32_t id,uint32_t value){
 static const uint8_t maximum[]={1,127,1,2,2,2,2,1,2,1,9,11};uint8_t *p;
 if(id>=12||value>maximum[id])return 0;
 if(id>=10){uint8_t before=z_sys.padding[1];z_sys.padding[1]=(uint8_t)(id==10?(before&0xf0)|value:(before&15)|(value<<4));if(before!=z_sys.padding[1])system_dirty=1;return 1;}
 if(id>=8){uint8_t before=z_sys.padding[0];if(id==8)z_sys.padding[0]=(uint8_t)((z_sys.padding[0]&~3u)|(value==1?0u:value==0?1u:2u));else{z_sys.padding[0]=(uint8_t)(value?z_sys.padding[0]&~4u:z_sys.padding[0]|4u);z_rec_click(value);}if(before!=z_sys.padding[0])system_dirty=1;return 1;}p=id<6?&z_sys.dual+id:id==6?&z_sys.midi_mode:&z_sys.transport;
 if(id&&*p!=value)system_dirty=1;if(id==6&&*p!=value)midi_mode_changed(value);*p=(uint8_t)value;if(id==0)core_pending=1;return 1;
}
static uint32_t system_velocity(uint32_t v){if(v>127)v=127;
 if(z_sys.velocity==1)v=127-(127-v)*(127-v)/127;else if(z_sys.velocity==2)v=100;return v?v:1;}
static void system_service(void){
 if(core_pending){uint32_t i,peak;int32_t expected[16],actual[16];
  static const float probe[16]={0,0.2f,-0.2f,0.9f,-0.9f,1.5f,-1.5f,0.01f,0,0.5f,-0.5f,0.85f,-0.85f,0.999f,-0.999f,0};
  core_pending=0;core_status=Z_CORE_OFF;system_stop();if(!z_sys.dual)return;
  {uint32_t lock=plat_lock();app_panic();plat_unlock(lock);}
#ifdef PLAT_HOST
  if(core_test_fault==1){core_status=Z_CORE_START_FAILED;return;}
#else
  if(plat_core_start()){core_status=Z_CORE_START_FAILED;return;}
#endif
  peak=z_output_kernel(probe,expected,8,4096,100,1);
  if(!core_run(probe,actual,8,4096,100,1)){system_stop();core_status=Z_CORE_TIMEOUT;core_timeouts++;return;}
  for(i=0;i<16;i++)if(expected[i]!=actual[i])break;
  if(i!=16||peak!=core_job.peak){system_stop();core_status=Z_CORE_TEST_FAILED;return;}
#ifdef PLAT_HOST
  core_status=Z_CORE_SIMULATED;
#else
  core_status=Z_CORE_RUNNING;
#endif
 }
}

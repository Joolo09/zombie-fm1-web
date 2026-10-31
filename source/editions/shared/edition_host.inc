/* SPDX-License-Identifier: MIT — simulated firmware tests, never real USB. */
static void ed_check(int condition,const char *message){if(!condition){fprintf(stderr,"FAIL: %s\n",message);exit(1);}printf("PASS: %s\n",message);}
static void ed_roll(void){uint32_t t=3;system_set(6,0);app_panic();z_song.selected=t;z_song.language=0;z_song.section=Z_SEQ;z_song.subpage=0;z_song.step_bank=z_song.step_edit=0;z_preset(t,16);z_clear_track(t);z_step_set(t,0,60,100,127,80);z_song.duration[t][0]=4;z_song.chord[t][0][0]=65;z_song.chord[t][0][1]=68;z_step_set(t,6,67,88,127,80);z_song.duration[t][6]=2;z_step_set(t,10,63,80,127,80);redraw_at=0;}
static void ed_checks(void){uint32_t t,p;int old;
 ed_check(z_sys.midi_mode==0&&midi_local,"startup is standalone with local sound");
 ed_check(!z_playing()&&!z_song.recording,"boot never auto-starts playback or recording");
 ed_check(z_sampler.tracks==(Z_SAMPLER_EDITION?63:0),"edition pad capabilities enforced");
 for(t=0;t<6;t++){old=z_get(t,Z_MODEL);z_set(t,Z_MODEL,Z_SAMPLER_EDITION?0:8);ed_check(z_get(t,Z_MODEL)==(Z_SAMPLER_EDITION?8:7),"foreign engine cannot enter this edition");z_set(t,Z_MODEL,old);}
 for(p=0;p<Z_PRESETS;p++){z_preset(0,p);ed_check(Z_SAMPLER_EDITION?z_get(0,Z_MODEL)==8:z_get(0,Z_MODEL)<8,"factory preset uses the correct engine family");}
 if(Z_SAMPLER_EDITION){ed_check(z_sample_frames(10)>0&&z_sample_frames(0)>0,"factory drum and zombie PCM available");ed_check(!z_sampler_enable(0,0)&&z_sampler.tracks==63,"dedicated sampler cannot accidentally disable pads");z_song.track[0].p[Z_MODEL]=0;z_song.track[0].p[Z_SUSTAIN]=0;z_song.track[0].p[Z_ATTACK]=127;z_apply_state();ed_check(z_get(0,Z_MODEL)==8&&z_get(0,Z_SUSTAIN)==127&&z_get(0,Z_ATTACK)==0,"legacy synth project cannot mute sampler envelope");}
 else{ed_check(z_sample_frames(0)==0,"groovebox does not carry Minecraft PCM");ed_check(z_sampler_enable(0,1)==-1&&z_sampler.tracks==0,"groovebox cannot enable hidden sampler");}
 system_set(6,2);ed_check(system_save()==0,"DAW settings saved in simulated flash");app_init();ed_check(z_sys.midi_mode==0&&midi_local,"saved controller mode cannot mute startup");
}
static void ed_project_audit(void){
 if(Z_SAMPLER_EDITION)return;
 app_panic();for(uint32_t o=0;o<OBJ_NOBJ;o++)store_len[o]=-1;app_init();
 for(uint32_t t=0;t<6;t++)ed_check(z_track_hits(t)==0,"cold boot: empty track");
 ed_check(z_song.padding[1]==0xD1,"new project owns its drum state");
 z_project legacy=z_song;legacy.padding[1]=0;legacy.track[5].steps[0]=(z_step){36,100,127,80};legacy.chord[5][0][0]=39;
 for(uint32_t v=1;v<=2;v++){legacy.version=v;store_len[OBJ_DRUM]=-1;plat_store_save(OBJ_SETTINGS,&legacy,v==1?1872:1908);app_init();ed_check(z_track_hits(5)==1,"partial legacy wire state keeps its GM base note");}legacy.version=3;
 ed_check(plat_store_save(OBJ_SETTINGS,&legacy,sizeof legacy)==0,"legacy root saved");store_len[OBJ_DRUM]=-1;app_init();
 ed_check(z_track_hits(5)==2,"legacy GM drums migrate once");ed_check(z_song.track[5].steps[0].velocity==0&&z_song.padding[1]==0xD1,"legacy shadow retired after migration");
 z_erase(5,0,2);ed_check(plat_store_save(OBJ_SETTINGS,&z_song,sizeof z_song)==0&&plat_store_save(OBJ_DRUM,&z_dm,sizeof z_dm)==0,"erased kit and root saved");app_init();ed_check(z_track_hits(5)==0,"erased drums stay erased after reboot");
 z_song.track[5].steps[0]=(z_step){36,100,127,80};plat_store_save(OBJ_SETTINGS,&z_song,sizeof z_song);store_len[OBJ_DRUM]=-1;app_init();ed_check(z_track_hits(5)==0,"missing kit cannot resurrect an authoritative legacy shadow");
 z_dm_step(0,0,100,127,1);plat_store_save(OBJ_DRUM,&z_dm,sizeof z_dm);store_len[OBJ_SETTINGS]=-1;app_init();ed_check(z_track_hits(5)==0,"orphaned drum journal cannot overlay a new project");
 z_project foreign=z_song;foreign.padding[0]=2;z_dm_step(0,0,100,127,1);plat_store_save(OBJ_DRUM,&z_dm,sizeof z_dm);plat_store_save(OBJ_SETTINGS,&foreign,sizeof foreign);app_init();ed_check(z_track_hits(5)==0,"foreign-edition root cannot import a stale Groovebox drum kit");
 z_project bad=z_song;bad.track[0].steps[0].chance=128;ed_check(!valid_project(&bad),"invalid chance rejected");bad=z_song;bad.track[0].p[Z_CHANNEL]=17;ed_check(!valid_project(&bad),"invalid MIDI channel rejected");
 for(uint32_t o=0;o<OBJ_NOBJ;o++)store_len[o]=-1;app_init();
}
/* Fuzz support: every saved or transient field must stay inside what the firmware itself can produce. */
static uint32_t inv_fail;
static void inv(int ok,const char *what,int a,int b){if(!ok){fprintf(stderr,"INVARIANT FAIL: %s (%d,%d)\n",what,a,b);inv_fail=1;}}
static void ed_invariants(void){uint32_t t,i,j;inv_fail=0;
 inv(z_song.selected<6,"selected track",z_song.selected,0);inv(z_song.section<Z_SECTIONS,"section",z_song.section,0);
 inv(z_song.subpage<page_count[z_song.section<Z_SECTIONS?z_song.section:0],"subpage",z_song.section,z_song.subpage);
 inv(z_song.step_edit<64,"step cursor",z_song.step_edit,0);inv(z_song.step_bank<4,"step bank",z_song.step_bank,0);inv(z_song.step_bank==z_song.step_edit/16,"bank follows cursor",z_song.step_bank,z_song.step_edit);
 inv(z_song.octave>=-3&&z_song.octave<=3,"octave",z_song.octave,0);inv(z_song.bpm>=30&&z_song.bpm<=300,"bpm",z_song.bpm,0);inv(z_song.swing<=60,"swing",z_song.swing,0);
 inv(z_song.language<=1&&z_song.rec_mode<=1&&z_song.rec_quant<=1&&z_song.metronome<=1&&z_song.recording<=1,"flags",z_song.language,z_song.recording);
 inv(z_song.fx_amount<=127&&z_song.fx_rate<=3,"fx",z_song.fx_amount,z_song.fx_rate);
 for(t=0;t<6;t++){
  for(i=0;i<Z_PARAMS;i++)inv(z_song.track[t].p[i]>=z_desc[i].min&&z_song.track[t].p[i]<=z_desc[i].max,z_desc[i].name,t,z_song.track[t].p[i]);
  inv(z_song.arp_mode[t]<=4&&z_song.arp_rate[t]<=3&&z_song.arp_oct[t]>=1&&z_song.arp_oct[t]<=3&&z_song.arp_hold[t]<=1,"arp",t,z_song.arp_mode[t]);
  inv(z_song.preset[t]<Z_PRESETS||z_song.preset[t]==255,"preset",t,z_song.preset[t]);
  for(i=0;i<64;i++){z_step *s=&z_song.track[t].steps[i];inv(s->note<=127&&s->velocity<=127&&s->chance<=127&&s->gate>=1&&s->gate<=127,"step",t,i);
   inv(z_song.duration[t][i]<=63,"duration",t,i);for(j=0;j<3;j++)inv(z_song.chord[t][i][j]<=128,"chord",t,i);}
 }
 if(!Z_SAMPLER_EDITION){inv(z_dm.selected<16,"drum selected",z_dm.selected,0);
  for(i=0;i<16;i++){z_dm_sound_t *p=&z_dm.sound[i];inv(p->level<=127&&p->decay>=1&&p->decay<=127&&p->tune>=-24&&p->tune<=24&&p->pan>=-64&&p->pan<=63&&p->mute<=1&&p->length>=1&&p->length<=64,"drum sound",i,p->length);
   for(j=0;j<64;j++){z_dm_step_t *s=&z_dm.step[i][j];inv(s->velocity<=127&&s->chance<=127&&s->ratchet>=1&&s->ratchet<=4,"drum step",i,j);}}}
 inv((z_sys.padding[1]&15u)<Z_SCALES&&(z_sys.padding[1]>>4)<12,"scale nibbles",z_sys.padding[1]&15,z_sys.padding[1]>>4);inv(!(z_sys.padding[0]&~15u),"recording option bits",z_sys.padding[0],0);
 inv(eu_hits<=64&&eu_rot<64,"euclid knobs",eu_hits,eu_rot);inv(grid_hold.key<27,"held grid key",grid_hold.key,0);inv(z_countin_state()<=8,"count-in beats",z_countin_state(),0);
 inv(z_voice_count()<=24,"voices",z_voice_count(),0);{uint32_t z_dsp_finite(void);inv(z_dsp_finite(),"DSP state finite (no NaN/inf)",0,0);}inv(z_sys.midi_mode<=2,"midi mode",z_sys.midi_mode,0);
 if(inv_fail)exit(3);
}
static uint32_t stuck_skip;
static void ed_stuck_prep(void){uint32_t t;stuck_skip=0;for(t=0;t<6;t++){if(z_song.arp_mode[t]&&z_song.arp_hold[t])stuck_skip=1;z_sustain(t,0);}z_countin(0);if(z_playing())z_transport(0);if(z_song.recording)z_record(0);}
static void ed_stuck_expect(void){if(stuck_skip||z_sys.midi_mode==1)return;inv_fail=0;{uint32_t z_held_voices(void);inv(z_held_voices()==0,"notes still held after every key, MIDI note and pedal released + STOP",z_held_voices(),0);}if(inv_fail)exit(4);}
int app_host_cmd(const char *cmd,const char *a,const char *b,const char *c,const char *d){(void)b;(void)c;(void)d;
 if(!strcmp(cmd,"projectaudit")){ed_project_audit();return 1;}
 if(!strcmp(cmd,"invariants")){ed_invariants();return 1;}
 /* sysex HEX: one editor frame without F0/F7, straight into the protocol handler. */
 if(!strcmp(cmd,"sysex")){static uint8_t w[4096];uint32_t n=0;const char *p=a;while(p&&p[0]&&p[1]&&n<sizeof w){unsigned v;if(sscanf(p,"%2x",&v)!=1)break;w[n++]=(uint8_t)(v&127);p+=2;}app_sysex(w,n);return 1;}
 if(!strcmp(cmd,"stuckprep")){ed_stuck_prep();return 1;}
 if(!strcmp(cmd,"stuckexpect")){ed_stuck_expect();return 1;}
 if(!strcmp(cmd,"voices")){void z_debug_voices(void);z_debug_voices();return 1;}
 if(!strcmp(cmd,"edgecheck")){
#ifdef APP_TOUCH_EDGE_POLL
 touch_on(1);sense_init(1);touch_held=touch_pending_valid=0;sn_ev_r=sn_ev_w=0;sn_push(EV_ON,0,101,0);sn_push(EV_OFF,0,0,0);touch_poll();ed_check(touch_keys()&1,"fast tap press survives queued release");ed_check(touch_vel(0)==101,"queued tap preserves velocity");touch_poll();ed_check(!(touch_keys()&1),"fast tap release is delivered next poll");sn_push(EV_ON,0,90,0);touch_poll();sn_push(EV_ON,0,110,0);touch_poll();ed_check(!(touch_keys()&1),"same-key retrigger exposes release");touch_poll();ed_check((touch_keys()&1)&&touch_vel(0)==110,"same-key retrigger exposes new attack");touch_held=touch_pending_valid=0;sn_ev_r=sn_ev_w=0;sense_init((1u<<27)-1);for(uint32_t k=0;k<27;k++)sn_push(EV_ON,k,100,0);for(uint32_t k=0;k<27;k++)sn_push(EV_OFF,k,0,0);touch_poll();ed_check(touch_held==((1u<<27)-1),"whole chord press burst is preserved");touch_poll();ed_check(!touch_held,"whole chord release burst is preserved");touch_on(0);touch_on(1);
#endif
 return 1;}
 if(!strcmp(cmd,"recsetup")){app_panic();z_init();panel_reset();z_song.selected=atoi(a);z_song.section=Z_SEQ;z_song.subpage=0;z_song.step_edit=z_song.step_bank=0;z_song.bpm=120;z_song.swing=0;z_song.rec_mode=1;z_song.rec_quant=0;for(uint32_t t=0;t<6;t++)z_erase(t,0,2);midi_local=1;system_set(6,0);redraw_at=0;return 1;}
 if(!strcmp(cmd,"stepcursor")){z_song.step_edit=atoi(a);z_song.step_bank=z_song.step_edit/16;return 1;}
 if(!strcmp(cmd,"editioncheck")){ed_checks();return 1;}
 if(!strcmp(cmd,"splashpreview")){app_splash();return 1;}
 if(!strcmp(cmd,"dmcheck")){if(Z_SAMPLER_EDITION)return 1;app_panic();z_init();z_song.selected=5;z_dm_step(15,63,111,77,4);z_dm_param(15,5,57);z_dm_param(15,2,-7);z_dm_select(15);ed_check(plat_store_save(OBJ_SETTINGS,&z_song,sizeof z_song)==0&&plat_store_save(OBJ_DRUM,&z_dm,sizeof z_dm)==0,"Zombie drum flash save");app_init();ed_check(z_dm.selected==15&&z_dm.sound[15].length==57&&z_dm.sound[15].tune==-7&&z_dm.step[15][63].ratchet==4&&z_dm.step[15][63].velocity==111,"Zombie drum project restored after reboot");ed_check(!z_playing()&&!z_song.recording,"drum project boot is stopped");z_song.selected=5;z_song.section=Z_SEQ;z_song.subpage=0;z_song.step_bank=3;z_song.step_edit=63;redraw_at=0;return 1;}
 if(!strcmp(cmd,"drumtest")){uint32_t t;system_set(6,0);app_panic();z_init();for(t=0;t<6;t++)z_clear_track(t);z_song.selected=5;z_song.section=Z_SEQ;z_song.subpage=0;z_song.step_edit=0;z_step_set(5,0,36,100,127,80);return 1;}
 if(!strcmp(cmd,"rolltest")){ed_roll();return 1;}
 if(!strcmp(cmd,"en")){z_song.language=1;redraw_at=0;return 1;}
 if(!strcmp(cmd,"ru")){z_song.language=0;redraw_at=0;return 1;}
 if(!strcmp(cmd,"home")){z_song.section=Z_HOME;z_song.subpage=0;redraw_at=0;return 1;}
 if(!strcmp(cmd,"track")){z_song.selected=clamp(atoi(a),0,5);redraw_at=0;return 1;}
 if(!strcmp(cmd,"synthtest")){system_set(6,0);z_song.selected=Z_SAMPLER_EDITION?0:3;z_preset(z_song.selected,16);z_set(z_song.selected,Z_MUTE,0);return 1;}
 if(!strcmp(cmd,"midiready")){uint32_t k;system_set(6,2);z_song.selected=0;for(k=0;k<4;k++)daw_values[0][k]=64;daw_values[0][4]=95;daw_values[0][5]=64;return 1;}
 if(!strcmp(cmd,"gatepad")){z_sampler.pad[0].mode=1;return 1;}
 if(!strcmp(cmd,"dawshow")){uint32_t j;const char *s="AFTERGLOW RACK";system_set(6,2);daw_seen=plat_ms();daw_host=2;daw_playing=1;daw_bpm=132;for(j=0;s[j];j++)daw_name[j]=s[j];daw_name[j]=0;return 1;}
 if(!strcmp(cmd,"defaults")){system_set(6,0);app_panic();z_init();panel_reset();midi_local=1;z_song.section=Z_HOME;z_song.subpage=0;redraw_at=0;return 1;}
 return 0;
}
int app_expect(const char *what,int *got){
 if(!strcmp(what,"dirty"))*got=saved_revision!=z_revision()||system_dirty;
 else if(!strcmp(what,"midi_mode"))*got=z_sys.midi_mode;
 else if(!strcmp(what,"model"))*got=z_get(z_song.selected,Z_MODEL);
 else if(!strcmp(what,"voices"))*got=z_voice_count();
 else if(!strcmp(what,"step_edit"))*got=z_song.step_edit;
 else if(!strcmp(what,"note"))*got=z_song.track[z_song.selected].steps[z_song.step_edit].note;
 else if(!strcmp(what,"velocity"))*got=z_song.track[z_song.selected].steps[z_song.step_edit].velocity;
 else if(!strcmp(what,"duration"))*got=z_song.duration[z_song.selected][z_song.step_edit];
 else if(!strcmp(what,"chord1"))*got=z_song.chord[z_song.selected][z_song.step_edit][0];
 else if(!strcmp(what,"sampler_bank"))*got=z_sampler.bank;
 else if(!strcmp(what,"playing"))*got=z_playing();
 else if(!strcmp(what,"recording"))*got=z_song.recording;
 else if(!strcmp(what,"subpage"))*got=z_song.subpage;
 else if(!strcmp(what,"section"))*got=z_song.section;
 else if(!strcmp(what,"bpm"))*got=z_song.bpm;
 else if(!strcmp(what,"daw_bpm"))*got=daw_bpm;
 else if(!strcmp(what,"layer"))*got=active_layer;
 else if(!strcmp(what,"locked"))*got=layer_locked;
 else if(!strcmp(what,"bank"))*got=z_song.step_bank;
 else if(!strcmp(what,"octave"))*got=z_song.octave;
 else if(!strcmp(what,"effect"))*got=z_live_state();
 else if(!strcmp(what,"dm_ratchet"))*got=z_dm.step[z_dm.selected][z_song.step_edit].ratchet;
 else if(!strcmp(what,"dm_sound"))*got=z_dm.selected;
 else if(!strcmp(what,"dm_length"))*got=z_dm.sound[z_dm.selected].length;
 else if(!strcmp(what,"gate"))*got=z_song.track[z_song.selected].steps[z_song.step_edit].gate;
 else if(!strcmp(what,"dm_velocity"))*got=z_dm.step[z_dm.selected][z_song.step_edit].velocity;
 else if(!strcmp(what,"part_hits")){*got=0;if(z_drum_track(z_song.selected)){for(uint32_t i=0;i<64;i++)*got+=z_dm.step[z_dm.selected][i].velocity!=0;}else for(uint32_t i=0;i<64;i++)*got+=z_song.track[z_song.selected].steps[i].velocity!=0;}
 else if(!strcmp(what,"track_hits")){*got=0;for(uint32_t d=0;d<16;d++)for(uint32_t i=0;i<64;i++)*got+=z_dm.step[d][i].velocity!=0;}
 else if(!strcmp(what,"drum_choice"))*got=drum_choice;
 else return 0;return 1;
}

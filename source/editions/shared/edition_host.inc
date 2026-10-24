/* SPDX-License-Identifier: MIT — simulated firmware tests, never real USB. */
static void ed_check(int condition,const char *message){if(!condition){fprintf(stderr,"FAIL: %s\n",message);exit(1);}printf("PASS: %s\n",message);}
static void ed_roll(void){uint32_t t=3;system_set(6,0);app_panic();z_song.selected=t;z_song.language=0;z_song.section=Z_SEQ;z_song.subpage=0;z_song.step_bank=z_song.step_edit=0;z_preset(t,16);z_clear_track(t);z_step_set(t,0,60,100,127,80);z_song.duration[t][0]=4;z_song.chord[t][0][0]=65;z_song.chord[t][0][1]=68;z_step_set(t,6,67,88,127,80);z_song.duration[t][6]=2;z_step_set(t,10,63,80,127,80);redraw_at=0;}
static void ed_checks(void){uint32_t t,p;int old;
 ed_check(z_sys.midi_mode==0&&midi_local,"startup is standalone with local sound");
 ed_check(!z_playing()&&!z_song.recording,"boot never auto-starts playback or recording");
 ed_check(z_sampler.tracks==(Z_SAMPLER_EDITION?63:0),"edition pad capabilities enforced");
 for(t=0;t<6;t++){old=z_get(t,Z_MODEL);z_set(t,Z_MODEL,Z_SAMPLER_EDITION?0:8);ed_check(z_get(t,Z_MODEL)==(Z_SAMPLER_EDITION?8:7),"foreign engine cannot enter this edition");z_set(t,Z_MODEL,old);}
 for(p=0;p<64;p++){z_preset(0,p);ed_check(Z_SAMPLER_EDITION?z_get(0,Z_MODEL)==8:z_get(0,Z_MODEL)<8,"factory preset uses the correct engine family");}
 if(Z_SAMPLER_EDITION){ed_check(z_sample_frames(10)>0&&z_sample_frames(0)>0,"factory drum and zombie PCM available");ed_check(!z_sampler_enable(0,0)&&z_sampler.tracks==63,"dedicated sampler cannot accidentally disable pads");z_song.track[0].p[Z_MODEL]=0;z_song.track[0].p[Z_SUSTAIN]=0;z_song.track[0].p[Z_ATTACK]=127;z_apply_state();ed_check(z_get(0,Z_MODEL)==8&&z_get(0,Z_SUSTAIN)==127&&z_get(0,Z_ATTACK)==0,"legacy synth project cannot mute sampler envelope");}
 else{ed_check(z_sample_frames(0)==0,"groovebox does not carry Minecraft PCM");ed_check(z_sampler_enable(0,1)==-1&&z_sampler.tracks==0,"groovebox cannot enable hidden sampler");}
 system_set(6,2);ed_check(system_save()==0,"DAW settings saved in simulated flash");app_init();ed_check(z_sys.midi_mode==0&&midi_local,"saved controller mode cannot mute startup");
}
#if !Z_SAMPLER_EDITION
static int h_wire(uint8_t cmd,const uint8_t *data,uint32_t n){uint8_t wire[245]={0x7d,0x5a,0x43,1,0};uint32_t at=5;wire[4]=cmd;for(uint32_t i=0;i<n;){uint32_t hi=at++;wire[hi]=0;for(uint32_t k=0;k<7&&i<n;k++,i++){wire[hi]|=(data[i]>>7)<<k;wire[at++]=data[i]&127;}}ed_check(app_sysex(wire,at),"drum SysEx recognized");return raw_reply[0];}
static void h_protocol_check(void){
 z_project before=z_song;uint8_t bytes[3288],payload[194],begin[4];
 ed_check(h_wire(44,(uint8_t[]){0,1,7,0},4)==0,"drum parameter command");
 ed_check(h_wire(44,(uint8_t[]){8,1,7,0},4)!=0,"invalid drum lane rejected");
 ed_check(h_wire(45,(uint8_t[]){0,63,1,1,56,3,12},7)==0,"last step, condition and ratchet command");
 ed_check(h_wire(45,(uint8_t[]){0,64,1,1,56,3,12},7)!=0,"step outside pattern rejected");
 ed_check(h_wire(43,0,0)==0,"independent drum snapshot command");memcpy(bytes,raw_reply+1,3288);
 wr32(begin,crc32(bytes,3288));ed_check(h_wire(47,begin,4)==0,"begin drum project transfer");
 ed_check(h_wire(48,0,0)!=0,"incomplete drum project transfer rejected");
 ed_check(h_wire(47,begin,4)==0,"restart drum transfer");
 for(uint32_t i=0;i<3288;i+=192){uint32_t n=3288-i;if(n>192)n=192;payload[0]=(uint8_t)i;payload[1]=(uint8_t)(i>>8);memcpy(payload+2,bytes+i,n);ed_check(h_wire(17,payload,n+2)==0,"drum transfer chunk");}
 ed_check(h_wire(48,0,0)==0,"validated drum project committed");
 ed_check(!memcmp(before.track,z_song.track,5*sizeof(z_song.track[0])),"drum commands preserve five synth tracks");
}
#endif
int app_host_cmd(const char *cmd,const char *a,const char *b,const char *c,const char *d){(void)b;(void)c;(void)d;
 #if !Z_SAMPLER_EDITION
 if(!strcmp(cmd,"hortatorcheck")){h_protocol_check();int rc=h_host_check();printf("Hortator check stage: %d\n",rc);ed_check(rc==0,"ratchets, probability, Grids, motion, CPU caps, user presets, four projects and song");return 1;}
#endif
 if(!strcmp(cmd,"editioncheck")){ed_checks();return 1;}
 if(!strcmp(cmd,"splashpreview")){app_splash();return 1;}
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
 else if(!strcmp(what,"drum_choice"))*got=drum_choice;
 else return 0;return 1;
}

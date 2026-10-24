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
int app_host_cmd(const char *cmd,const char *a,const char *b,const char *c,const char *d){(void)b;(void)c;(void)d;
 if(!strcmp(cmd,"editioncheck")){ed_checks();return 1;}
 if(!strcmp(cmd,"rolltest")){ed_roll();return 1;}
 if(!strcmp(cmd,"en")){z_song.language=1;redraw_at=0;return 1;}
 if(!strcmp(cmd,"ru")){z_song.language=0;redraw_at=0;return 1;}
 if(!strcmp(cmd,"home")){z_song.section=Z_HOME;z_song.subpage=0;redraw_at=0;return 1;}
 if(!strcmp(cmd,"track")){z_song.selected=clamp(atoi(a),0,5);redraw_at=0;return 1;}
 if(!strcmp(cmd,"synthtest")){system_set(6,0);z_song.selected=Z_SAMPLER_EDITION?0:3;z_preset(z_song.selected,16);z_set(z_song.selected,Z_MUTE,0);return 1;}
 if(!strcmp(cmd,"midiready")){uint32_t k;system_set(6,2);z_song.selected=0;for(k=0;k<4;k++)daw_values[0][k]=64;daw_values[0][4]=95;daw_values[0][5]=64;return 1;}
 if(!strcmp(cmd,"gatepad")){z_sampler.pad[0].mode=1;return 1;}
 if(!strcmp(cmd,"dawshow")){uint32_t j;const char *s="AFTERGLOW RACK";system_set(6,2);daw_seen=plat_ms();daw_host=2;daw_playing=1;daw_bpm=132;for(j=0;s[j];j++)daw_name[j]=s[j];daw_name[j]=0;return 1;}
 if(!strcmp(cmd,"defaults")){system_set(6,0);app_panic();z_init();midi_local=1;z_song.section=Z_HOME;z_song.subpage=0;redraw_at=0;return 1;}
 return 0;
}
int app_expect(const char *what,int *got){
 if(!strcmp(what,"midi_mode"))*got=z_sys.midi_mode;
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
 else return 0;return 1;
}

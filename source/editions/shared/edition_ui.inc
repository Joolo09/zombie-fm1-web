/* SPDX-License-Identifier: MIT — original 240x240 Afterglow device UI.
 * Animation is display-only: no sleeps, MIDI sends, audio writes or heap use. */
#if Z_SAMPLER_EDITION
#define UI_BG RGB(8,14,24)
#define UI_CARD RGB(18,30,46)
#define UI_GRID RGB(30,45,63)
#define UI_TEXT RGB(231,242,246)
#define UI_MUTED RGB(133,158,178)
#define UI_PAST RGB(34,22,30)
#else
#define UI_BG RGB(10,17,14)
#define UI_CARD RGB(24,36,27)
#define UI_GRID RGB(45,65,41)
#define UI_TEXT RGB(226,239,206)
#define UI_MUTED RGB(145,168,124)
#define UI_PAST RGB(38,22,26) /* steps after the loop end */
#endif
#undef C_BLACK
#define C_BLACK UI_BG
static const uint16_t ui_colors[6]={RGB(255,154,91),RGB(242,106,144),RGB(243,210,112),RGB(112,235,198),RGB(123,180,255),RGB(190,144,249)};
static uint32_t ui_view_key=0xffffffffu,ui_view_since,ui_pad_until[32],ui_wave_at;
static uint8_t ui_meter,ui_wave[96],ui_wave_source=255;
static float ui_sin(float t);
static uint32_t ui_boot_at,ui_input_at,ui_knob_at[4],ui_lit[6];
static uint8_t ui_vu[6];
static int ui_zhead,ui_zmouth,ui_zwalk,ui_zplay;
typedef struct{uint32_t born;int16_t x;uint8_t color,kind;}ui_spark_t;
static ui_spark_t ui_sparks[12];static uint32_t ui_spark_next;
static uint8_t ui_about,ui_wake_src;static uint32_t ui_about_at,ui_home_tap_at,ui_enc_at[7];static int8_t ui_enc_dir[7];
/* what counts as a touch: src 1 a key, 2 a button, 3.. an encoder (EN_* + 3) */
static void ui_input_from(uint32_t src){uint32_t now=plat_ms();ui_input_at=now;ui_wake_src=(uint8_t)src;if(ui_about&&now-ui_about_at>300)ui_about=0;}
/* one lone detent is not a touch (a resting encoder can tick by itself on the device); two, or a second the same way soon after, are */
static void ui_enc(uint32_t role,int d){uint32_t now=plat_ms();int s=d>0?1:-1;if(role<7){if(d>=2||d<=-2||(ui_enc_dir[role]==s&&now-ui_enc_at[role]<700))ui_input_from(3+role);ui_enc_dir[role]=(int8_t)s;ui_enc_at[role]=now;}}
static void ui_knob_touch(uint32_t k,int d){ui_enc(EN_K1+k,d);if(k<4)ui_knob_at[k]=plat_ms()|1;}
static uint32_t ui_knob_glow(uint32_t k){uint32_t age=plat_ms()-ui_knob_at[k&3];return ui_knob_at[k&3]&&age<500?(500-age)*100/500:0;}
/* new voices (a key, a step, MIDI) light their track's meter and send a note flying */
static void ui_scene_tick(uint32_t now){uint32_t t;
 if(!ui_boot_at)ui_boot_at=now|1;if(!ui_input_at)ui_input_at=now|1;
 for(t=0;t<6;t++){uint32_t lit=z_note_lights(t,24);
  if(lit&~ui_lit[t]){ui_spark_t *s=&ui_sparks[ui_spark_next++%12];s->born=now|1;s->x=(int16_t)((ui_spark_next*17+t*11)%44);s->color=(uint8_t)t;s->kind=(uint8_t)(ui_spark_next&1);ui_vu[t]=127;}
  else if(lit&&ui_vu[t]<70)ui_vu[t]=70;else ui_vu[t]=(uint8_t)(ui_vu[t]*3/4);ui_lit[t]=lit;}
 /* drum hits are too short to catch between frames: read them off the grid as each step passes */
 {static uint32_t last=0xffffffffu;uint32_t at=z_dm_position(0);if(z_playing()&&at!=last&&!z_get(5,Z_MUTE)){uint32_t d,hit=0;for(d=0;d<16;d++)if(!z_dm.sound[d].mute&&z_dm.step[d][z_dm_position(d)].velocity)hit=1;if(hit){if(ui_vu[5]<100){ui_spark_t *s=&ui_sparks[ui_spark_next++%12];s->born=now|1;s->x=(int16_t)((ui_spark_next*29)%44);s->color=5;s->kind=(uint8_t)(ui_spark_next&1);}ui_vu[5]=127;}}last=z_playing()?at:0xffffffffu;}
 ui_zplay=z_playing();if(ui_zplay){uint32_t ph=z_position()%4;ui_zhead=ph==0?2:ph==1?1:0;ui_zwalk=(int)((z_position()>>1)&1);}else{ui_zhead=0;ui_zwalk=0;}
 {uint32_t m=0;for(t=0;t<6;t++)if(ui_vu[t]>m)m=ui_vu[t];m=m/40;if(ui_meter/30>m)m=ui_meter/30;ui_zmouth=(int)(m>3?3:m);}
}
static uint16_t ui_accent(void){return ui_colors[z_song.selected];}
static uint16_t ui_blend(uint16_t a,uint16_t b,uint32_t amount){uint32_t r=((a>>11)*(127-amount)+(b>>11)*amount)/127,g=(((a>>5)&63)*(127-amount)+((b>>5)&63)*amount)/127,bl=((a&31)*(127-amount)+(b&31)*amount)/127;return (r<<11)|(g<<5)|bl;}
static void ui_round(int x,int y,int w,int h,uint16_t c,uint16_t bg){cv_rect(x,y,w,h,c);cv_rect(x,y,1,1,bg);cv_rect(x+w-1,y,1,1,bg);cv_rect(x,y+h-1,1,1,bg);cv_rect(x+w-1,y+h-1,1,1,bg);}
static void ui_icon(int x,int y,uint16_t c){uint32_t i;
 if(z_countin_state()||z_song.recording){static const uint8_t w[9]={3,7,9,9,9,9,9,7,3};for(i=0;i<9;i++)cv_rect(x+(9-w[i])/2,y+i,w[i],1,RGB(255,111,145));}
 else if(z_playing()){uint16_t g=z_position()%4==0?RGB(214,255,200):RGB(126,224,140);for(i=0;i<5;i++)cv_rect(x+1+i,y+i,1,9-2*i,g);}
 else cv_rect(x+1,y+1,7,7,c);}
static void edition_note(uint32_t track,uint32_t note,uint32_t velocity,uint32_t on){if(z_drum_track(track)&&on&&velocity){z_dm_select(z_drum_index(note));drum_choice=(uint8_t)z_drum_note(z_dm.selected);}if(Z_SAMPLER_EDITION&&on&&velocity&&note>=36&&note<68)ui_pad_until[note-36]=plat_ms()+180;}
static void ui_prepare(void){uint32_t key=z_song.section|(z_song.subpage<<4)|((active_layer+1)<<8)|(z_song.selected<<16)|(z_sys.midi_mode<<20),now=plat_ms();
 pal[0]=UI_GRID;pal[1]=RGB(62,82,103);pal[2]=UI_MUTED;pal[3]=ui_accent();pal[4]=Z_SAMPLER_EDITION?RGB(191,153,255):ui_accent();
 if(key!=ui_view_key){ui_view_key=key;ui_view_since=now;}
 ui_meter=output_peak>ui_meter?output_peak:ui_meter*3/4;ui_scene_tick(now);
 if(Z_SAMPLER_EDITION&&z_playing()){uint32_t t=z_song.selected,at=z_position()%z_get(t,Z_LENGTH);z_step *s=&z_song.track[t].steps[at];if(s->velocity&&s->note>=36&&s->note<68)ui_pad_until[s->note-36]=now+65;}
}
static void ui_header(const char *title,const char *subtitle){char b[16];uint16_t color=C_HI;
 uint32_t count=z_countin_state();uint16_t rec=RGB(255,111,145);
 if(count)subtitle=word("ОТСЧЕТ: ИГРАЙ С ЕДИНИЦЫ","COUNT-IN: PLAY ON ONE");else if(save_after_stop)subtitle=word("SAVE: ПОСЛЕ STOP","SAVE: WHEN STOPPED");
 cv_begin(240,34,UI_BG);cv_rect(0,0,240,2,count||z_song.recording?rec:color);ui_icon(6,6,UI_MUTED);z_text_clip(20,5,&FONT_XS,title,UI_TEXT,138);z_text_clip(20,19,&FONT_XS,subtitle,count||z_song.recording?rec:UI_MUTED,138);
 if(saved_revision!=z_revision()||system_dirty)cv_text(162,5,&FONT_XS,"*",color);
 if(count){number((int)count,b);cv_text(201-text_w(&FONT_M,b)/2,1,&FONT_M,b,rec);cv_text(201-text_w(&FONT_XS,"REC")/2,23,&FONT_XS,"REC",rec);}
 else{number(z_sys.midi_mode==2?daw_bpm:z_song.bpm,b);cv_text(201-text_w(&FONT_M,b)/2,1,&FONT_M,b,UI_TEXT);cv_text(201-text_w(&FONT_XS,"BPM")/2,23,&FONT_XS,"BPM",UI_MUTED);}cv_blit(0,0);
}
static void ui_tracks(void){uint32_t i;char b[4];cv_begin(240,24,UI_BG);
 for(i=0;i<6;i++){int x=6+i*38,sel=i==z_song.selected,on=z_note_lights(i,24)!=0;uint16_t c=ui_colors[i],fill=sel?c:on?ui_blend(UI_CARD,c,50):UI_CARD;
  ui_round(x,3,34,18,fill,UI_BG);if(!sel)cv_rect(x+2,3,30,2,c);number(i+1,b);cv_text(x+5,6,&FONT_XS,b,sel?UI_BG:UI_TEXT);
  if(z_get(i,Z_MUTE))cv_text(x+20,6,&FONT_XS,"M",sel?UI_BG:UI_MUTED);else if(z_solo_state()==i+1)cv_text(x+20,6,&FONT_XS,"S",sel?UI_BG:c);else if(z_track_hits(i))cv_rect(x+24,10,4,4,sel?UI_BG:c);
  if(ui_vu[i]>8)cv_rect(x+2,21,ui_vu[i]*30/127,2,c);}cv_blit(0,34);
}
static void ui_footer(const char *hint){cv_begin(240,18,UI_BG);if((int32_t)(message_until-plat_ms())>0)z_text_clip(7,2,&FONT_XS,message,UI_TEXT,226);else z_text_clip(7,2,&FONT_XS,hint,UI_MUTED,226);cv_blit(0,222);}
static void ui_transition(void){uint32_t age=plat_ms()-ui_view_since;
 cv_begin(240,2,UI_GRID);if(age<220)cv_rect(0,0,(220-age)*240/220,2,C_HI);else if(z_playing())cv_rect((z_position()%16)*15,0,15,2,C_HI);cv_blit(0,238);
}
static void ui_note_name(uint32_t note,char *b){static const char *const names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};const char *p=names[note%12];uint32_t n=0;while(*p)b[n++]=*p++;number((int)note/12-1,b+n);}
static void ui_cell(int x,int y,const char *name,const char *value,int amount){ /* y: the knob, 0..3 (its card glows when turned) */
 uint32_t age=plat_ms()-ui_view_since,offset=age<220?(220-age)*4/220:0;uint16_t a=ui_accent();ui_round(x,0,110,40,ui_blend(UI_CARD,a,ui_knob_glow((uint32_t)y)*45/100),UI_BG);
 z_text_clip(x+7,4,&FONT_XS,name,UI_MUTED,98);z_text_clip(x+7,17+offset,&FONT_S,value,a,98);
 ui_round(x+7,35,96,3,UI_GRID,UI_CARD);if(amount>0){int w=clamp(amount,0,127)*96/127;cv_rect(x+7,35,w,3,a);cv_rect(x+6+w,34,2,5,UI_TEXT);}(void)y;
}
/* 0.18: a living scene. Display only: these read the song and the voices, never change sound, MIDI or saved state. */
static void ui_disc(int cx,int cy,int r,uint16_t c){int dy,w;for(dy=-r;dy<=r;dy++){w=0;while((w+1)*(w+1)+dy*dy<=r*r)w++;cv_rect(cx-w,cy+dy,2*w+1,1,c);}}
static void ui_moon(int cx,int cy,int r,uint16_t bg){
 ui_disc(cx,cy,r+3,ui_blend(bg,RGB(230,236,190),10));ui_disc(cx,cy,r+1,ui_blend(bg,RGB(230,236,190),26));ui_disc(cx,cy,r,RGB(226,232,186));
 ui_disc(cx-r/3,cy-r/4,r/4,RGB(196,204,160));ui_disc(cx+r/3,cy+r/3,r/5,RGB(200,208,164));ui_disc(cx+r/5,cy-r/2,r/7+1,RGB(204,210,168));}
static void ui_stars(int x0,int y0,int w,int h,int n,uint32_t now,int yoff){uint32_t i,r=0x2545f491u;
 for(i=0;i<(uint32_t)n;i++){r^=r<<13;r^=r>>17;r^=r<<5;int x=x0+(int)(r%(uint32_t)w),y=y0+(int)((r>>12)%(uint32_t)h);uint32_t tw=(now/(240+i*23)+i)%7;
  uint16_t c=tw==0?RGB(250,250,220):tw<3?RGB(140,160,140):RGB(80,98,86);cv_rect(x,y-yoff,1,1,c);if(tw==0){cv_rect(x-1,y-yoff,3,1,RGB(150,160,130));cv_rect(x,y-1-yoff,1,3,RGB(150,160,130));cv_rect(x,y-yoff,1,1,RGB(255,255,235));}}}
static void ui_bat(int x,int y,uint32_t now,uint16_t c){if((now/120)&1){cv_rect(x,y,1,2,c);cv_rect(x+6,y,1,2,c);cv_rect(x+1,y+1,5,1,c);cv_rect(x+2,y+2,3,1,c);cv_rect(x+3,y,1,1,c);}
 else{cv_rect(x+3,y,1,1,c);cv_rect(x+1,y+1,5,1,c);cv_rect(x,y+2,2,1,c);cv_rect(x+5,y+2,2,1,c);cv_rect(x+2,y+2,3,1,c);}}
/* a music note, 5 x 7, kind 1 the beamed pair */
static void ui_note_glyph(int x,int y,int kind,uint16_t c){
 if(kind){cv_rect(x,y+5,3,2,c);cv_rect(x+5,y+4,3,2,c);cv_rect(x+2,y,1,6,c);cv_rect(x+7,y-1,1,6,c);cv_rect(x+2,y,6,1,c);cv_rect(x+2,y-1,6,1,c);}
 else{cv_rect(x,y+5,3,2,c);cv_rect(x+2,y,1,6,c);cv_rect(x+3,y,1,1,c);cv_rect(x+4,y+1,1,2,c);}}
static void ui_draw_sparks(int ox,int oy,int rise,uint32_t now,uint16_t bg){uint32_t i;
 for(i=0;i<12;i++){ui_spark_t *s=&ui_sparks[i];uint32_t age=now-s->born;if(!s->born||age>1100)continue;
  int x=ox+s->x/2+(int)(ui_sin(age/700.0f+s->x/10.0f)*4),y=oy-(int)(age*rise/1100);

  ui_note_glyph(x,y,s->kind,ui_blend(ui_colors[s->color],bg,age*127/1100));}}
/* Original block zombie: procedural pixels; it breathes and blinks, and nods to the beat while the song plays. */
static int ui_zscale=1,ui_zox,ui_zoy;
static void ui_zrect(int x,int y,int w,int h,uint16_t c){cv_rect(ui_zox+x*ui_zscale,ui_zoy+y*ui_zscale,w*ui_zscale,h*ui_zscale,c);}
static void ui_zombie(uint32_t now){
 int walk=ui_zwalk,bob=ui_zplay?0:((now/700)&1),x=12,y=1+bob,h=y+ui_zhead;
 uint16_t dark=RGB(12,26,17),skin=RGB(100,154,58),shade=RGB(60,108,44),light=RGB(147,186,78),shirt=RGB(31,119,111),pants=RGB(66,59,105);
 ui_zrect(x+9,52,29,3,RGB(18,28,17));
 ui_zrect(x+9,y+29,22,16,dark);ui_zrect(x+11,y+29,18,14,shirt);ui_zrect(x+12,y+31,4,11,RGB(35,139,126));ui_zrect(x+24,y+33,4,9,RGB(24,87,84));
 if(ui_zplay&&walk){ui_zrect(x+2,y+16,8,17,shade);ui_zrect(x+2,y+14,8,4,skin);}else{ui_zrect(x+2,y+28+walk,8,17,shade);ui_zrect(x+2,y+40+walk,8,4,skin);}
 if(ui_zplay&&!walk){ui_zrect(x+30,y+16,8,17,skin);ui_zrect(x+30,y+14,8,4,light);}else{ui_zrect(x+30,y+28-walk,8,17,skin);ui_zrect(x+30,y+40-walk,8,4,light);}
 ui_zrect(x+11,y+43,8,9+walk,pants);ui_zrect(x+21,y+43,8,10-walk,pants);ui_zrect(x+10,y+51+walk,10,3,dark);ui_zrect(x+20,y+52-walk,10,3,dark);
 ui_zrect(x+5,h,30,29,dark);ui_zrect(x+7,h+2,26,25,skin);ui_zrect(x+7,h+2,26,5,shade);ui_zrect(x+9,h+3,5,3,light);ui_zrect(x+22,h+4,8,3,RGB(72,127,46));
 ui_zrect(x+7,h+10,4,8,shade);ui_zrect(x+29,h+8,4,17,shade);ui_zrect(x+11,h+21,5,5,light);ui_zrect(x+22,h+22,6,4,shade);ui_zrect(x+17,h+15,5,6,shade);
 int blink=(now/100)%45==0;ui_zrect(x+11,h+11,7,blink?2:5,dark);ui_zrect(x+23,h+11,7,blink?2:5,dark);
 if(!blink){uint16_t eye=z_song.recording||z_countin_state()?RGB(255,105,89):ui_zplay?ui_colors[z_song.selected]:RGB(203,224,148);ui_zrect(x+12,h+12,3,2,eye);ui_zrect(x+24,h+12,3,2,eye);if(ui_zplay){ui_zrect(x+15,h+12,1,1,ui_blend(eye,dark,60));ui_zrect(x+27,h+12,1,1,ui_blend(eye,dark,60));}}
 if(ui_zmouth){ui_zrect(x+15,h+22,10,2+ui_zmouth,dark);ui_zrect(x+17,h+23,6,ui_zmouth,RGB(122,34,44));}else{ui_zrect(x+15,h+23,10,2,dark);ui_zrect(x+19,h+25,7,1,shade);}
}
/* idle for a minute (no key, button or knob), outside a layer and not recording: the night scene, until any touch */
#define UI_SAVER_MS 60000u
/* 0: nothing holds it back; else why not (shown on the About screen) */
static uint32_t ui_saver_block(void){uint32_t now=plat_ms();if(!saver_on())return 1;if(active_layer>=0)return 2;if(z_song.recording||z_countin_state())return 3;if(grid_hold.key>=0)return 4;if(z_sys.midi_mode==2)return 5;if((int32_t)(message_until-now)>0)return 6;return 0;}
static uint32_t ui_saver_active(void){return ui_input_at&&plat_ms()-ui_input_at>UI_SAVER_MS&&!ui_saver_block();}
/* the keys while it sleeps: a slow wave across the white keys (glow on, stopped) */
static uint32_t ui_saver_keys(uint32_t now){static const uint8_t white[16]=PLAT_WHITE_KEYS;uint32_t m=0,p=(now/140)%30,j;
 for(j=0;j<16;j++){int d=(int)j-(int)(p<16?p:30-p);if(d>=-1&&d<=1)m|=1u<<white[j];}return m;}
static void ui_saver_band(int yoff,uint32_t now){uint32_t i,t;int play=ui_zplay;char b[12];
 static const uint16_t sky[8]={RGB(6,9,20),RGB(8,12,24),RGB(9,15,27),RGB(10,18,29),RGB(11,21,30),RGB(12,24,30),RGB(13,26,28),RGB(14,28,26)};
 for(i=0;i<8;i++)cv_rect(0,(int)i*24-yoff,240,24,sky[i]);
 ui_stars(0,4,240,150,34,now,yoff);
 ui_moon(186,52-yoff,24,sky[2]);
 for(i=0;i<2;i++){int x=(int)((now/(28+i*9)+i*130)%300)-30,y=40+(int)i*26+(int)(ui_sin(now/900.0f+i)*10);ui_bat(x,y-yoff,now+i*60,RGB(30,40,44));}
 /* hills, three graves, the ground */
 for(i=0;i<240;i+=4){int hh=(int)(14+ui_sin(i/170.0f+0.2f)*8+ui_sin(i/60.0f)*3);cv_rect((int)i,196-hh-yoff,4,hh+4,RGB(16,30,24));}
 {static const int gx[3]={18,150,206};for(i=0;i<3;i++){int x=gx[i],y=170+(int)(i&1)*6;cv_rect(x+2,y-yoff,18,30,RGB(78,88,96));cv_rect(x,y+4-yoff,22,26,RGB(78,88,96));cv_rect(x+4,y-2-yoff,14,2,RGB(78,88,96));cv_rect(x+3,y+2-yoff,16,1,RGB(108,118,124));cv_text(x+3,y+8-yoff,&FONT_XS,"RIP",RGB(40,48,54));}}
 cv_rect(0,200-yoff,240,40,RGB(40,30,22));cv_rect(0,198-yoff,240,4,RGB(70,112,46));for(i=0;i<240;i+=13){cv_rect((int)i+3,208+(int)(i%3)*7-yoff,3,2,RGB(60,46,32));cv_rect((int)i,197-yoff,2,2,RGB(96,140,58));}
 /* the six tracks as glowing candles while the song plays */
 if(play)for(t=0;t<6;t++){int x=160+(int)t*12,hgt=4+ui_vu[t]*40/127;uint16_t c=ui_colors[t];cv_rect(x,198-hgt-yoff,8,hgt,ui_blend(RGB(14,28,26),c,40+ui_vu[t]*80/127));cv_rect(x,198-hgt-2-yoff,8,2,c);}
 ui_zscale=2;ui_zox=24;ui_zoy=90-yoff;ui_zombie(now);
 if(play)ui_draw_sparks(128,128-yoff,90,now,sky[3]);
 else for(i=0;i<3;i++){uint32_t age=(now+i*900)%2700;int x=112+(int)i*7+(int)(ui_sin(age/1300.0f)*4),y=100-(int)(age*40/2700)-(int)i*4;cv_text(x,y-yoff,&FONT_XS,i==2?"Z":"z",ui_blend(RGB(200,220,170),sky[2],age*120/2700));}
 if(yoff==0){cv_text(10,8,&FONT_XS,"ZOMBIE FM-1",RGB(150,190,120));
  if(play){number(z_song.bpm,b);cv_text(10,22,&FONT_M,b,RGB(226,239,206));cv_text(12+text_w(&FONT_M,b),36,&FONT_XS,"BPM",RGB(145,168,124));
   for(i=0;i<16;i++)cv_rect(10+(int)i*5,50,4,4,i==z_position()%16?RGB(255,255,235):i%4?RGB(40,60,50):RGB(80,110,80));}
  else z_text(10,22,&FONT_XS,(now/4000)&1?word("ДЕРЖИ HOME: ДРУГАЯ ЗАСТАВКА","HOLD HOME: NEXT SAVER"):word("ЛЮБАЯ КНОПКА: ПРОСНУТЬСЯ","ANY KEY: WAKE UP"),RGB(110,130,100));}
}
/* 0.18.1: four screensavers (hold HOME on one for the next), and the About screen (HOME twice). */
static uint32_t ui_saver_label_at;
static void ui_saver_next(void){system_set(14,(saver_style()+1)%4);ui_saver_label_at=plat_ms()|1;}
/* HOME on a screensaver belongs to it: a tap wakes, a hold turns to the next one (again every 0.6 s); 1 = HOME is taken */
static uint8_t ui_saver_hold;static uint32_t ui_saver_hold_at;
static int ui_saver_home(uint32_t buttons,uint32_t held,uint32_t now){int down=(int)(buttons>>B_HOME&1);
 if(ui_saver_hold){if(!down){if(ui_saver_hold==1)ui_input_from(2);ui_saver_hold=0;return 1;}if(now-ui_saver_hold_at>=600){ui_saver_next();ui_saver_hold_at=now;ui_saver_hold=2;}return 1;}
 if(down&&!(held>>B_HOME&1)&&ui_saver_active()){ui_saver_hold=1;ui_saver_hold_at=now;return 1;}
 return 0;}
static void ui_home_tap(uint32_t now){if(ui_home_tap_at&&now-ui_home_tap_at<450){ui_about=1;ui_about_at=now|1;ui_home_tap_at=0;}else ui_home_tap_at=now|1;}
static void ui_saver_top(uint32_t now,const char *title){char b[12];cv_text(10,8,&FONT_XS,title,RGB(150,190,120));
 if(z_playing()){uint32_t i;number(z_song.bpm,b);cv_text(10,22,&FONT_M,b,RGB(236,242,220));cv_text(12+text_w(&FONT_M,b),36,&FONT_XS,"BPM",RGB(160,170,150));for(i=0;i<16;i++)cv_rect(10+(int)i*5,50,4,4,i==z_position()%16?RGB(255,255,235):i%4?RGB(50,60,60):RGB(110,120,110));}(void)now;}
static void ui_saver_label(uint32_t now,int yoff){uint32_t age=now-ui_saver_label_at;char t[48];uint32_t k=0;const char *p;
 if(!ui_saver_label_at||age>1600)return;p=word("ЗАСТАВКА: ","SCREENSAVER: ");while(*p)t[k++]=*p++;p=z_song.language?ui_saver_en[saver_style()]:ui_saver_ru[saver_style()];while(*p&&k<40)t[k++]=*p++;t[k]=0;
 ui_round(30,92-yoff,180,42,RGB(20,26,24),RGB(0,0,0));cv_rect(30,92-yoff,180,2,ui_colors[saver_style()]);z_text(40,98-yoff,&FONT_XS,t,RGB(236,242,220));z_text(40,115-yoff,&FONT_XS,word("SAVE: ЗАПОМНИТЬ","SAVE: KEEP IT"),RGB(140,150,140));}
static void ui_synth_band(int yoff,uint32_t now){int i,dy,r=44,play=ui_zplay;
 static const uint16_t sky[10]={RGB(16,6,38),RGB(26,8,52),RGB(40,10,66),RGB(58,12,80),RGB(80,16,90),RGB(108,20,98),RGB(138,28,98),RGB(166,40,92),RGB(194,58,84),RGB(220,80,74)};
 for(i=0;i<10;i++)cv_rect(0,i*13-yoff,240,13,sky[i]);ui_stars(0,2,240,64,22,now,yoff);
 for(dy=-r;dy<=r;dy++){int w=0,y=96+dy;while((w+1)*(w+1)+dy*dy<=r*r)w++;if(dy>4&&((dy+(int)(now/70))%9)<1+dy/12)continue;cv_rect(120-w,y-yoff,2*w+1,1,ui_blend(RGB(255,224,96),RGB(255,64,150),(uint32_t)(dy+r)*127/(2*r)));}
 for(i=0;i<240;i+=3){int h=(int)(16+ui_sin(i/110.0f)*9+ui_sin(i/41.0f+0.3f)*5);cv_rect(i,130-h-yoff,3,h,RGB(34,8,58));cv_rect(i,130-h-yoff,3,1,RGB(255,92,210));}
 if(play)for(i=0;i<6;i++){int h=ui_vu[i]*34/127;cv_rect(84+i*12,129-h-yoff,8,h,ui_colors[i]);}
 cv_rect(0,130-yoff,240,110,RGB(12,3,26));cv_rect(0,130-yoff,240,1,RGB(255,140,230));
 {float ph=play?(float)(z_position()%4)/4.0f:(float)(now%1600)/1600.0f;for(i=0;i<12;i++){float z=(float)i+1.0f-ph;if(z<0.35f)continue;int y=130+(int)(100.0f/z);if(y>239)continue;cv_rect(0,y-yoff,240,1,ui_blend(RGB(60,10,80),RGB(255,70,200),(uint32_t)(127-(int)(z*10>127?127:z*10))));}}
 for(i=-9;i<=9;i++)cv_line(120+i*4,130-yoff,120+i*44,239-yoff,RGB(200,50,180));
 ui_zscale=1;ui_zox=88;ui_zoy=152-yoff;ui_zombie(now);if(play)ui_draw_sparks(136,150-yoff,70,now,RGB(12,3,26));
 if(!yoff)ui_saver_top(now,"ZOMBIE FM-1");}
static void ui_matrix_band(int yoff,uint32_t now){static const char set[]="ZOMBIE0123456789#$%&*+=<>FM";int c,i;char ch[2]={0,0};
 for(c=0;c<20;c++){uint32_t h=(uint32_t)c*2654435761u,speed=36+(h>>8)%64;if(ui_zplay)speed=speed*3/2;int head=(int)((now*speed/1000+(h>>16))%380)-60;uint16_t lead=ui_zplay&&ui_vu[c%6]>60?ui_colors[c%6]:RGB(214,255,214);
  for(i=0;i<14;i++){int y=head-i*12;if(y-yoff<-12||y-yoff>=120)continue;ch[0]=set[((h>>4)+(uint32_t)i*7+now/(140+(uint32_t)(c%5)*50))%(sizeof set-1)];cv_text(c*12+2,y-yoff,&FONT_XS,ch,i==0?lead:ui_blend(RGB(60,230,90),RGB(0,0,0),(uint32_t)i*9));}}
 if(yoff<150&&yoff+120>90){static const char *const msg[2]={"WAKE UP, ZOMBIE...","FOLLOW THE BEAT"};const char *m=msg[(now/9000)&1];uint32_t n=(now%9000)/110,len=0;char t[24];while(m[len])len++;if(n>len)n=len;for(i=0;i<(int)n;i++)t[i]=m[i];t[n]=0;
  ui_round(30,94-yoff,180,48,RGB(0,0,0),RGB(0,0,0));cv_rect(30,94-yoff,180,1,RGB(60,230,90));cv_rect(30,141-yoff,180,1,RGB(60,230,90));cv_text(42,104-yoff,&FONT_XS,t,RGB(120,255,140));if((now/400)&1)cv_rect(42+text_w(&FONT_XS,t)+1,104-yoff,6,10,RGB(120,255,140));
  if(ui_zplay){char b[8];number(z_song.bpm,b);cv_text(42,122-yoff,&FONT_XS,b,RGB(60,230,90));cv_text(42+text_w(&FONT_XS,b)+4,122-yoff,&FONT_XS,"BPM",RGB(40,150,60));}}}
static void ui_bounce_band(int yoff,uint32_t now){const int w=112,h=50;uint32_t tx=now*46/1000,ty=now*33/1000,rx=240-w,ry=240-h,bx=tx%(2*rx),by=ty%(2*ry);int x=(int)(bx<rx?bx:2*rx-bx),y=(int)(by<ry?by:2*ry-by);uint32_t hits=tx/rx+ty/ry;uint16_t c=ui_colors[hits%6];
 ui_stars(0,0,240,240,26,now,yoff);
 if((x<4||x>(int)rx-4)&&(y<4||y>(int)ry-4)&&!yoff)z_text(80,112,&FONT_S,word("УГОЛ!","CORNER!"),RGB(255,255,200));
 if(ui_zplay&&z_position()%4==0)ui_round(x-3,y-3-yoff,w+6,h+6,ui_blend(c,RGB(0,0,0),70),RGB(4,4,10));
 ui_round(x,y-yoff,w,h,c,RGB(4,4,10));cv_text(x+10,y+6-yoff,&FONT_S,"ZOMBIE",RGB(8,10,14));cv_text(x+10,y+30-yoff,&FONT_XS,"FM-1  GROOVEBOX",ui_blend(c,RGB(0,0,0),80));
 if(!yoff&&ui_zplay)ui_saver_top(now,"");}
static void ui_draw_saver(void){uint32_t now=plat_ms(),s=saver_style();int b;static const uint16_t bg[4]={RGB(6,9,20),RGB(16,6,38),RGB(0,0,0),RGB(4,4,10)};
 for(b=0;b<2;b++){cv_begin(240,120,bg[s]);if(s==1)ui_synth_band(b*120,now);else if(s==2)ui_matrix_band(b*120,now);else if(s==3)ui_bounce_band(b*120,now);else ui_saver_band(b*120,now);ui_saver_label(now,b*120);cv_blit(0,(uint32_t)b*120);}}
/* About: a live picture of the device instead of a page of text */
static void ui_card(int x,int y,int w,int h,const char *title,uint16_t c){ui_round(x,y,w,h,UI_CARD,UI_BG);cv_rect(x+2,y,w-4,2,c);z_text_clip(x+6,y+4,&FONT_XS,title,UI_MUTED,(uint32_t)w-10);}
static void ui_big(int x,int y,const char *v,uint16_t c){cv_text(x,y,&FONT_S,v,c);}
static void ui_draw_about(void){uint32_t now=plat_ms(),t,notes=0,up=now/1000,cpu=plat_cpu_pct(),idle=(now-ui_input_at)/1000,why=ui_saver_block();char b[40],v[24];int i;
 for(t=0;t<6;t++)notes+=z_track_hits(t);
 /* top: a wave of letters, the version, the owner, the zombie */
 cv_begin(240,120,UI_BG);{static const char title[]="ZOMBIE FM-1";int x=10;for(i=0;title[i];i++){char ch[2]={title[i],0};int dy=(int)(ui_sin(now/900.0f+i*0.09f)*3);cv_text(x,8+dy,&FONT_S,ch,title[i]==' '?UI_BG:ui_colors[(i+now/400)%6]);x+=text_w(&FONT_S,ch);}}
 cv_text(10,30,&FONT_XS,"GROOVEBOX " Z_EDITION_VERSION,UI_MUTED);z_text_clip(10,44,&FONT_XS,z_sys.name,UI_TEXT,150);
 ui_zscale=1;ui_zox=178;ui_zoy=0;ui_zplay=z_playing();ui_zombie(now);
 ui_card(4,62,74,56,word("ЗВУК","AUDIO"),ui_colors[0]);number((int)cpu,v);{uint32_t k=0;while(v[k])k++;v[k++]='%';v[k]=0;}ui_big(10,76,v,cpu>85?RGB(255,111,145):UI_TEXT);cv_rect(10,100,62,4,UI_GRID);cv_rect(10,100,(int)(cpu>100?100:cpu)*62/100,4,cpu>85?RGB(255,111,145):ui_colors[0]);
  cv_rect(10,108,(int)(ui_meter>100?100:ui_meter)*62/100,3,ui_colors[3]);
 ui_card(83,62,74,56,word("РАБОТАЕТ","UPTIME"),ui_colors[1]);{uint32_t k=0;number((int)(up/3600),b);while(b[k])k++;b[k++]=':';b[k++]=(char)('0'+up/60%60/10);b[k++]=(char)('0'+up/60%10);b[k++]=':';b[k++]=(char)('0'+up%60/10);b[k++]=(char)('0'+up%10);b[k]=0;}ui_big(89,76,b,UI_TEXT);
 ui_card(162,62,74,56,word("ПРОЕКТ","PROJECT"),ui_colors[2]);number((int)notes,v);ui_big(168,76,v,UI_TEXT);z_text(168,100,&FONT_XS,saved_revision!=z_revision()?word("НЕ СОХР.","UNSAVED"):word("СОХРАНЕН","SAVED"),saved_revision!=z_revision()?RGB(255,180,90):UI_MUTED);
 cv_blit(0,0);
 cv_begin(240,120,UI_BG);
 ui_card(4,2,74,56,"MIDI USB",ui_colors[3]);{uint32_t in=plat_midi_count(PORT_USB,0),out=plat_midi_count(PORT_USB,1);int ai=plat_midi_ago(PORT_USB,0)<250,ao=plat_midi_ago(PORT_USB,1)<250;
  cv_rect(10,20,6,6,ai?ui_colors[3]:UI_GRID);number((int)in,v);cv_text(20,18,&FONT_XS,v,UI_TEXT);cv_rect(10,36,6,6,ao?ui_colors[4]:UI_GRID);number((int)out,v);cv_text(20,34,&FONT_XS,v,UI_TEXT);z_text(10,46,&FONT_XS,word("ВХОД/ВЫХОД","IN / OUT"),UI_MUTED);}
 ui_card(83,2,74,56,word("ПРЕСЕТЫ","PRESETS"),ui_colors[4]);ui_big(89,16,"128",UI_TEXT);cv_rect(89,40,32,5,ui_colors[0]);cv_rect(121,40,16,5,ui_colors[4]);cv_rect(137,40,16,5,ui_colors[5]);cv_text(89,47,&FONT_XS,"F  IDM ZMB",UI_MUTED);
 ui_card(162,2,74,56,word("ЯДРО 2","CORE 2"),ui_colors[5]);z_text(168,18,&FONT_XS,core_status==Z_CORE_RUNNING||core_status==Z_CORE_SIMULATED?word("РАБОТАЕТ","RUNNING"):core_status==Z_CORE_OFF?word("ВЫКЛ","OFF"):word("СБОЙ","FAULT"),core_status==Z_CORE_RUNNING||core_status==Z_CORE_SIMULATED?RGB(126,224,140):UI_MUTED);number((int)plat_xruns(),v);z_text(168,34,&FONT_XS,word("СРЫВЫ:","XRUNS:"),UI_MUTED);cv_text(212,34,&FONT_XS,v,UI_TEXT);
 /* the screensaver: which, when, or what holds it back (and what last counted as a touch) */
 ui_round(4,62,232,38,UI_CARD,UI_BG);{uint32_t k=0;const char *p=word("ЗАСТАВКА ","SAVER ");while(*p)b[k++]=*p++;p=!saver_on()?word("ВЫКЛ","OFF"):z_song.language?ui_saver_en[saver_style()]:ui_saver_ru[saver_style()];while(*p&&k<38)b[k++]=*p++;b[k]=0;z_text(10,66,&FONT_XS,b,UI_TEXT);
  static const char *const wr[8]={"","ВЫКЛ В НАСТРОЙКАХ","ОТКРЫТ СЛОЙ","ИДЕТ ЗАПИСЬ","ЗАЖАТ ШАГ","РЕЖИМ DAW","СООБЩЕНИЕ",""},*const we[8]={"","OFF IN SETTINGS","A LAYER IS OPEN","RECORDING","A STEP IS HELD","DAW MODE","A MESSAGE",""};
  if(why&&why<7)z_text(10,82,&FONT_XS,z_song.language?we[why]:wr[why],RGB(255,180,90));else{uint32_t left=idle>=60?0:60-idle;k=0;p=word("ЧЕРЕЗ ","IN ");while(*p)v[k++]=*p++;number((int)left,v+k);while(v[k])k++;v[k++]=' ';p=word("С","S");while(*p)v[k++]=*p++;v[k]=0;z_text(10,82,&FONT_XS,v,ui_colors[3]);cv_rect(120,86,110,3,UI_GRID);cv_rect(120,86,(int)(idle>60?60:idle)*110/60,3,ui_colors[3]);}}
 {static const char *const src[]={"-","KEY","BTN","SELECT","ALGO","PRESET","K1","K2","K3","K4"};uint32_t k=0;const char *p=word("ПОСЛЕДНЕЕ КАСАНИЕ: ","LAST TOUCH: ");while(*p)b[k++]=*p++;p=src[ui_wake_src<10?ui_wake_src:0];while(*p)b[k++]=*p++;b[k]=0;z_text(8,104,&FONT_XS,b,UI_MUTED);}
 cv_blit(0,120);}
static uint32_t ui_hit_at(uint32_t t,uint32_t at){if(!z_drum_track(t))return z_song.track[t].steps[at].velocity!=0;for(uint32_t d=0;d<16;d++)if(at<z_dm.sound[d].length&&z_dm.step[d][at].velocity)return 1;return 0;}
static void ui_zombie_home(void){uint32_t t=z_song.selected,pg=z_song.subpage%2,now=plat_ms(),hits=z_track_hits(t);char b[32];
 ui_header("ZOMBIE FM-1",z_song.recording?(z_playing()?"REC LIVE / 1:16":"REC STEP / NEXT ON RELEASE"):z_playing()?word("ИГРАЕТ / PLAY","PLAYING"):word("ГОТОВ / STOP","READY / STOP"));ui_tracks();
 cv_begin(240,144,UI_BG);
 {uint32_t age=now-ui_boot_at;int rise=age<1800?(int)((1800-age)*58/1800):0,off=ui_zplay?(int)((now/70)%16):0; /* at power-on it climbs out of the ground */
  ui_stars(2,2,100,64,9,now,0);ui_moon(91,16,8,UI_BG);{int bx=(int)((now/45)%280)-60;if(bx>-8&&bx<100)ui_bat(bx,34+(int)(ui_sin(now/800.0f)*6),now,RGB(76,98,70));}
  ui_zscale=2;ui_zox=-16;ui_zoy=8+rise*2;ui_zombie(now);
  for(int x=4-off;x<100;x+=16){cv_rect(x<4?4:x,132,x<4?15-(4-x):15,12,RGB(63,46,30));if(x>=4){cv_rect(x,130,15,3,RGB(84,123,49));cv_rect(x+2,136,4,3,RGB(86,61,38));cv_rect(x+9,139,3,2,RGB(39,30,23));}}
  if(rise)for(uint32_t i=0;i<7;i++){uint32_t k=(age/2+i*53)%60;cv_rect(14+(int)i*11+(int)(k/12),126-(int)(k*(60-k)/40),3,3,i&1?RGB(86,61,38):RGB(63,46,30));}
  ui_draw_sparks(78,58,50,now,UI_BG);}
 z_text_clip(110,4,&FONT_S,z_drum_track(t)?z_drum_name(z_drum_note(z_dm.selected)):z_song.preset[t]==255?word("СВОЙ ЗВУК","CUSTOM SOUND"):z_preset_name(z_song.preset[t]),UI_TEXT,122);
 z_text(110,24,&FONT_XS,hits?(z_drum_track(t)?word("ВСЕ:","ALL:"):word("НОТЫ:","HITS:")):word("ПУСТАЯ ДОРОЖКА","EMPTY TRACK"),hits?UI_MUTED:RGB(151,211,92));if(hits){number(hits,b);cv_text(158,24,&FONT_XS,b,RGB(188,230,117));}
 for(uint32_t k=0;k<4;k++){uint32_t id=page_knob(Z_HOME,pg,k),y=42+k*25;int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*120/(z_desc[id].max-z_desc[id].min):0;ui_round(108,y,126,23,ui_blend(UI_CARD,ui_colors[t],ui_knob_glow(k)*45/100),UI_BG);number(k+1,b);cv_text(111,y+4,&FONT_XS,b,UI_MUTED);z_text_clip(122,y+4,&FONT_XS,label(id),UI_MUTED,id==U_LANGUAGE?46:76);const char *value=value_text(id,b);z_text_clip(id==U_LANGUAGE?174:202,y+2,&FONT_S,value,ui_colors[t],id==U_LANGUAGE?60:30);cv_rect(111,y+21,120,1,UI_GRID);if(amount>0)cv_rect(111,y+21,amount,1,ui_colors[t]);}cv_blit(0,58);
 cv_begin(240,20,UI_BG);for(uint32_t i=0;i<16;i++){uint32_t at=z_song.step_bank*16+i,len=(uint32_t)z_get(t,Z_LENGTH),d=z_playing()?(z_position()%len+len-at)%len:99;uint16_t c=ui_hit_at(t,at)?ui_colors[t]:UI_CARD;if(d<3)c=ui_blend(c,UI_TEXT,d==0?110:d==1?55:25);cv_rect(8+i*14,4,10,8,c);if(d==0)cv_rect(8+i*14,1,10,2,UI_TEXT);}cv_blit(0,202);
 static const char *const en[]={"ALGO: TRACK / SELECT: BPM","REC: STEP / PLAY+REC: LIVE","SEQ+EDIT: DELETE STEP","HOME HOLD: HELP / NEW PROJECT"},*const ru[]={"ALGO: ТРЕК / SELECT: ТЕМП","REC: ШАГИ / PLAY+REC: ИГРА","SEQ+EDIT: УДАЛИТЬ ШАГ","УДЕРЖИ HOME: ПОМОЩЬ / НОВЫЙ"};ui_footer(z_song.language?en[(now/4000)%4]:ru[(now/4000)%4]);
}
/* 0.16 look (after Felucca 1.7's flat cards): transport icons, rounded cards, a live picture of what each page edits. */
static float ui_sin(float t){t-=(int)t;if(t<0)t+=1;float x=t<0.5f?t*2:(t-0.5f)*2,y=4*x*(1-x);y=y*(0.775f+0.225f*y);return t<0.5f?y:-y;}
/* a filled curve: ys[i] is the height (0..h) at column x0+i */
static void ui_fill(int x0,int n,const uint8_t *ys,int h,uint16_t line,uint16_t fill){int i;
 for(i=0;i<n;i++){int y=h-1-ys[i];if(y<0)y=0;cv_rect(x0+i,y+1,1,h-1-y,fill);if(i)cv_line(x0+i-1,h-1-ys[i-1],x0+i,y,line);}}
static void ui_vis_env(int16_t *p,uint16_t a){uint8_t ys[224];int at=p[Z_ATTACK],dc=p[Z_DECAY],su=p[Z_SUSTAIN],re=p[Z_RELEASE],i,h=60;
 int wa=6+at*46/127,wd=6+dc*46/127,wr=6+re*58/127,ws=224-wa-wd-wr,s=su*(h-6)/127;if(ws<20)ws=20;
 for(i=0;i<224;i++){int v;if(i<wa)v=(h-6)*i/wa;else if(i<wa+wd){int k=i-wa;v=(h-6)-((h-6)-s)*k/wd;}else if(i<wa+wd+ws)v=s;else{int k=i-wa-wd-ws;v=k<wr?s-s*k/wr:0;}ys[i]=(uint8_t)(v<0?0:v);}
 ui_fill(8,224,ys,h+4,a,ui_blend(UI_BG,a,28));
 if(wa>=12)cv_text(8+wa/2-3,h+3,&FONT_XS,"A",UI_MUTED);if(wd>=12)cv_text(8+wa+wd/2-3,h+3,&FONT_XS,"D",UI_MUTED);cv_text(8+wa+wd+ws/2-3,h+3,&FONT_XS,"S",UI_MUTED);cv_text(8+wa+wd+ws+wr/2-3,h+3,&FONT_XS,"R",UI_MUTED);}
static void ui_vis_lfo(int16_t *p,uint16_t a,uint32_t now){uint8_t ys[224];int i,depth=p[Z_LFO_DEPTH];float cycles=1+p[Z_LFO_RATE]/24.0f,ph=(float)(now%4000)/4000.0f*(0.4f+p[Z_LFO_RATE]/40.0f);
 for(i=0;i<224;i++){float v=0.5f+0.45f*ui_sin(ph+cycles*i/224.0f)*(depth?0.25f+depth/170.0f:0);ys[i]=(uint8_t)(v*70);}
 ui_fill(8,224,ys,74,depth?a:UI_MUTED,ui_blend(UI_BG,depth?a:UI_GRID,22));if(!depth)z_text(66,16,&FONT_XS,word("LFO ВЫКЛ: K2 ГЛУБИНА","LFO OFF: K2 DEPTH"),UI_MUTED);}
static float ui_shape(float ph,int wave){float u=ph-(int)ph;return wave==0?ui_sin(ph):wave==1?2*u-1:wave==2?(u<0.5f?0.8f:-0.8f):(u<0.5f?4*u-1:3-4*u);}
static void ui_vis_osc(int16_t *p,uint16_t a,uint32_t t){uint8_t ys[224];int i,model=p[Z_MODEL],wave=p[Z_WAVE];uint32_t r=0x9e3779b9u;
 for(i=0;i<224;i++){float ph=i/112.0f,x,u=ph-(int)ph;(void)u;
  if(z_drum_track(t))x=ui_sin(ph*3)*(1-i/224.0f);
  else if(model==0)x=ui_shape(ph,wave)*(1-p[Z_MIX]/254.0f)+ui_shape(ph*(1+p[Z_DETUNE]/400.0f),wave)*p[Z_MIX]/254.0f;
  else if(model==1)x=ui_sin(ph+ui_sin(ph*p[Z_RATIO])*p[Z_FM]/90.0f);
  else if(model==2){float pv=0.08f+p[Z_FM]/151.0f,q=u<pv?u*0.5f/pv:0.5f+(u-pv)*0.5f/(1-pv);x=ui_sin(q);}
  else if(model==3){float m=p[Z_MIX]/127.0f;x=ui_sin(ph)*(1-m)+0.5f*(ui_sin(ph*3)+ui_sin(ph*5))*m;}
  else if(model==4){r^=r<<13;r^=r>>17;r^=r<<5;x=((int)(r>>24)-128)/160.0f;}
  else {float e=1-i/224.0f;x=(model==5?ui_sin(ph*(1.5f-ph*0.5f)):((r^=r<<13,r^=r>>17,r^=r<<5,((int)(r>>24)-128)/128.0f)))*e*e;}
  if(x>1)x=1;if(x<-1)x=-1;ys[i]=(uint8_t)(36+x*32);}
 cv_rect(8,36,224,1,UI_GRID);for(i=1;i<224;i++)cv_line(7+i,72-ys[i-1],8+i,72-ys[i],a);}
static void ui_vis_filter(int16_t *p,uint16_t a){uint8_t ys[224];int i;float fc=0.08f+p[Z_CUTOFF]*0.86f/127.0f,q=p[Z_RESONANCE]/127.0f;
 for(i=0;i<224;i++){float f=i/224.0f,g=f<fc?1:1/(1+(f-fc)*(f-fc)*80),d=(f-fc)*14,peak=q*1.1f/(1+d*d);float v=g*0.62f+peak*0.36f;if(v>1)v=1;ys[i]=(uint8_t)(v*70);}
 ui_fill(8,224,ys,74,a,ui_blend(UI_BG,a,26));cv_rect(8+(int)(fc*224),2,1,72,ui_blend(UI_BG,UI_TEXT,40));}
static void ui_vis_mixer(void){uint32_t i;for(i=0;i<6;i++){int x=10+i*38,lv=z_get(i,Z_LEVEL)*60/127,pan=z_get(i,Z_PAN),mute=z_get(i,Z_MUTE);uint16_t c=mute?UI_MUTED:ui_colors[i];
 ui_round(x,4,30,64,UI_CARD,UI_BG);cv_rect(x+11,66-lv,8,lv,c);cv_rect(x+3+(pan+64)*22/127,70,4,3,c);if(mute)cv_text(x+12,8,&FONT_XS,"M",UI_TEXT);else if(z_solo_state()==i+1)cv_text(x+12,8,&FONT_XS,"S",c);}}
static void ui_vis_fx(int16_t *p,uint16_t a){int i,drive=p[Z_DRIVE];
 ui_round(8,4,70,70,UI_CARD,UI_BG);for(i=0;i<60;i++){float x=(i-30)/30.0f,k=1+drive/20.0f,y=x*k/(1+(x*k<0?-x*k:x*k))*1.4f;if(y>1)y=1;if(y<-1)y=-1;cv_rect(13+i,39-(int)(y*28),2,2,a);}
 ui_round(84,4,70,70,UI_CARD,UI_BG);for(i=0;i<6;i++){int h=p[Z_DELAY]*56/127;for(int k=0;k<i;k++)h=h*36/100;if(h<1)break;cv_rect(92+i*10,70-h,6,h,ui_blend(UI_CARD,a,120-i*14));}
 ui_round(160,4,72,70,UI_CARD,UI_BG);for(i=0;i<64;i++){int h=(int)(p[Z_REVERB]*60/127*(1-i/64.0f)*(1-i/64.0f));cv_rect(164+i,70-h,1,h,ui_blend(UI_CARD,a,60+(i&3)*12));}
 cv_text(12,62,&FONT_XS,"DRV",UI_MUTED);cv_text(88,62,&FONT_XS,"DLY",UI_MUTED);cv_text(164,62,&FONT_XS,"REV",UI_MUTED);}
static void ui_vis_swing(uint16_t a){uint32_t i,now=plat_ms();for(i=0;i<16;i++){int x=10+i*14+((i&1)?z_song.swing*7/60:0),beat=!(i&3),cur=z_playing()&&z_position()%16==i;
 ui_round(x,24-(beat?6:0),10,beat?34:22,cur?UI_TEXT:beat?a:ui_blend(UI_CARD,a,50),UI_BG);}(void)now;}
static void ui_vis_arp(uint32_t t,uint16_t a){uint32_t i,mode=z_song.arp_mode[t],oct=z_song.arp_oct[t];static const uint8_t up[8]={0,1,2,3,4,5,6,7};
 for(i=0;i<16;i++){uint32_t n=mode==0?0:mode==1?up[i%8]:mode==2?7-i%8:mode==3?(i%14<8?i%14:14-i%14):((i*7+3)%8);int h=mode?8+(int)(n*(40+oct*6)/8):4;ui_round(10+i*14,70-h,10,h,mode?ui_blend(UI_CARD,a,60+n*9):UI_GRID,UI_BG);}if(!mode)z_text(66,24,&FONT_XS,word("ARP ВЫКЛ: K1 РЕЖИМ","ARP OFF: K1 MODE"),UI_MUTED);}
static void ui_vis_drum(uint16_t a){uint32_t d=z_dm.selected,i;uint8_t ys[224];float ms=0.2f+z_dm.sound[d].decay/80.0f;
 for(i=0;i<224;i++){float t=i/224.0f/ms*1.6f,e=1/(1+t*t*12);ys[i]=(uint8_t)(e*64);}ui_fill(8,224,ys,70,a,ui_blend(UI_BG,a,26));
 for(i=0;i<16;i++){uint32_t n=0;for(uint32_t at=0;at<64;at++)n+=z_dm.step[i][at].velocity!=0;cv_rect(160+i*4,72-(int)(n>16?16:n),3,(int)(n>16?16:n)+1,i==d?UI_TEXT:a);}}
/* the picture for the page on screen: what the four knobs below change */
static void ui_visual(uint32_t t,uint32_t pg){int16_t *p=z_song.track[t].p;uint16_t a=ui_accent();uint32_t s=z_song.section,now=plat_ms();
 if(z_drum_track(t)&&s!=Z_SEL&&s!=Z_GLO&&s!=Z_FX)ui_vis_drum(a);
 else if(s==Z_ENV&&pg==0)ui_vis_env(p,a);
 else if(s==Z_LFO&&pg==0)ui_vis_lfo(p,a,now);
 else if(s==Z_EDIT&&pg<2)ui_vis_osc(p,a,t);
 else if((s==Z_EDIT&&pg==2)||(s==Z_LFO&&pg==1)||(s==Z_FX&&pg==2))ui_vis_filter(p,a);
 else if(s==Z_SEL)ui_vis_mixer();
 else if(s==Z_FX&&pg==0)ui_vis_fx(p,a);
 else if(s==Z_ARP)ui_vis_arp(t,a);
 else if(s==Z_GLO&&pg==0)ui_vis_swing(a);
 else{uint32_t i;for(i=0;i<16;i++){uint32_t at=z_song.step_bank*16+i;ui_round(8+i*14,22,11,30,ui_hit_at(t,at)?a:UI_CARD,UI_BG);if(z_playing()&&at==z_position()%z_get(t,Z_LENGTH))cv_rect(8+i*14,56,11,3,UI_TEXT);}}
}
static void ui_draw_home(void){if(!Z_SAMPLER_EDITION&&z_song.section==Z_HOME){ui_zombie_home();return;}uint32_t t=z_song.selected,pg=z_song.subpage%page_count[z_song.section],i;char b[32];
 {char sub[48];uint32_t k=0;const char *q=word("СТР. ","PAGE ");while(*q)sub[k++]=*q++;number((int)pg+1,sub+k);while(sub[k])k++;q=word(" ИЗ "," OF ");while(*q)sub[k++]=*q++;number(page_count[z_song.section],sub+k);while(sub[k])k++;
  q=z_song.recording?word(" / ЗАПИСЬ"," / RECORDING"):z_playing()?word(" / ИГРАЕТ"," / PLAYING"):"";while(*q&&k<46)sub[k++]=*q++;sub[k]=0;
  ui_header(z_song.section==Z_HOME?Z_EDITION_TITLE:(z_song.language?section_en[z_song.section]:section_ru[z_song.section]),sub);}ui_tracks();
 cv_begin(240,78,UI_BG);ui_visual(t,pg);
 {const char *name=z_drum_track(t)?z_drum_name(z_drum_note(z_dm.selected)):z_song.preset[t]==255?word("СВОЙ ЗВУК","CUSTOM SOUND"):z_preset_name(z_song.preset[t]);int w=0;for(const char *q=name;*q;q++)if(((uint8_t)*q&0xc0)!=0x80)w+=6;if(w>150)w=150;
  cv_rect(4,0,w+8,13,UI_BG);z_text_clip(8,0,&FONT_XS,name,UI_TEXT,150);const char *tag=z_drum_track(t)?"DRUM KIT":z_models[z_get(t,Z_MODEL)];cv_rect(232-(int)text_w(&FONT_XS,tag)-4,0,(int)text_w(&FONT_XS,tag)+6,13,UI_BG);cv_text(232-(int)text_w(&FONT_XS,tag),0,&FONT_XS,tag,ui_accent());}
 cv_blit(0,58);
 for(i=0;i<2;i++){uint32_t j;cv_begin(240,42,UI_BG);for(j=0;j<2;j++){uint32_t id=page_knob(z_song.section,pg,i*2+j);int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*127/(z_desc[id].max-z_desc[id].min):0;ui_cell(6+j*118,(int)(i*2+j),label(id),value_text(id,b),amount);}cv_blit(0,136+i*42);}
 cv_begin(240,2,UI_BG);cv_blit(0,220);ui_footer(word("SELECT: ТЕМП  КНОПКА: СТР.","SELECT: BPM  BUTTON: PAGE"));
}
static void ui_roll_note(uint32_t at,uint32_t note,uint32_t base,uint32_t selected,uint32_t duration){int row=(int)note-(int)base,x=24+(int)(at%16)*13,w=duration?duration*13-2:10;if(row<0||row>12)return;if(x+w>232)w=232-x;cv_rect(x,2+(12-row)*8,w,6,at==selected?UI_TEXT:C_HI);if(at==selected)cv_rect(x,2+(12-row)*8,2,6,UI_BG);}
static void ui_draw_drum_roll(void){uint32_t selected=z_song.step_edit,bank=selected/16,group=z_dm.selected/6,i,j,len=z_dm.sound[z_dm.selected].length;char b[24],title[48]; /* Cyrillic is 2 bytes a glyph: 29 B of text + number */
 char totals[40];uint32_t n=0,part_hits=0;for(i=0;i<64;i++)part_hits+=z_dm.step[z_dm.selected][i].velocity!=0;const char *prefix=word("ВСЕ: ","ALL: ");while(*prefix)totals[n++]=*prefix++;number(z_track_hits(5),totals+n);while(totals[n])n++;prefix=word(" / ЗВУК: "," / SOUND: ");while(*prefix)totals[n++]=*prefix++;number(part_hits,totals+n);
 {const char *p=word("ДРАМ-КИТ / ШАГОВ ","DRUM KIT / STEPS ");uint32_t k=0,common=z_dm_common_length();while(*p)title[k++]=*p++;if(common)number((int)common,title+k);else{p=word("РАЗНО","MIXED");while(*p)title[k++]=*p++;title[k]=0;}}
 ui_header(title,z_song.recording?(z_playing()?"REC LIVE / GM MIDI":"REC STEP / RELEASE: NEXT"):totals);ui_tracks();
 cv_begin(240,20,UI_BG);z_text_clip(6,2,&FONT_XS,z_drum_name(z_drum_note(z_dm.selected)),C_HI,95);cv_text(98,5,&FONT_XS,group==0?"A1-6":group==1?"B7-12":"C13-16",UI_MUTED);for(i=0;i<4;i++){cv_rect(137+i*23,2,19,14,i==bank?C_HI:i*16<len?UI_CARD:UI_BG);number(i+1,b);cv_text(143+i*23,3,&FONT_XS,b,i==bank?UI_BG:i*16<len?UI_MUTED:UI_GRID);}cv_blit(0,58);
 cv_begin(240,108,UI_BG);for(j=0;j<6;j++){uint32_t d=group*6+j;if(d<16)for(i=0;i<16;i++)if(bank*16+i>=z_dm.sound[d].length)cv_rect(73+i*10,j*17+1,9,15,UI_PAST);}cv_rect(72+(selected%16)*10,0,10,105,RGB(36,55,32));cv_rect(0,(z_dm.selected%6)*17,235,16,RGB(38,58,37));for(i=0;i<6;i++){static const char *const names[16]={"KICK","SNARE","C.HAT","O.HAT","CLAP","RIM","L.TOM","M.TOM","H.TOM","CRASH","RIDE","COWBEL","TAMB","SHAKER","CLAVES","WOOD"};uint32_t d=group*6+i;b[0]='O';b[1]='P';b[2]=(char)('1'+i);b[3]=' ';const char *name=d<16?names[d]:"--";uint32_t n=4;while(*name&&n<23)b[n++]=*name++;b[n]=0;z_text_clip(2,i*17+3,&FONT_XS,b,d==z_dm.selected?UI_TEXT:UI_MUTED,68);cv_rect(72,i*17+16,163,1,UI_GRID);}
 for(i=0;i<=16;i++)cv_rect(72+i*10,0,1,105,i%4?UI_GRID:RGB(55,76,98));
 for(i=0;i<16;i++){uint32_t at=bank*16+i;for(j=0;j<6;j++){uint32_t d=group*6+j;if(d<16&&z_dm.step[d][at].velocity){const z_dm_step_t *v=&z_dm.step[d][at];uint32_t y=3+j*17;
   /* brightness is the hit's velocity; a roll splits the cell into its repeats */
   cv_rect(74+i*10,y,7,12,at>=z_dm.sound[d].length?UI_MUTED:at==selected&&d==z_dm.selected?UI_TEXT:ui_blend(UI_GRID,C_HI,35+v->velocity*92/127));
   for(uint32_t r=1;r<v->ratchet;r++)cv_rect(74+i*10,y+12*r/v->ratchet,7,1,UI_BG);}}}

 {uint32_t head=z_dm_position(z_dm.selected);if(z_playing()&&head/16==bank)cv_rect(72+(head%16)*10,0,1,106,RGB(255,130,163));}cv_blit(0,78);
 cv_begin(240,36,UI_BG);if(grid_hold.key>=0){const z_dm_step_t *h=&z_dm.step[grid_hold.sound][grid_hold.at];static const char *const ru[4]={"ШАГ","ШАНС","СИЛА","ДРОБЬ"},*const en[4]={"STEP","CHANCE","VELOCITY","ROLL"};int vals[4]={grid_hold.at+1,h->chance,h->velocity,h->ratchet};
  ui_round(2,0,236,34,ui_blend(UI_BG,C_HI,30),UI_BG);for(i=0;i<4;i++){z_text_clip(6+i*58,2,&FONT_XS,z_song.language?en[i]:ru[i],UI_TEXT,54);number(vals[i],b);z_text_clip(6+i*58,17,&FONT_XS,h->velocity||i==0?b:"-",C_HI,54);}}
 else if(tools_page())for(i=0;i<4;i++){uint32_t id=layer_knob(i);z_text_clip(6+i*58,2,&FONT_XS,label(id),C_HI,54);z_text_clip(6+i*58,17,&FONT_XS,value_text(id,b),C_HI,54);}
 else for(i=0;i<4;i++){int steps=active_layer==Z_SEQ&&i==0;z_text_clip(6+i*58,2,&FONT_XS,steps?label(U_STEPS):dm_label(i),steps?C_HI:UI_MUTED,54);const char *v=steps?value_text(U_STEPS,b):(number(dm_value(i),b),b);z_text_clip(6+i*58,17,&FONT_XS,v,C_HI,54);}cv_blit(0,186);ui_footer(grid_hold.key>=0?word("ДЕРЖИ ШАГ: K2 ШАНС K3 СИЛА K4 ДРОБЬ","HELD STEP: K2 CHANCE K3 VEL K4 ROLL"):tools_page()?word("ЧЕРН: 1 ЭВКЛ 2 БИТ 3 ПОЛОВ 4 COOK","BLACK: 1 EUCL 2 BEAT 3 HALF 4 COOK"):roll_edit?word("EDIT+БЕЛАЯ: СТЕРЕТЬ ШАГ","EDIT+WHITE: ERASE STEP"):active_layer==Z_SEQ?word("K1: ШАГОВ / PRESET: БАНК","K1: STEPS / PRESET: BANK"):!z_song.recording&&selected>=len?word("ШАГ ЗА ДЛИНОЙ: SEQ+K1 = ШАГОВ","PAST THE LOOP: SEQ+K1 = STEPS"):z_song.recording?word("КЛ: НОТЫ / REC: ЗАКОНЧИТЬ","KEYS: NOTES / REC: FINISH"):word("OP1-6 ЗВУК / PITCH-GLOBAL ГРУППА","OP1-6 SOUND / PITCH-GLOBAL BANK"));
}
static void ui_draw_roll(void){if(z_drum_track(z_song.selected)){ui_draw_drum_roll();return;}uint32_t t=z_song.selected,selected=z_song.step_edit,bank=selected/16,base,i,j,len=(uint32_t)z_get(t,Z_LENGTH);char b[28],value[20],steps[32];z_step *focus=&z_song.track[t].steps[selected];
 uint32_t low=focus->note,high=focus->note;for(j=0;j<3;j++)if(z_song.chord[t][selected][j]){uint32_t n=z_song.chord[t][selected][j]-1;if(n<low)low=n;if(n>high)high=n;}base=high-low<=12?(low+high>12?(low+high)/2-6:0):(focus->note>3?focus->note-3:0);if(base>115)base=115;
 {const char *p=word("ШАГОВ ","STEPS ");uint32_t k=0;while(*p)steps[k++]=*p++;number((int)len,steps+k);while(steps[k])k++;p=word(" / SEQ+K1"," / SEQ+K1");while(*p)steps[k++]=*p++;steps[k]=0;}
 ui_header(word("ПИАНОРОЛЛ","PIANO ROLL"),z_song.recording?(z_playing()?"REC LIVE / 1:16":"REC STEP / RELEASE: NEXT"):steps);ui_tracks();
 cv_begin(240,20,UI_BG);number(selected+1,b);z_text(8,2,&FONT_XS,word("ШАГ","STEP"),UI_MUTED);cv_text(40,2,&FONT_XS,b,selected<len?UI_TEXT:UI_MUTED);b[0]='/';number((int)len,b+1);cv_text(58,2,&FONT_XS,b,UI_MUTED);for(i=0;i<4;i++){cv_rect(137+i*23,2,19,14,i==bank?C_HI:i*16<len?UI_CARD:UI_BG);number(i+1,b);cv_text(143+i*23,3,&FONT_XS,b,i==bank?UI_BG:i*16<len?UI_MUTED:UI_GRID);}cv_blit(0,58);
 cv_begin(240,108,UI_BG);for(i=0;i<16;i++)if(bank*16+i>=len)cv_rect(24+i*13,0,12,105,UI_PAST);cv_rect(24+(selected%16)*13,0,12,105,RGB(36,55,32));
 for(i=0;i<13;i++){uint32_t note=base+12-i;int black=note%12==1||note%12==3||note%12==6||note%12==8||note%12==10;cv_rect(2,1+i*8,19,7,black?UI_CARD:RGB(111,131,149));cv_rect(23,8+i*8,209,1,UI_GRID);if(note%12==0||note==focus->note){ui_note_name(note,b);cv_rect(1,1+i*8,21,8,UI_BG);z_text_clip(1,i*8,&FONT_XS,b,note==focus->note?UI_TEXT:UI_MUTED,21);}}
 for(i=0;i<=16;i++)cv_rect(23+i*13,0,1,105,i%4==0?RGB(55,76,98):UI_GRID);
 cv_rect(24+(selected%16)*13,0,12,1,UI_TEXT);
 for(i=0;i<16;i++){uint32_t at=bank*16+i;z_step *s=&z_song.track[t].steps[at];if(!s->velocity)continue;ui_roll_note(at,s->note,base,selected,z_song.duration[t][at]);for(j=0;j<3;j++)if(z_song.chord[t][at][j])ui_roll_note(at,z_song.chord[t][at][j]-1,base,selected,z_song.duration[t][at]);}
 {uint32_t head=z_position()%z_get(t,Z_LENGTH);if(z_playing()&&head/16==bank)cv_rect(23+(head%16)*13,0,1,106,RGB(255,130,163));}cv_blit(0,78);
 cv_begin(240,36,UI_BG);
 if(grid_hold.key>=0&&!grid_hold.drum){const z_step *h=&z_song.track[t].steps[grid_hold.at];static const char *const ru[4]={"НОТА","СИЛА","ДЛИНА","ГЕЙТ"},*const en[4]={"NOTE","VELOCITY","LENGTH","GATE"};
  ui_round(2,0,236,34,ui_blend(UI_BG,C_HI,30),UI_BG);for(i=0;i<4;i++){z_text_clip(6+i*58,2,&FONT_XS,z_song.language?en[i]:ru[i],UI_TEXT,54);
   if(i==0)ui_note_name(h->note,value);else if(i==1)number(h->velocity,value);else if(i==2){if(z_song.duration[t][grid_hold.at])number(z_song.duration[t][grid_hold.at],value);else{value[0]='A';value[1]='U';value[2]='T';value[3]='O';value[4]=0;}}else number(h->gate,value);
   z_text_clip(6+i*58,17,&FONT_XS,h->velocity||i==0?value:"-",C_HI,54);}}
 else for(i=0;i<4;i++){uint32_t id=active_layer==Z_SEQ?layer_knob(i):page_knob(Z_SEQ,z_song.subpage,i);z_text_clip(6+i*58,2,&FONT_XS,label(id),id==U_STEPS?C_HI:UI_MUTED,54);if(id==U_NOTE){ui_note_name(focus->note,value);}else{const char *v=value_text(id,b);uint32_t k=0;while(v[k]&&k<19){value[k]=v[k];k++;}value[k]=0;}z_text_clip(6+i*58,17,&FONT_XS,value,C_HI,54);}cv_blit(0,186);ui_footer(grid_hold.key>=0&&!grid_hold.drum?word("ШАГ: K1 НОТА K2 СИЛА K3 ДЛИНА K4 ГЕЙТ","STEP: K1 NOTE K2 VEL K3 LEN K4 GATE"):tools_page()?word("ЧЕРН: 1 ЭВКЛ 2 СЛУЧ 3 ПОЛОВ 4 COOK","BLACK: 1 EUCL 2 RAND 3 HALF 4 COOK"):roll_edit?word("КЛАВИШИ: ДОБАВИТЬ / УБРАТЬ","KEYS: ADD / REMOVE AT STEP"):active_layer==Z_SEQ?word("K1: ШАГОВ / PRESET: БАНК","K1: STEPS / PRESET: BANK"):!z_song.recording&&selected>=len?word("ШАГ ЗА ДЛИНОЙ: SEQ+K1 = ШАГОВ","PAST THE LOOP: SEQ+K1 = STEPS"):z_song.recording?word("КЛ: НОТЫ / REC: ЗАКОНЧИТЬ","KEYS: NOTES / REC: FINISH"):word("КЛ: 16 ШАГОВ / EDIT: УДАЛИТЬ","KEYS: 16 STEPS / EDIT: DELETE"));
}
static void edition_splash(void){uint32_t y,i;
 for(y=0;y<240;y+=24){cv_begin(240,24,0);for(i=0;i<5760;i++){cv_px[i]=swap16(Z_LOGO240_PAL[Z_LOGO240_PX[y*240+i]]);}cv_blit(0,y);}lcd_sync();
}

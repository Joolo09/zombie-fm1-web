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
static uint16_t ui_accent(void){return ui_colors[z_song.selected];}
static uint16_t ui_blend(uint16_t a,uint16_t b,uint32_t amount){uint32_t r=((a>>11)*(127-amount)+(b>>11)*amount)/127,g=(((a>>5)&63)*(127-amount)+((b>>5)&63)*amount)/127,bl=((a&31)*(127-amount)+(b&31)*amount)/127;return (r<<11)|(g<<5)|bl;}
static void ui_round(int x,int y,int w,int h,uint16_t c,uint16_t bg){cv_rect(x,y,w,h,c);cv_rect(x,y,1,1,bg);cv_rect(x+w-1,y,1,1,bg);cv_rect(x,y+h-1,1,1,bg);cv_rect(x+w-1,y+h-1,1,1,bg);}
static void ui_icon(int x,int y,uint16_t c){uint32_t i;
 if(z_countin_state()||z_song.recording){static const uint8_t w[9]={3,7,9,9,9,9,9,7,3};for(i=0;i<9;i++)cv_rect(x+(9-w[i])/2,y+i,w[i],1,RGB(255,111,145));}
 else if(z_playing())for(i=0;i<5;i++)cv_rect(x+1+i,y+i,1,9-2*i,RGB(126,224,140));
 else cv_rect(x+1,y+1,7,7,c);}
static void edition_note(uint32_t track,uint32_t note,uint32_t velocity,uint32_t on){if(z_drum_track(track)&&on&&velocity){z_dm_select(z_drum_index(note));drum_choice=(uint8_t)z_drum_note(z_dm.selected);}if(Z_SAMPLER_EDITION&&on&&velocity&&note>=36&&note<68)ui_pad_until[note-36]=plat_ms()+180;}
static void ui_prepare(void){uint32_t key=z_song.section|(z_song.subpage<<4)|((active_layer+1)<<8)|(z_song.selected<<16)|(z_sys.midi_mode<<20),now=plat_ms();
 pal[0]=UI_GRID;pal[1]=RGB(62,82,103);pal[2]=UI_MUTED;pal[3]=ui_accent();pal[4]=Z_SAMPLER_EDITION?RGB(191,153,255):ui_accent();
 if(key!=ui_view_key){ui_view_key=key;ui_view_since=now;}
 ui_meter=output_peak>ui_meter?output_peak:ui_meter*3/4;
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
  if(z_get(i,Z_MUTE))cv_text(x+20,6,&FONT_XS,"M",sel?UI_BG:UI_MUTED);else if(z_solo_state()==i+1)cv_text(x+20,6,&FONT_XS,"S",sel?UI_BG:c);else if(z_track_hits(i))cv_rect(x+24,10,4,4,sel?UI_BG:c);}cv_blit(0,34);
}
static void ui_footer(const char *hint){cv_begin(240,18,UI_BG);if((int32_t)(message_until-plat_ms())>0)z_text_clip(7,2,&FONT_XS,message,UI_TEXT,226);else z_text_clip(7,2,&FONT_XS,hint,UI_MUTED,226);cv_blit(0,222);}
static void ui_transition(void){uint32_t age=plat_ms()-ui_view_since;
 cv_begin(240,2,UI_GRID);if(age<220)cv_rect(0,0,(220-age)*240/220,2,C_HI);else if(z_playing())cv_rect((z_position()%16)*15,0,15,2,C_HI);cv_blit(0,238);
}
static void ui_note_name(uint32_t note,char *b){static const char *const names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};const char *p=names[note%12];uint32_t n=0;while(*p)b[n++]=*p++;number((int)note/12-1,b+n);}
static void ui_cell(int x,int y,const char *name,const char *value,int amount){
 uint32_t age=plat_ms()-ui_view_since,offset=age<220?(220-age)*4/220:0;uint16_t a=ui_accent();ui_round(x,0,110,40,UI_CARD,UI_BG);
 z_text_clip(x+7,4,&FONT_XS,name,UI_MUTED,98);z_text_clip(x+7,17+offset,&FONT_S,value,a,98);
 ui_round(x+7,35,96,3,UI_GRID,UI_CARD);if(amount>0){int w=clamp(amount,0,127)*96/127;cv_rect(x+7,35,w,3,a);cv_rect(x+6+w,34,2,5,UI_TEXT);}(void)y;
}
/* Original block zombie: procedural pixels, display-only breathing, blink and walking. */
static int ui_zscale=1,ui_zox,ui_zoy;
static void ui_zrect(int x,int y,int w,int h,uint16_t c){cv_rect(ui_zox+x*ui_zscale,ui_zoy+y*ui_zscale,w*ui_zscale,h*ui_zscale,c);}
static void ui_zombie(uint32_t now){
 int walk=z_playing()?((now/150)&1):0,bob=z_playing()?((now/180)&1):((now/700)&1),x=12,y=1+bob;
 uint16_t dark=RGB(12,26,17),skin=RGB(100,154,58),shade=RGB(60,108,44),light=RGB(147,186,78),shirt=RGB(31,119,111),pants=RGB(66,59,105);
 ui_zrect(x+9,52,29,3,RGB(18,28,17));
 ui_zrect(x+9,y+29,22,16,dark);ui_zrect(x+11,y+29,18,14,shirt);ui_zrect(x+12,y+31,4,11,RGB(35,139,126));ui_zrect(x+24,y+33,4,9,RGB(24,87,84));
 ui_zrect(x+2,y+28+walk,8,17,shade);ui_zrect(x+30,y+28-walk,8,17,skin);ui_zrect(x+2,y+40+walk,8,4,skin);ui_zrect(x+30,y+40-walk,8,4,light);
 ui_zrect(x+11,y+43,8,9+walk,pants);ui_zrect(x+21,y+43,8,10-walk,pants);ui_zrect(x+10,y+51+walk,10,3,dark);ui_zrect(x+20,y+52-walk,10,3,dark);
 ui_zrect(x+5,y,30,29,dark);ui_zrect(x+7,y+2,26,25,skin);ui_zrect(x+7,y+2,26,5,shade);ui_zrect(x+9,y+3,5,3,light);ui_zrect(x+22,y+4,8,3,RGB(72,127,46));
 ui_zrect(x+7,y+10,4,8,shade);ui_zrect(x+29,y+8,4,17,shade);ui_zrect(x+11,y+21,5,5,light);ui_zrect(x+22,y+22,6,4,shade);ui_zrect(x+17,y+15,5,6,shade);
 int blink=(now/100)%45==0;ui_zrect(x+11,y+11,7,blink?2:5,dark);ui_zrect(x+23,y+11,7,blink?2:5,dark);
 if(!blink){uint16_t eye=z_song.recording?RGB(255,105,89):RGB(203,224,148);ui_zrect(x+12,y+12,3,2,eye);ui_zrect(x+24,y+12,3,2,eye);}
 ui_zrect(x+15,y+23,10,2,dark);ui_zrect(x+19,y+25,7,1,shade);
}
static uint32_t ui_hit_at(uint32_t t,uint32_t at){if(!z_drum_track(t))return z_song.track[t].steps[at].velocity!=0;for(uint32_t d=0;d<16;d++)if(at<z_dm.sound[d].length&&z_dm.step[d][at].velocity)return 1;return 0;}
static void ui_zombie_home(void){uint32_t t=z_song.selected,pg=z_song.subpage%2,now=plat_ms(),hits=z_track_hits(t);char b[32];
 ui_header("ZOMBIE FM-1",z_song.recording?(z_playing()?"REC LIVE / 1:16":"REC STEP / NEXT ON RELEASE"):z_playing()?word("ИГРАЕТ / PLAY","PLAYING"):word("ГОТОВ / STOP","READY / STOP"));ui_tracks();
 cv_begin(240,144,UI_BG);ui_zscale=2;ui_zox=-16;ui_zoy=8;ui_zombie(now);
 for(uint32_t x=4;x<100;x+=16){cv_rect(x,132,15,12,RGB(63,46,30));cv_rect(x,130,15,3,RGB(84,123,49));cv_rect(x+2,136,4,3,RGB(86,61,38));cv_rect(x+9,139,3,2,RGB(39,30,23));}
 for(uint32_t i=0;i<3;i++){uint32_t x=6+(i*31+now/90)%91,y=10+(i*37+now/190)%110;cv_rect(x,y,2,2,RGB(71,100,43));}
 z_text_clip(110,4,&FONT_S,z_drum_track(t)?z_drum_name(z_drum_note(z_dm.selected)):z_song.preset[t]==255?word("СВОЙ ЗВУК","CUSTOM SOUND"):z_preset_name(z_song.preset[t]),UI_TEXT,122);
 z_text(110,24,&FONT_XS,hits?(z_drum_track(t)?word("ВСЕ:","ALL:"):word("НОТЫ:","HITS:")):word("ПУСТАЯ ДОРОЖКА","EMPTY TRACK"),hits?UI_MUTED:RGB(151,211,92));if(hits){number(hits,b);cv_text(158,24,&FONT_XS,b,RGB(188,230,117));}
 for(uint32_t k=0;k<4;k++){uint32_t id=page_knob(Z_HOME,pg,k),y=42+k*25;int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*120/(z_desc[id].max-z_desc[id].min):0;ui_round(108,y,126,23,UI_CARD,UI_BG);number(k+1,b);cv_text(111,y+4,&FONT_XS,b,UI_MUTED);z_text_clip(122,y+4,&FONT_XS,label(id),UI_MUTED,id==U_LANGUAGE?46:76);const char *value=value_text(id,b);z_text_clip(id==U_LANGUAGE?174:202,y+2,&FONT_S,value,ui_colors[t],id==U_LANGUAGE?60:30);cv_rect(111,y+21,120,1,UI_GRID);if(amount>0)cv_rect(111,y+21,amount,1,ui_colors[t]);}cv_blit(0,58);
 cv_begin(240,20,UI_BG);for(uint32_t i=0;i<16;i++){uint32_t at=z_song.step_bank*16+i;cv_rect(8+i*14,4,10,8,ui_hit_at(t,at)?ui_colors[t]:UI_CARD);if(z_playing()&&at==z_position()%z_get(t,Z_LENGTH))cv_rect(8+i*14,1,10,2,UI_TEXT);}cv_blit(0,202);
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
 for(i=0;i<2;i++){uint32_t j;cv_begin(240,42,UI_BG);for(j=0;j<2;j++){uint32_t id=page_knob(z_song.section,pg,i*2+j);int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*127/(z_desc[id].max-z_desc[id].min):0;ui_cell(6+j*118,0,label(id),value_text(id,b),amount);}cv_blit(0,136+i*42);}
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
 else for(i=0;i<4;i++){int steps=active_layer==Z_SEQ&&i==0;z_text_clip(6+i*58,2,&FONT_XS,steps?label(U_STEPS):dm_label(i),steps?C_HI:UI_MUTED,54);const char *v=steps?value_text(U_STEPS,b):(number(dm_value(i),b),b);z_text_clip(6+i*58,17,&FONT_XS,v,C_HI,54);}cv_blit(0,186);ui_footer(grid_hold.key>=0?word("ДЕРЖИ ШАГ: K2 ШАНС K3 СИЛА K4 ДРОБЬ","HELD STEP: K2 CHANCE K3 VEL K4 ROLL"):roll_edit?word("EDIT+БЕЛАЯ: СТЕРЕТЬ ШАГ","EDIT+WHITE: ERASE STEP"):active_layer==Z_SEQ?word("K1: ШАГОВ / PRESET: БАНК","K1: STEPS / PRESET: BANK"):!z_song.recording&&selected>=len?word("ШАГ ЗА ДЛИНОЙ: SEQ+K1 = ШАГОВ","PAST THE LOOP: SEQ+K1 = STEPS"):z_song.recording?word("КЛ: НОТЫ / REC: ЗАКОНЧИТЬ","KEYS: NOTES / REC: FINISH"):word("OP1-6 ЗВУК / PITCH-GLOBAL ГРУППА","OP1-6 SOUND / PITCH-GLOBAL BANK"));
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
 else for(i=0;i<4;i++){uint32_t id=active_layer==Z_SEQ?layer_knob(i):page_knob(Z_SEQ,z_song.subpage,i);z_text_clip(6+i*58,2,&FONT_XS,label(id),id==U_STEPS?C_HI:UI_MUTED,54);if(id==U_NOTE){ui_note_name(focus->note,value);}else{const char *v=value_text(id,b);uint32_t k=0;while(v[k]&&k<19){value[k]=v[k];k++;}value[k]=0;}z_text_clip(6+i*58,17,&FONT_XS,value,C_HI,54);}cv_blit(0,186);ui_footer(grid_hold.key>=0&&!grid_hold.drum?word("ДЕРЖИ ШАГ: K1 НОТА K2 СИЛА K3 ДЛИНА K4 ГЕЙТ","HELD STEP: K1 NOTE K2 VEL K3 LEN K4 GATE"):roll_edit?word("КЛАВИШИ: ДОБАВИТЬ / УБРАТЬ","KEYS: ADD / REMOVE AT STEP"):active_layer==Z_SEQ?word("K1: ШАГОВ / PRESET: БАНК","K1: STEPS / PRESET: BANK"):!z_song.recording&&selected>=len?word("ШАГ ЗА ДЛИНОЙ: SEQ+K1 = ШАГОВ","PAST THE LOOP: SEQ+K1 = STEPS"):z_song.recording?word("КЛ: НОТЫ / REC: ЗАКОНЧИТЬ","KEYS: NOTES / REC: FINISH"):word("КЛ: 16 ШАГОВ / EDIT: УДАЛИТЬ","KEYS: 16 STEPS / EDIT: DELETE"));
}
static void edition_splash(void){uint32_t y,i;
 for(y=0;y<240;y+=24){cv_begin(240,24,0);for(i=0;i<5760;i++){cv_px[i]=swap16(Z_LOGO240_PAL[Z_LOGO240_PX[y*240+i]]);}cv_blit(0,y);}lcd_sync();
}

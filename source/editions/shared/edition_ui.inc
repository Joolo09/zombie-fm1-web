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
static void edition_note(uint32_t track,uint32_t note,uint32_t velocity,uint32_t on){if(z_drum_track(track)&&on&&velocity){z_dm_select(z_drum_index(note));drum_choice=(uint8_t)z_drum_note(z_dm.selected);}if(Z_SAMPLER_EDITION&&on&&velocity&&note>=36&&note<68)ui_pad_until[note-36]=plat_ms()+180;}
static void ui_prepare(void){uint32_t key=z_song.section|(z_song.subpage<<4)|((active_layer+1)<<8)|(z_song.selected<<16)|(z_sys.midi_mode<<20),now=plat_ms();
 pal[0]=UI_GRID;pal[1]=RGB(62,82,103);pal[2]=UI_MUTED;pal[3]=ui_accent();pal[4]=Z_SAMPLER_EDITION?RGB(191,153,255):ui_accent();
 if(key!=ui_view_key){ui_view_key=key;ui_view_since=now;}
 ui_meter=output_peak>ui_meter?output_peak:ui_meter*3/4;
 if(Z_SAMPLER_EDITION&&z_playing()){uint32_t t=z_song.selected,at=z_position()%z_get(t,Z_LENGTH);z_step *s=&z_song.track[t].steps[at];if(s->velocity&&s->note>=36&&s->note<68)ui_pad_until[s->note-36]=now+65;}
}
static void ui_header(const char *title,const char *subtitle){char b[16];uint16_t color=C_HI;
 cv_begin(240,34,UI_BG);cv_rect(0,0,240,2,color);z_text_clip(8,5,&FONT_XS,title,UI_TEXT,150);z_text_clip(8,19,&FONT_XS,subtitle,z_song.recording?RGB(255,111,145):UI_MUTED,150);
 if(saved_revision!=z_revision()||system_dirty)cv_text(162,5,&FONT_XS,"*",color);
 number(z_sys.midi_mode==2?daw_bpm:z_song.bpm,b);cv_text(201-text_w(&FONT_M,b)/2,1,&FONT_M,b,UI_TEXT);cv_text(201-text_w(&FONT_XS,"BPM")/2,23,&FONT_XS,"BPM",UI_MUTED);cv_blit(0,0);
}
static void ui_tracks(void){uint32_t i;char b[4];cv_begin(240,24,UI_BG);
 for(i=0;i<6;i++){int x=6+i*38,sel=i==z_song.selected;cv_rect(x,2,34,18,sel?ui_colors[i]:UI_CARD);cv_rect(x,2,34,2,ui_colors[i]);number(i+1,b);cv_text(x+4,5,&FONT_XS,b,sel?UI_BG:UI_TEXT);if(z_track_hits(i))cv_rect(x+27,7,3,3,sel?UI_BG:ui_colors[i]);if(z_get(i,Z_MUTE))cv_text(x+19,5,&FONT_XS,"M",sel?UI_BG:UI_MUTED);else if(z_solo_state()==i+1)cv_text(x+19,5,&FONT_XS,"S",sel?UI_BG:ui_colors[i]);}cv_blit(0,34);
}
static void ui_footer(const char *hint){cv_begin(240,18,UI_BG);if((int32_t)(message_until-plat_ms())>0)z_text_clip(7,2,&FONT_XS,message,UI_TEXT,226);else z_text_clip(7,2,&FONT_XS,hint,UI_MUTED,226);cv_blit(0,222);}
static void ui_transition(void){uint32_t age=plat_ms()-ui_view_since;
 cv_begin(240,2,UI_GRID);if(age<220)cv_rect(0,0,(220-age)*240/220,2,C_HI);else if(z_playing())cv_rect((z_position()%16)*15,0,15,2,C_HI);cv_blit(0,238);
}
static void ui_note_name(uint32_t note,char *b){static const char *const names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};const char *p=names[note%12];uint32_t n=0;while(*p)b[n++]=*p++;number((int)note/12-1,b+n);}
static void ui_cell(int x,int y,const char *name,const char *value,int amount){
 uint32_t age=plat_ms()-ui_view_since,offset=age<220?(220-age)*4/220:0;cv_rect(x,0,110,40,UI_CARD);z_text_clip(x+6,3,&FONT_XS,name,UI_MUTED,98);z_text_clip(x+6,17+offset,&FONT_S,value,C_HI,98);cv_rect(x+6,36,98,2,UI_GRID);if(amount>0)cv_rect(x+6,36,clamp(amount,0,127)*98/127,2,C_HI);(void)y;
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
 for(uint32_t k=0;k<4;k++){uint32_t id=page_knob(Z_HOME,pg,k),y=42+k*25;int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*120/(z_desc[id].max-z_desc[id].min):0;cv_rect(108,y,126,23,UI_CARD);number(k+1,b);cv_text(111,y+4,&FONT_XS,b,UI_MUTED);z_text_clip(122,y+4,&FONT_XS,label(id),UI_MUTED,id==U_LANGUAGE?46:76);const char *value=value_text(id,b);z_text_clip(id==U_LANGUAGE?174:202,y+2,&FONT_S,value,ui_colors[t],id==U_LANGUAGE?60:30);cv_rect(111,y+21,120,1,UI_GRID);if(amount>0)cv_rect(111,y+21,amount,1,ui_colors[t]);}cv_blit(0,58);
 cv_begin(240,20,UI_BG);for(uint32_t i=0;i<16;i++){uint32_t at=z_song.step_bank*16+i;cv_rect(8+i*14,4,10,8,ui_hit_at(t,at)?ui_colors[t]:UI_CARD);if(z_playing()&&at==z_position()%z_get(t,Z_LENGTH))cv_rect(8+i*14,1,10,2,UI_TEXT);}cv_blit(0,202);
 static const char *const en[]={"ALGO: TRACK / SELECT: BPM","REC: STEP / PLAY+REC: LIVE","SEQ+EDIT: DELETE STEP","HOME HOLD: HELP / NEW PROJECT"},*const ru[]={"ALGO: ТРЕК / SELECT: ТЕМП","REC: ШАГИ / PLAY+REC: ИГРА","SEQ+EDIT: УДАЛИТЬ ШАГ","УДЕРЖИ HOME: ПОМОЩЬ / НОВЫЙ"};ui_footer(z_song.language?en[(now/4000)%4]:ru[(now/4000)%4]);
}
static void ui_draw_home(void){if(!Z_SAMPLER_EDITION&&z_song.section==Z_HOME){ui_zombie_home();return;}uint32_t t=z_song.selected,pg=z_song.subpage%page_count[z_song.section],i;char b[32];
 ui_header(z_song.section==Z_HOME?Z_EDITION_TITLE:(z_song.language?section_en[z_song.section]:section_ru[z_song.section]),z_song.recording?word("ЗАПИСЬ / REC","RECORDING"):z_playing()?word("ИГРАЕТ / USB MIDI","PLAYING / USB MIDI"):"STANDALONE / USB MIDI");ui_tracks();
 cv_begin(240,42,UI_BG);z_text_clip(8,3,&FONT_S,z_drum_track(t)?"DRUM KIT / 16 SOUNDS":z_song.preset[t]==255?word("СВОЙ ЗВУК","CUSTOM SOUND"):z_preset_name(z_song.preset[t]),UI_TEXT,224);cv_text(8,25,&FONT_XS,z_drum_track(t)?"GM / CH 6":z_models[z_get(t,Z_MODEL)],C_HI);cv_text(132,25,&FONT_XS,"OCT",UI_MUTED);number(z_song.octave,b);cv_text(158,25,&FONT_XS,b,UI_TEXT);cv_text(185,25,&FONT_XS,z_song.arp_mode[t]?(z_song.arp_hold[t]?"HOLD":"ARP"):"",C_HI);cv_blit(0,58);
 cv_begin(240,16,UI_BG);cv_text(8,1,&FONT_XS,"OUT",UI_MUTED);cv_rect(35,5,158,5,UI_GRID);cv_rect(35,5,ui_meter*158/127,5,C_HI);number(z_voice_count(),b);cv_text(205,1,&FONT_XS,b,UI_TEXT);cv_text(213,1,&FONT_XS,Z_SAMPLER_EDITION?"/8":"/24",UI_MUTED);cv_blit(0,100);
 for(i=0;i<2;i++){uint32_t j;cv_begin(240,44,UI_BG);for(j=0;j<2;j++){uint32_t id=page_knob(z_song.section,pg,i*2+j);int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*127/(z_desc[id].max-z_desc[id].min):0;ui_cell(6+j*118,0,label(id),value_text(id,b),amount);}cv_blit(0,116+i*44);}
 cv_begin(240,18,UI_BG);for(i=0;i<16;i++){uint32_t at=z_song.step_bank*16+i;cv_rect(8+i*14,4,10,8,ui_hit_at(t,at)?C_HI:UI_CARD);if(z_playing()&&at==z_position()%z_get(t,Z_LENGTH))cv_rect(8+i*14,1,10,2,UI_TEXT);}cv_blit(0,204);ui_footer(word("SELECT: ТЕМП  КНОПКА: СТР.","SELECT: BPM  BUTTON: PAGE"));
}
static void ui_roll_note(uint32_t at,uint32_t note,uint32_t base,uint32_t selected,uint32_t duration){int row=(int)note-(int)base,x=24+(int)(at%16)*13,w=duration?duration*13-2:10;if(row<0||row>12)return;if(x+w>232)w=232-x;cv_rect(x,2+(12-row)*8,w,6,at==selected?UI_TEXT:C_HI);if(at==selected)cv_rect(x,2+(12-row)*8,2,6,UI_BG);}
static void ui_draw_drum_roll(void){uint32_t selected=z_song.step_edit,bank=selected/16,group=z_dm.selected/6,i,j,len=z_dm.sound[z_dm.selected].length;char b[24],title[48]; /* Cyrillic is 2 bytes a glyph: 29 B of text + number */
 char totals[40];uint32_t n=0,part_hits=0;for(i=0;i<64;i++)part_hits+=z_dm.step[z_dm.selected][i].velocity!=0;const char *prefix=word("ВСЕ: ","ALL: ");while(*prefix)totals[n++]=*prefix++;number(z_track_hits(5),totals+n);while(totals[n])n++;prefix=word(" / ЗВУК: "," / SOUND: ");while(*prefix)totals[n++]=*prefix++;number(part_hits,totals+n);
 {const char *p=word("ДРАМ-КИТ / ШАГОВ ","DRUM KIT / STEPS ");uint32_t k=0,common=z_dm_common_length();while(*p)title[k++]=*p++;if(common)number((int)common,title+k);else{p=word("РАЗНО","MIXED");while(*p)title[k++]=*p++;title[k]=0;}}
 ui_header(title,z_song.recording?(z_playing()?"REC LIVE / GM MIDI":"REC STEP / RELEASE: NEXT"):totals);ui_tracks();
 cv_begin(240,20,UI_BG);z_text_clip(6,2,&FONT_XS,z_drum_name(z_drum_note(z_dm.selected)),C_HI,95);cv_text(98,5,&FONT_XS,group==0?"A1-6":group==1?"B7-12":"C13-16",UI_MUTED);for(i=0;i<4;i++){cv_rect(137+i*23,2,19,14,i==bank?C_HI:i*16<len?UI_CARD:UI_BG);number(i+1,b);cv_text(143+i*23,3,&FONT_XS,b,i==bank?UI_BG:i*16<len?UI_MUTED:UI_GRID);}cv_blit(0,58);
 cv_begin(240,108,UI_BG);for(j=0;j<6;j++){uint32_t d=group*6+j;if(d<16)for(i=0;i<16;i++)if(bank*16+i>=z_dm.sound[d].length)cv_rect(73+i*10,j*17+1,9,15,UI_PAST);}cv_rect(72+(selected%16)*10,0,10,105,RGB(36,55,32));cv_rect(0,(z_dm.selected%6)*17,235,16,RGB(38,58,37));for(i=0;i<6;i++){static const char *const names[16]={"KICK","SNARE","C.HAT","O.HAT","CLAP","RIM","L.TOM","M.TOM","H.TOM","CRASH","RIDE","COWBEL","TAMB","SHAKER","CLAVES","WOOD"};uint32_t d=group*6+i;b[0]='O';b[1]='P';b[2]=(char)('1'+i);b[3]=' ';const char *name=d<16?names[d]:"--";uint32_t n=4;while(*name&&n<23)b[n++]=*name++;b[n]=0;z_text_clip(2,i*17+3,&FONT_XS,b,d==z_dm.selected?UI_TEXT:UI_MUTED,68);cv_rect(72,i*17+16,163,1,UI_GRID);}
 for(i=0;i<=16;i++)cv_rect(72+i*10,0,1,105,i%4?UI_GRID:RGB(55,76,98));
 for(i=0;i<16;i++){uint32_t at=bank*16+i;for(j=0;j<6;j++){uint32_t d=group*6+j;if(d<16&&z_dm.step[d][at].velocity)cv_rect(74+i*10,3+j*17,7,12,at>=z_dm.sound[d].length?UI_MUTED:at==selected&&d==z_dm.selected?UI_TEXT:C_HI);}}

 {uint32_t head=z_dm_position(z_dm.selected);if(z_playing()&&head/16==bank)cv_rect(72+(head%16)*10,0,1,106,RGB(255,130,163));}cv_blit(0,78);
 cv_begin(240,36,UI_BG);for(i=0;i<4;i++){int steps=active_layer==Z_SEQ&&i==0;z_text_clip(6+i*58,2,&FONT_XS,steps?label(U_STEPS):dm_label(i),steps?C_HI:UI_MUTED,54);const char *v=steps?value_text(U_STEPS,b):(number(dm_value(i),b),b);z_text_clip(6+i*58,17,&FONT_XS,v,C_HI,54);}cv_blit(0,186);ui_footer(roll_edit?word("EDIT+БЕЛАЯ: СТЕРЕТЬ ШАГ","EDIT+WHITE: ERASE STEP"):active_layer==Z_SEQ?word("K1: ШАГОВ / PRESET: БАНК","K1: STEPS / PRESET: BANK"):!z_song.recording&&selected>=len?word("ШАГ ЗА ДЛИНОЙ: SEQ+K1 = ШАГОВ","PAST THE LOOP: SEQ+K1 = STEPS"):z_song.recording?word("КЛ: НОТЫ / REC: ЗАКОНЧИТЬ","KEYS: NOTES / REC: FINISH"):word("OP1-6 ЗВУК / PITCH-GLOBAL ГРУППА","OP1-6 SOUND / PITCH-GLOBAL BANK"));
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
 for(i=0;i<4;i++){uint32_t id=active_layer==Z_SEQ?layer_knob(i):page_knob(Z_SEQ,z_song.subpage,i);z_text_clip(6+i*58,2,&FONT_XS,label(id),id==U_STEPS?C_HI:UI_MUTED,54);if(id==U_NOTE){ui_note_name(focus->note,value);}else{const char *v=value_text(id,b);uint32_t k=0;while(v[k]&&k<19){value[k]=v[k];k++;}value[k]=0;}z_text_clip(6+i*58,17,&FONT_XS,value,C_HI,54);}cv_blit(0,186);ui_footer(roll_edit?word("КЛАВИШИ: ДОБАВИТЬ / УБРАТЬ","KEYS: ADD / REMOVE AT STEP"):active_layer==Z_SEQ?word("K1: ШАГОВ / PRESET: БАНК","K1: STEPS / PRESET: BANK"):!z_song.recording&&selected>=len?word("ШАГ ЗА ДЛИНОЙ: SEQ+K1 = ШАГОВ","PAST THE LOOP: SEQ+K1 = STEPS"):z_song.recording?word("КЛ: НОТЫ / REC: ЗАКОНЧИТЬ","KEYS: NOTES / REC: FINISH"):word("КЛ: 16 ШАГОВ / EDIT: УДАЛИТЬ","KEYS: 16 STEPS / EDIT: DELETE"));
}
static void edition_splash(void){uint32_t y,i;
 for(y=0;y<240;y+=24){cv_begin(240,24,0);for(i=0;i<5760;i++){cv_px[i]=swap16(Z_LOGO240_PAL[Z_LOGO240_PX[y*240+i]]);}cv_blit(0,y);}lcd_sync();
}

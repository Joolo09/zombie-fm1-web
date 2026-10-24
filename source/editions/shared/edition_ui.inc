/* SPDX-License-Identifier: MIT — original 240x240 Afterglow device UI.
 * Animation is display-only: no sleeps, MIDI sends, audio writes or heap use. */
#define UI_BG RGB(8,14,24)
#define UI_CARD RGB(18,30,46)
#define UI_GRID RGB(30,45,63)
#define UI_TEXT RGB(231,242,246)
#define UI_MUTED RGB(133,158,178)
#undef C_BLACK
#define C_BLACK UI_BG
static const uint16_t ui_colors[6]={RGB(255,154,91),RGB(242,106,144),RGB(243,210,112),RGB(112,235,198),RGB(123,180,255),RGB(190,144,249)};
static uint32_t ui_view_key=0xffffffffu,ui_view_since,ui_pad_until[32],ui_wave_at;
static uint8_t ui_meter,ui_wave[96],ui_wave_source=255;
static uint16_t ui_accent(void){return ui_colors[z_song.selected];}
static uint16_t ui_blend(uint16_t a,uint16_t b,uint32_t amount){uint32_t r=((a>>11)*(127-amount)+(b>>11)*amount)/127,g=(((a>>5)&63)*(127-amount)+((b>>5)&63)*amount)/127,bl=((a&31)*(127-amount)+(b&31)*amount)/127;return (r<<11)|(g<<5)|bl;}
static void edition_note(uint32_t note,uint32_t velocity,uint32_t on){if(Z_SAMPLER_EDITION&&on&&velocity&&note>=36&&note<68)ui_pad_until[note-36]=plat_ms()+180;}
static void ui_prepare(void){uint32_t key=z_song.section|(z_song.subpage<<4)|((active_layer+1)<<8)|(z_song.selected<<16)|(z_sys.midi_mode<<20),now=plat_ms();
 pal[0]=UI_GRID;pal[1]=RGB(62,82,103);pal[2]=UI_MUTED;pal[3]=ui_accent();pal[4]=Z_SAMPLER_EDITION?RGB(191,153,255):ui_accent();
 if(key!=ui_view_key){ui_view_key=key;ui_view_since=now;}
 ui_meter=output_peak>ui_meter?output_peak:ui_meter*3/4;
 if(Z_SAMPLER_EDITION&&z_playing()){uint32_t t=z_song.selected,at=z_position()%z_get(t,Z_LENGTH);z_step *s=&z_song.track[t].steps[at];if(s->velocity&&s->note>=36&&s->note<68)ui_pad_until[s->note-36]=now+65;}
}
static void ui_header(const char *title,const char *subtitle){char b[16];uint16_t color=C_HI;
 cv_begin(240,34,UI_BG);cv_rect(0,0,240,2,color);z_text_clip(8,5,&FONT_XS,title,UI_TEXT,150);z_text_clip(8,19,&FONT_XS,subtitle,z_song.recording?RGB(255,111,145):UI_MUTED,150);
 number(z_sys.midi_mode==2?daw_bpm:z_song.bpm,b);cv_text(201-text_w(&FONT_M,b)/2,1,&FONT_M,b,UI_TEXT);cv_text(201-text_w(&FONT_XS,"BPM")/2,23,&FONT_XS,"BPM",UI_MUTED);cv_blit(0,0);
}
static void ui_tracks(void){uint32_t i;char b[4];cv_begin(240,24,UI_BG);
 for(i=0;i<6;i++){int x=6+i*38,sel=i==z_song.selected;cv_rect(x,2,34,18,sel?ui_colors[i]:UI_CARD);cv_rect(x,2,34,2,ui_colors[i]);number(i+1,b);cv_text(x+4,5,&FONT_XS,b,sel?UI_BG:UI_TEXT);if(z_get(i,Z_MUTE))cv_text(x+19,5,&FONT_XS,"M",sel?UI_BG:UI_MUTED);else if(z_solo_state()==i+1)cv_text(x+19,5,&FONT_XS,"S",sel?UI_BG:ui_colors[i]);}cv_blit(0,34);
}
static void ui_footer(const char *hint){cv_begin(240,18,UI_BG);if((int32_t)(message_until-plat_ms())>0)z_text_clip(7,2,&FONT_XS,message,UI_TEXT,226);else z_text_clip(7,2,&FONT_XS,hint,UI_MUTED,226);cv_blit(0,222);}
static void ui_transition(void){uint32_t age=plat_ms()-ui_view_since;
 cv_begin(240,2,UI_GRID);if(age<220)cv_rect(0,0,(220-age)*240/220,2,C_HI);else if(z_playing())cv_rect((z_position()%16)*15,0,15,2,C_HI);cv_blit(0,238);
}
static void ui_note_name(uint32_t note,char *b){static const char *const names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};const char *p=names[note%12];uint32_t n=0;while(*p)b[n++]=*p++;number((int)note/12-1,b+n);}
static void ui_cell(int x,int y,const char *name,const char *value,int amount){
 uint32_t age=plat_ms()-ui_view_since,offset=age<220?(220-age)*4/220:0;cv_rect(x,0,110,40,UI_CARD);z_text_clip(x+6,3,&FONT_XS,name,UI_MUTED,98);z_text_clip(x+6,17+offset,&FONT_S,value,C_HI,98);cv_rect(x+6,36,98,2,UI_GRID);if(amount>0)cv_rect(x+6,36,clamp(amount,0,127)*98/127,2,C_HI);(void)y;
}
static void ui_draw_home(void){uint32_t t=z_song.selected,pg=z_song.subpage%page_count[z_song.section],i;char b[32];
 ui_header(z_song.section==Z_HOME?Z_EDITION_TITLE:(z_song.language?section_en[z_song.section]:section_ru[z_song.section]),z_song.recording?word("ЗАПИСЬ / REC","RECORDING"):z_playing()?word("ИГРАЕТ / USB MIDI","PLAYING / USB MIDI"):"STANDALONE / USB MIDI");ui_tracks();
 cv_begin(240,42,UI_BG);z_text_clip(8,3,&FONT_S,z_drum_track(t)?"DRUM KIT / 16 SOUNDS":z_song.preset[t]==255?word("СВОЙ ЗВУК","CUSTOM SOUND"):z_preset_name(z_song.preset[t]),UI_TEXT,224);cv_text(8,25,&FONT_XS,z_drum_track(t)?"GM / CH 6":z_models[z_get(t,Z_MODEL)],C_HI);cv_text(132,25,&FONT_XS,"OCT",UI_MUTED);number(z_song.octave,b);cv_text(158,25,&FONT_XS,b,UI_TEXT);cv_text(185,25,&FONT_XS,z_song.arp_mode[t]?(z_song.arp_hold[t]?"HOLD":"ARP"):"",C_HI);cv_blit(0,58);
 cv_begin(240,16,UI_BG);cv_text(8,1,&FONT_XS,"OUT",UI_MUTED);cv_rect(35,5,158,5,UI_GRID);cv_rect(35,5,ui_meter*158/127,5,C_HI);number(z_voice_count(),b);cv_text(205,1,&FONT_XS,b,UI_TEXT);cv_text(213,1,&FONT_XS,"/8",UI_MUTED);cv_blit(0,100);
 for(i=0;i<2;i++){uint32_t j;cv_begin(240,44,UI_BG);for(j=0;j<2;j++){uint32_t id=pages[z_song.section][pg][i*2+j];int v=id<Z_PARAMS?z_get(t,id):0,amount=id<Z_PARAMS?(v-z_desc[id].min)*127/(z_desc[id].max-z_desc[id].min):0;ui_cell(6+j*118,0,label(id),value_text(id,b),amount);}cv_blit(0,116+i*44);}
 cv_begin(240,18,UI_BG);for(i=0;i<16;i++){uint32_t at=z_song.step_bank*16+i;cv_rect(8+i*14,4,10,8,z_song.track[t].steps[at].velocity?C_HI:UI_CARD);if(z_playing()&&at==z_position()%z_get(t,Z_LENGTH))cv_rect(8+i*14,1,10,2,UI_TEXT);}cv_blit(0,204);ui_footer(word("SEQ: ПИАНОРОЛЛ  HOLD: МЕНЮ","SEQ: PIANO ROLL  HOLD: MENU"));
}
static void ui_roll_note(uint32_t at,uint32_t note,uint32_t base,uint32_t selected,uint32_t duration){int row=(int)note-(int)base,x=24+(int)(at%16)*13,w=duration?duration*13-2:10;if(row<0||row>12)return;if(x+w>232)w=232-x;cv_rect(x,2+(12-row)*8,w,6,at==selected?UI_TEXT:C_HI);if(at==selected)cv_rect(x,2+(12-row)*8,2,6,UI_BG);}
static void ui_draw_drum_roll(void){uint32_t t=5,selected=z_song.step_edit,bank=selected/16,group=z_drum_index(z_song.track[5].steps[selected].note)/8,i,j;char b[24];
 ui_header(word("ДРАМ-КИТ / 6","DRUM KIT / 6"),z_song.recording?"REC / GM MIDI":"16 SOUNDS / WHITE KEYS");ui_tracks();
 cv_begin(240,20,UI_BG);z_text_clip(6,2,&FONT_XS,z_drum_name(z_song.track[t].steps[selected].note),C_HI,95);for(i=0;i<4;i++){cv_rect(137+i*23,2,19,14,i==bank?C_HI:UI_CARD);number(i+1,b);cv_text(143+i*23,3,&FONT_XS,b,i==bank?UI_BG:UI_MUTED);}cv_blit(0,58);
 cv_begin(240,108,UI_BG);for(i=0;i<8;i++){z_text_clip(2,i*13+2,&FONT_XS,z_drum_name(z_drum_note(group*8+i)),UI_MUTED,68);cv_rect(72,i*13+12,163,1,UI_GRID);}
 for(i=0;i<=16;i++)cv_rect(72+i*10,0,1,105,i%4?UI_GRID:RGB(55,76,98));
 for(i=0;i<16;i++){uint32_t at=bank*16+i;z_step *s=&z_song.track[t].steps[at];if(!s->velocity)continue;for(j=0;j<4;j++){uint32_t n=j?z_song.chord[t][at][j-1]:s->note+1;if(n){uint32_t d=z_drum_index(n-1);if(d/8==group)cv_rect(74+i*10,2+(d%8)*13,7,9,at==selected?UI_TEXT:C_HI);}}}
 {uint32_t head=z_position()%z_get(t,Z_LENGTH);if(z_playing()&&head/16==bank)cv_rect(72+(head%16)*10,0,1,106,RGB(255,130,163));}cv_blit(0,78);
 cv_begin(240,36,UI_BG);for(i=0;i<4;i++){uint32_t id=pages[Z_SEQ][z_song.subpage%3][i];z_text_clip(6+i*58,2,&FONT_XS,label(id),UI_MUTED,54);z_text_clip(6+i*58,17,&FONT_XS,value_text(id,b),C_HI,54);}cv_blit(0,186);ui_footer(word("K2: ЗВУК  SEQ: СТРАНИЦА","K2: SOUND  SEQ: PAGE"));
}
static void ui_draw_roll(void){if(z_drum_track(z_song.selected)){ui_draw_drum_roll();return;}uint32_t t=z_song.selected,selected=z_song.step_edit,bank=selected/16,base,i,j;char b[28],value[20];z_step *focus=&z_song.track[t].steps[selected];
 base=focus->note/12*12;base=base>3?base-3:0;if(base>115)base=115;
 ui_header(word("ПИАНОРОЛЛ","PIANO ROLL"),z_song.recording?word("ЗАПИСЬ / АККОРДЫ","RECORD / CHORDS"):word("4 НОТЫ / 64 ШАГА","4 NOTES / 64 STEPS"));ui_tracks();
 cv_begin(240,20,UI_BG);number(selected+1,b);z_text(8,2,&FONT_XS,word("ШАГ","STEP"),UI_MUTED);cv_text(40,2,&FONT_XS,b,UI_TEXT);cv_text(58,2,&FONT_XS,"/64",UI_MUTED);for(i=0;i<4;i++){cv_rect(137+i*23,2,19,14,i==bank?C_HI:UI_CARD);number(i+1,b);cv_text(143+i*23,3,&FONT_XS,b,i==bank?UI_BG:UI_MUTED);}cv_blit(0,58);
 cv_begin(240,108,UI_BG);
 for(i=0;i<13;i++){uint32_t note=base+12-i;int black=note%12==1||note%12==3||note%12==6||note%12==8||note%12==10;cv_rect(2,1+i*8,19,7,black?UI_CARD:RGB(111,131,149));cv_rect(23,8+i*8,209,1,UI_GRID);}
 for(i=0;i<=16;i++)cv_rect(23+i*13,0,1,105,i%4==0?RGB(55,76,98):UI_GRID);
 cv_rect(24+(selected%16)*13,0,12,1,UI_TEXT);
 for(i=0;i<16;i++){uint32_t at=bank*16+i;z_step *s=&z_song.track[t].steps[at];if(!s->velocity)continue;ui_roll_note(at,s->note,base,selected,z_song.duration[t][at]);for(j=0;j<3;j++)if(z_song.chord[t][at][j])ui_roll_note(at,z_song.chord[t][at][j]-1,base,selected,z_song.duration[t][at]);}
 {uint32_t head=z_position()%z_get(t,Z_LENGTH);if(z_playing()&&head/16==bank)cv_rect(23+(head%16)*13,0,1,106,RGB(255,130,163));}cv_blit(0,78);
 cv_begin(240,36,UI_BG);
 for(i=0;i<4;i++){uint32_t id=pages[Z_SEQ][z_song.subpage%3][i];z_text_clip(6+i*58,2,&FONT_XS,label(id),UI_MUTED,54);if(id==U_NOTE){ui_note_name(focus->note,value);}else{const char *v=value_text(id,b);uint32_t k=0;while(v[k]&&k<19){value[k]=v[k];k++;}value[k]=0;}z_text_clip(6+i*58,17,&FONT_XS,value,C_HI,54);}cv_blit(0,186);ui_footer(word("SEQ: СТР.  PRESET: БАНК","SEQ: PAGE  PRESET: BANK"));
}
static void edition_splash(void){uint32_t y,i;uint16_t color=Z_SAMPLER_EDITION?RGB(191,153,255):RGB(112,235,198);
 for(y=0;y<240;y+=40){cv_begin(240,40,UI_BG);for(i=0;i<6;i++)cv_rect(8+i*39,(y/40+i)%3*9,28,4,ui_colors[i]);if(y==80)cv_text(38,6,&FONT_M,"ZOMBIE",UI_TEXT);if(y==120)cv_text(25,2,&FONT_S,Z_SAMPLER_EDITION?"SAMPLER":"GROOVEBOX",color);if(y==160)cv_text(58,5,&FONT_XS,Z_EDITION_VERSION " / FM-1",UI_MUTED);cv_blit(0,y);}lcd_sync();
}

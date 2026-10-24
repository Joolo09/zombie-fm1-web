/* SPDX-License-Identifier: MIT — original 240x240 pad and waveform view. */
static int sampler_open(void){return Z_SAMPLER_EDITION&&z_sys.midi_mode!=2&&(z_song.section==Z_HOME||z_song.section==Z_EDIT);}
static void sampler_knob(uint32_t k,int delta){uint32_t index=z_sampler.selected;z_pad *p=&z_sampler.pad[index];
 if(!sampler_page){
  if(k==0){z_sampler.selected=z_sampler.bank*16+clamp((int)(index%16)+delta,0,15);z_touch();}
  else if(k==1)z_sampler_set(index,10,clamp((int)z_sampler.bank+delta,0,1));
  else if(k==2)z_sampler_set(index,4,clamp((int)p->gain+delta,0,127));
  else z_sampler_set(index,2,clamp((int)p->source+delta,0,13));
 }else if(sampler_page==1){
  if(k==0)z_sampler_set(index,0,clamp((int)p->start+delta*256,0,(int)p->end-1));
  else if(k==1)z_sampler_set(index,1,clamp((int)p->end+delta*256,(int)p->start+1,65535));
  else if(k==2)z_sampler_set(index,3,clamp((int)p->tune+delta,-24,24));
  else z_sampler_set(index,4,clamp((int)p->gain+delta,0,127));
 }else{
  if(k==0)z_sampler_set(index,6,clamp((int)p->mode+delta,0,2));
  else if(k==1)z_sampler_set(index,7,delta>0);
  else if(k==2)z_sampler_set(index,8,clamp((int)p->choke+delta,0,8));
  else z_sampler_set(index,9,clamp((int)p->quality+delta,0,2));
 }
}
static void sampler_black(uint32_t j){uint32_t pad=z_sampler.selected;
 if(j<2)z_sampler_set(pad,10,j);
 else if(j==2)z_sampler_set(pad,7,!z_sampler.pad[pad].reverse);
 else if(j==3)z_sampler_set(pad,6,(z_sampler.pad[pad].mode+1)%3);
 else if(j<7)z_sampler_chop(pad,j==4?4:j==5?8:16);
 else if(j==7)app_panic();
}
/* Labels and values occupy separate rows in four fixed 58-pixel columns. */
static void sampler_cell(uint32_t column,const char *name,const char *value){int x=6+column*58;
 z_text_clip(x,1,&FONT_XS,name,C_GRAY,52);z_text_clip(x,15,&FONT_XS,value,C_HI,52);
}
static void draw_sampler(void){uint32_t i,j,index=z_sampler.selected,now=plat_ms();z_pad *p=&z_sampler.pad[index];char b[20];static const char *const sources[]={"MC ZOMBIE","MC HURT","MC DEATH","MC CREEPER","MC BOOM","MC XP","USR1","USR2","USR3","USR4","KICK","SNARE","HAT","CLAP"};
 ui_header("ZOMBIE SAMPLER",z_song.recording?word("ЗАПИСЬ ПЭДОВ","RECORDING PADS"):"32 PADS / USB MIDI");
 cv_begin(240,22,UI_BG);b[0]=index<16?'A':'B';number(index%16+1,b+1);cv_text(7,3,&FONT_S,b,UI_TEXT);z_text_clip(44,4,&FONT_XS,sources[p->source],C_HI,132);cv_text(182,4,&FONT_XS,z_sampler.bank?"BANK B":"BANK A",UI_MUTED);cv_blit(0,34);
 if(ui_wave_source!=p->source||now-ui_wave_at>300){for(i=0;i<96;i++)ui_wave[i]=z_sample_peak(p->source,i,96)*17/32768;ui_wave_source=p->source;ui_wave_at=now;}
 cv_begin(240,40,UI_CARD);cv_rect(8,19,224,1,UI_GRID);for(i=0;i<96;i++){int h=ui_wave[i];cv_rect(8+i*224/96,19-h,2,h*2+1,C_HI);}cv_rect(8+(uint32_t)p->start*222/65535,1,1,37,UI_TEXT);cv_rect(8+(uint32_t)p->end*222/65535,1,1,37,RGB(255,164,108));cv_blit(0,56);
 for(i=0;i<4;i++){cv_begin(240,23,UI_BG);for(j=0;j<4;j++){uint32_t pad=z_sampler.bank*16+i*4+j,x=6+j*58;uint16_t color=ui_colors[(pad%16)%6];int lit=(int32_t)(ui_pad_until[pad]-now)>0,sel=pad==index;b[0]=z_sampler.bank?'B':'A';number(pad%16+1,b+1);cv_rect(x,1,54,20,lit?ui_blend(UI_CARD,color,(ui_pad_until[pad]-now)*127/180):UI_CARD);cv_rect(x,1,3,20,color);if(sel){cv_rect(x,1,54,1,UI_TEXT);cv_rect(x,20,54,1,UI_TEXT);}cv_text(x+(54-text_w(&FONT_XS,b))/2,5,&FONT_XS,b,lit?UI_BG:sel?UI_TEXT:UI_MUTED);}cv_blit(0,96+i*23);}
 cv_begin(240,34,UI_BG);
 if(!sampler_page){b[0]=index<16?'A':'B';number(index%16+1,b+1);sampler_cell(0,word("K1 ПЭД","K1 PAD"),b);sampler_cell(1,word("K2 БАНК","K2 BANK"),z_sampler.bank?"B":"A");number(p->gain,b);sampler_cell(2,word("K3 УРОВ.","K3 GAIN"),b);sampler_cell(3,word("K4 ИСТ.","K4 SRC"),sources[p->source]);}
 else if(sampler_page==1){number((uint32_t)p->start*100/65535,b);sampler_cell(0,word("K1 СТАРТ","K1 START"),b);number((uint32_t)p->end*100/65535,b);sampler_cell(1,word("K2 КОНЕЦ","K2 END"),b);number(p->tune,b);sampler_cell(2,word("K3 СТРОЙ","K3 TUNE"),b);number(p->gain,b);sampler_cell(3,word("K4 УРОВ.","K4 GAIN"),b);}
 else{static const char *const modes[]={"SHOT","GATE","LOOP"};sampler_cell(0,word("K1 РЕЖИМ","K1 MODE"),modes[p->mode]);sampler_cell(1,word("K2 НАПР.","K2 DIR"),p->reverse?"REV":"FWD");number(p->choke,b);sampler_cell(2,word("K3 ГРУПП","K3 CHOKE"),b);sampler_cell(3,"K4 BITS",p->quality==0?"16":p->quality==1?"12":"8");}cv_blit(0,188);
 ui_footer(word("PRESET: СТР. / ЧЕРНЫЕ: БАНК","PRESET: PAGE / BLACK: BANK"));
}

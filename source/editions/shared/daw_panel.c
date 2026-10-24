/* SPDX-License-Identifier: MIT — original DAW controller screen, not a DAW UI copy. */
static void draw_daw(void){uint32_t i,t=z_song.selected;char buf[48];
 cv_begin(240,34,C_BLACK);cv_text(8,1,&FONT_XS,daw_host==2?"ABLETON LIVE / MIDI":daw_host==1?"FL STUDIO / MIDI":"DAW / MIDI",C_HI);
 z_text(8,17,&FONT_XS,!daw_live()?word("ЖДУ СКРИПТ DAW","WAIT DAW SCRIPT"):daw_recording?"REC":daw_playing?"PLAY":"STOP",C_GRAY);
 number(daw_bpm,buf);cv_text(202-text_w(&FONT_M,buf)/2,0,&FONT_M,buf,C_WHITE);cv_text(202-text_w(&FONT_XS,"BPM")/2,22,&FONT_XS,"BPM",C_GRAY);cv_blit(0,0);
 cv_begin(240,20,C_BLACK);for(i=0;i<6;i++){uint32_t x=6+i*39;cv_rect(x,1,34,16,i==t?C_HI:C_LINE);number(i+1,buf);cv_text(x+12,2,&FONT_XS,buf,i==t?C_BLACK:C_GRAY);}cv_blit(0,34);
 cv_begin(240,42,C_BLACK);z_text_clip(8,4,&FONT_S,daw_live()&&daw_name[0]?daw_name:word("РЕЖИМ КОНТРОЛЛЕРА","CONTROLLER MODE"),C_WHITE,224);
 cv_text(8,26,&FONT_XS,"USB MIDI / CH",C_GRAY);number(z_get(t,Z_CHANNEL),buf);cv_text(92,26,&FONT_XS,buf,C_HI);cv_blit(0,54);
 for(i=0;i<2;i++){uint32_t j;cv_begin(240,44,C_BLACK);for(j=0;j<2;j++){uint32_t k=i*2+j,x=8+j*120;int v=daw_values[t][k];
  buf[0]='K';buf[1]=(char)('1'+k);buf[2]=0;cv_text(x,0,&FONT_XS,buf,C_GRAY);cv_text(x+22,0,&FONT_XS,"CC",C_GRAY);number(daw_cc[k],buf);cv_text(x+40,0,&FONT_XS,buf,C_GRAY);
  number(v,buf);cv_text(x,14,&FONT_S,buf,C_HI);cv_rect(x,36,104,2,C_LINE);cv_rect(x,36,v*104/127,2,C_HI);
 }cv_blit(0,96+i*44);}
 cv_begin(240,30,C_BLACK);cv_text(8,0,&FONT_XS,"VOLUME",C_GRAY);number(daw_values[t][4],buf);cv_text(62,0,&FONT_XS,buf,C_HI);cv_text(128,0,&FONT_XS,"PAN",C_GRAY);number((int)daw_values[t][5]-64,buf);cv_text(164,0,&FONT_XS,buf,C_HI);
 cv_text(8,16,&FONT_XS,"ALGO: CH  SELECT: BPM  PRESET: PAN",C_GRAY);cv_blit(0,184);
 cv_begin(240,26,C_BLACK);z_text(8,2,&FONT_XS,word("ВНУТРЕННИЙ ЗВУК ОТКЛЮЧЕН","LOCAL AUDIO IS OFF"),C_GRAY);z_text(8,15,&FONT_XS,word("HOME+PLAY: СИНТЕЗ / ТЕСТ","HOME+PLAY: SYNTH / TEST"),C_HI);cv_blit(0,214);
}

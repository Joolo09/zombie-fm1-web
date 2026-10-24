/* SPDX-License-Identifier: MIT
 * Device navigation, UTF-8 Cyrillic text and every printed panel button. */
enum {U_TRACK=Z_PARAMS,U_PRESET,U_OCT,U_BPM,U_SWING,U_CLOCK,U_LANGUAGE,U_BANK,
      U_ARP_MODE,U_ARP_RATE,U_ARP_OCT,U_ARP_HOLD,U_STEP,U_NOTE,U_VELOCITY,U_GATE,U_CHANCE,U_REC_MODE,U_REC_QUANT,U_CLICK,U_FX_AMOUNT,U_FX_RATE,U_CORE,U_TRIM,U_LIMITER,U_GLOW,U_CURVE,U_START,U_MIDI_MODE,U_DAW_TRANSPORT,U_DURATION,U_CHORD1,U_CHORD2,U_CHORD3,U_PAD_SOURCE,U_PAD_TUNE,U_PAD_GAIN,U_PAD_MODE};
static const char *word(const char *ru,const char *en){return z_song.language?en:ru;}
static const uint8_t section_button[Z_SECTIONS]={B_HOME,B_FX,B_SEL,B_ENV,B_LFO,B_EDIT,B_GLO,B_ARP,B_SEQ};
static uint8_t sampler_page;
enum {SECTION_HOLD_MS=450};
static const uint8_t page_count[Z_SECTIONS]={2,3,2,2,2,Z_SAMPLER_EDITION?4:3,6,2,3};
static const uint8_t pages[Z_SECTIONS][6][4]={
 {{U_PRESET,Z_CUTOFF,Z_RESONANCE,Z_LEVEL},{U_TRACK,U_OCT,U_LANGUAGE,Z_CHANNEL}},
 {{Z_DRIVE,Z_DELAY,Z_REVERB,Z_LEVEL},{U_FX_AMOUNT,U_FX_RATE,U_BPM,U_CLICK},{Z_CUTOFF,Z_RESONANCE,Z_PAN,Z_LEVEL}},
 {{U_TRACK,U_PRESET,Z_MUTE,Z_PAN},{Z_CHANNEL,U_OCT,Z_TRANSPOSE,Z_LEVEL}},
 {{Z_ATTACK,Z_DECAY,Z_SUSTAIN,Z_RELEASE},{Z_GATE,Z_LENGTH,U_REC_MODE,U_REC_QUANT}},
 {{Z_LFO_RATE,Z_LFO_DEPTH,Z_FM,Z_RATIO},{Z_CUTOFF,Z_RESONANCE,Z_MIX,Z_DETUNE}},
 {{Z_MODEL,Z_WAVE,Z_MIX,Z_DETUNE},{Z_FM,Z_RATIO,Z_SAMPLER_EDITION?Z_SAMPLE:Z_LFO_RATE,Z_SAMPLER_EDITION?Z_ROOT:Z_LFO_DEPTH},{Z_CUTOFF,Z_RESONANCE,Z_TRANSPOSE,Z_PROBABILITY}},
 {{U_BPM,U_SWING,U_CLOCK,U_LANGUAGE},{U_OCT,Z_CHANNEL,Z_LEVEL,Z_PAN},{U_REC_MODE,U_REC_QUANT,U_CLICK,Z_LENGTH},{U_CORE,U_TRIM,U_LIMITER,U_GLOW},{U_CURVE,U_START,U_LANGUAGE,U_BPM},{U_MIDI_MODE,U_DAW_TRANSPORT,U_TRACK,Z_CHANNEL}},
 {{U_ARP_MODE,U_ARP_RATE,U_ARP_OCT,U_ARP_HOLD},{U_BPM,U_SWING,Z_GATE,Z_PROBABILITY}},
 {{U_STEP,U_NOTE,U_VELOCITY,U_DURATION},{U_CHORD1,U_CHORD2,U_CHORD3,U_CHANCE},{U_BANK,Z_LENGTH,U_REC_MODE,U_REC_QUANT}}
};
static const char *const section_ru[Z_SECTIONS]={"ГЛАВНАЯ","ЭФФЕКТЫ","ДОРОЖКА","ОГИБАЮЩАЯ","МОДУЛЯЦИЯ","СИНТЕЗ","НАСТРОЙКИ","АРПЕДЖИАТОР","ШАГИ"};
static const char *const section_en[Z_SECTIONS]={"HOME","FX","TRACK","ENVELOPE","LFO","SYNTH","SETTINGS","ARPEGGIATOR","STEPS"};
static const char *const param_ru[Z_PARAMS]={"ДВИЖОК","ФОРМА","СМЕСЬ","РАССТРОЙКА","СРЕЗ","РЕЗОНАНС","АТАКА","СПАД","УДЕРЖАНИЕ","ЗАТУХАНИЕ","ГЛУБИНА FM","FM ЧАСТОТА","ПЕРЕГРУЗ","ДИЛЕЙ","РЕВЕРБ","УРОВЕНЬ","ПАНОРАМА","LFO ЧАСТОТА","LFO ГЛУБИНА","СЭМПЛ","ОСНОВНАЯ НОТА","БЕЗ ЗВУКА","MIDI КАНАЛ","ДЛИНА","ДЛИНА НОТЫ","ТРАНСПОЗИЦИЯ","ВЕРОЯТНОСТЬ"};
static const char *const special_ru[]={"ДОРОЖКА","ПРЕСЕТ","ОКТАВА","ТЕМП","СВИНГ","СИНХРОНИЗАЦИЯ","ЯЗЫК","СТРАНИЦА ШАГОВ","РЕЖИМ","СКОРОСТЬ","ОКТАВЫ","ЗАЩЕЛКА","ШАГ","НОТА","СИЛА","ДЛИНА НОТЫ","ВЕРОЯТНОСТЬ","ЗАПИСЬ","КВАНТИЗАЦИЯ","МЕТРОНОМ","СИЛА FX","РИТМ FX","CPU1 / ЭКСП.","ВЫХОД %","ЛИМИТЕР","ПОДСВЕТКА","КЛАВИШИ","ПРИ ЗАПУСКЕ","MIDI РЕЖИМ","DAW ТРАНСПОРТ","ДЛИНА","АККОРД 1","АККОРД 2","АККОРД 3","ИСТОЧНИК","СТРОЙ ПЭДА","УРОВЕНЬ","РЕЖИМ ПЭДА"};
static const char *const special_en[]={"TRACK","PRESET","OCTAVE","BPM","SWING","CLOCK","LANGUAGE","STEP PAGE","MODE","RATE","OCTAVES","HOLD","STEP","NOTE","VELOCITY","GATE","CHANCE","RECORD MODE","QUANTIZE","METRONOME","FX AMOUNT","FX RATE","CPU1 / EXP.","OUTPUT %","LIMITER","KEY GLOW","VELOCITY","START PAGE","MIDI MODE","DAW TRANSPORT","LENGTH","CHORD 1","CHORD 2","CHORD 3","SOURCE","PAD TUNE","PAD GAIN","PAD MODE"};
static void number(int n,char *s){char tmp[12];int i=0;if(n<0){*s++='-';n=-n;}do{tmp[i++]=(char)('0'+n%10);n/=10;}while(n);while(i)*s++=tmp[--i];*s=0;}
/* Platform text is byte-oriented. Decode UTF-8 here and add the Cyrillic glyphs,
 * without changing the shared driver or replacing it with question marks. */
static int z_text(int x,int y,const felucca_font_t *font,const char *s,uint16_t color){
 int large=font->h>=16,cw=large?8:6,height=large?16:12;
 while(*s){uint32_t ch=(uint8_t)*s++;
  if(ch>=0xc0&&ch<0xe0&&((uint8_t)*s&0xc0)==0x80){ch=(ch&31)<<6|((uint8_t)*s++&63);}
  else if(ch>=0xe0){if(*s)s++;if(*s)s++;ch='?';}
  if(ch>=0x400&&ch<0x460){const uint8_t *rows=large?Z_CYR16[ch-0x400]:Z_CYR12[ch-0x400];int gx,gy;
   for(gy=0;gy<height;gy++)for(gx=0;gx<cw;gx++)if(rows[gy]>>(cw-1-gx)&1)cv_rect(x+gx,y+gy,1,1,color);x+=cw;
  }else{char a[2]={(char)(ch<128?ch:'?'),0};x=cv_text(x,y,font,a,color);}
 }return x;
}
static int panel_setting(uint32_t id,uint32_t value){
 if(id==0){if(value>6)return 0;z_song.octave=(int8_t)((int)value-3);}
 else if(id==1){if(value>=Z_SECTIONS)return 0;z_song.section=(uint8_t)value;z_song.subpage=value==Z_GLO&&z_sys.midi_mode?5:0;}
 else if(id==2){if(value>=page_count[z_song.section])return 0;z_song.subpage=(uint8_t)value;}
 else if(id==3){if(value>3)return 0;z_song.step_bank=(uint8_t)value;z_song.step_edit=(uint8_t)(value*16+z_song.step_edit%16);}
 else if(id==4){if(value>1)return 0;z_record(value);}
 else if(id==5){if(value>63)return 0;z_song.step_edit=(uint8_t)value;z_song.step_bank=(uint8_t)(value/16);}
 else if(id==6){if(value>1)return 0;z_song.language=(uint8_t)value;}
 else if(id==7){if(value>=6)return 0;z_song.selected=(uint8_t)value;if(z_sys.midi_mode==2)midi_cc(15,110,value);}
 else if(id==8){if(value>1)return 0;z_external(value);}
 else if(id==9){if(value>1)return 0;z_song.rec_mode=value;}
 else if(id==10){if(value>1)return 0;z_song.rec_quant=value;}
 else if(id==11){if(value>1)return 0;z_song.metronome=value;}
 else if(id==12){if(value>127)return 0;z_song.fx_amount=value;}
 else if(id==13){if(value>3)return 0;z_song.fx_rate=value;}
 else return 0;
 z_touch();return 1;
}
static const char *label(uint32_t id){if(id<Z_PARAMS)return z_song.language?z_desc[id].name:param_ru[id];return z_song.language?special_en[id-Z_PARAMS]:special_ru[id-Z_PARAMS];}
static const char *value_text(uint32_t id,char *str){uint32_t t=z_song.selected;int value=0;z_step *step=&z_song.track[t].steps[z_song.step_edit];
 if(id<Z_PARAMS){value=z_get(t,id);
  if(id==Z_MODEL)return z_drum_track(t)?"DRUM KIT":z_models[value];
  if(id==Z_WAVE){static const char *const ru[]={"СИНУС","ПИЛА","ИМПУЛЬС","ТРЕУГОЛЬНИК"},*const en[]={"SINE","SAW","PULSE","TRIANGLE"};return z_song.language?en[value]:ru[value];}
  if(id==Z_MUTE)return value?word("ВКЛ","ON"):word("ВЫКЛ","OFF");
  if(id==Z_SAMPLE){static const char *const name[]={"MC ZOMBIE","MC HURT","MC DEATH","MC CREEPER","MC BOOM","MC XP","USR1","USR2","USR3","USR4","ZC KICK","ZC SNARE","ZC HAT","ZC CLAP"};return name[value];}
 }else switch(id){
  case U_PAD_SOURCE:value=z_sampler.pad[z_sampler.selected].source;break;
  case U_PAD_TUNE:value=z_sampler.pad[z_sampler.selected].tune;break;
  case U_PAD_GAIN:value=z_sampler.pad[z_sampler.selected].gain;break;
  case U_PAD_MODE:return z_sampler.pad[z_sampler.selected].mode==0?"SHOT":z_sampler.pad[z_sampler.selected].mode==1?"GATE":"LOOP";
  case U_DURATION:value=z_song.duration[t][z_song.step_edit];if(!value)return "AUTO";break;
  case U_CHORD1:case U_CHORD2:case U_CHORD3:value=z_song.chord[t][z_song.step_edit][id-U_CHORD1];if(!value)return "OFF";value--;break;
  case U_MIDI_MODE:return z_sys.midi_mode==2?"DAW CONTROL":z_sys.midi_mode==1?"DAW SYNTH":"STANDALONE";
  case U_DAW_TRANSPORT:return z_sys.transport?word("ВКЛ","ON"):word("ВЫКЛ","OFF");
  case U_CORE:{static const char *const a[]={"CPU0","CPU1 ON","START FAIL","TEST FAIL","TIMEOUT","SIMULATED"};return core_pending?"WAIT...":a[core_status];}
  case U_TRIM:value=z_sys.trim;break;
  case U_LIMITER:return z_sys.limiter?word("ВКЛ","ON"):word("ВЫКЛ","OFF");
  case U_GLOW:return z_sys.glow==0?word("ВЫКЛ","OFF"):z_sys.glow==1?word("ИГРА","PLAY"):word("ВСЕ","ALL");
  case U_CURVE:return z_sys.velocity==0?word("ОБЫЧНО","NORMAL"):z_sys.velocity==1?word("ЛЕГКО","LIGHT"):"FIXED 100";
  case U_START:return z_sys.start==0?"HOME":z_sys.start==1?"SEQ":"SEL";
  case U_REC_MODE:return z_song.rec_mode?word("ЗАМЕНА","REPLACE"):word("НАЛОЖЕНИЕ","OVERDUB");
  case U_REC_QUANT:return z_song.rec_quant?word("ВНИЗ 1/16","FLOOR 1/16"):word("БЛИЖ. 1/16","NEAREST 1/16");
  case U_CLICK:return z_song.metronome?word("ВКЛ","ON"):word("ВЫКЛ","OFF");
  case U_FX_AMOUNT:value=z_song.fx_amount;break;
  case U_FX_RATE:{static const char *const rate[]={"1/4","1/8","1/16","1/32"};return rate[z_song.fx_rate];}
  case U_TRACK:value=t+1;break;
  case U_PRESET:{if(z_drum_track(t))return "DRUM KIT";if(z_song.preset[t]==255)return word("ИЗМЕНЕН","EDITED");value=z_song.preset[t]+1;break;}
  case U_OCT:value=z_song.octave;break;
  case U_BPM:value=z_song.bpm;break;
  case U_SWING:value=z_song.swing;break;
  case U_CLOCK:return z_song.reserved?"USB / TRS":word("ВНУТРЕННИЙ","INTERNAL");
  case U_LANGUAGE:return z_song.language?"ENGLISH":"РУССКИЙ";
  case U_BANK:value=z_song.step_bank+1;break;
  case U_ARP_MODE:{static const char *const ru[]={"ВЫКЛ","ВВЕРХ","ВНИЗ","ТУДА-ОБРАТНО","СЛУЧАЙНО"},*const en[]={"OFF","UP","DOWN","UP/DOWN","RANDOM"};return z_song.language?en[z_song.arp_mode[t]]:ru[z_song.arp_mode[t]];}
  case U_ARP_RATE:{static const char *const rate[]={"1/8","1/16","1/32","1/8T"};return rate[z_song.arp_rate[t]];}
  case U_ARP_OCT:value=z_song.arp_oct[t];break;
  case U_ARP_HOLD:return z_song.arp_hold[t]?word("ВКЛ","ON"):word("ВЫКЛ","OFF");
  case U_STEP:value=z_song.step_edit+1;break;
  case U_NOTE:value=step->note;break;
  case U_VELOCITY:value=step->velocity;break;
  case U_GATE:value=step->gate;break;
  case U_CHANCE:value=step->chance;break;
 }
 number(value,str);return str;
}
static void knob(uint32_t id,int delta){uint32_t t=z_song.selected;int value;z_step s=z_song.track[t].steps[z_song.step_edit];
 if(id<Z_PARAMS){z_set(t,id,z_get(t,id)+delta);return;}
 switch(id){
  case U_PAD_SOURCE:z_sampler_set(z_sampler.selected,2,clamp((int)z_sampler.pad[z_sampler.selected].source+delta,0,13));break;
  case U_PAD_TUNE:z_sampler_set(z_sampler.selected,3,clamp((int)z_sampler.pad[z_sampler.selected].tune+delta,-24,24));break;
  case U_PAD_GAIN:z_sampler_set(z_sampler.selected,4,clamp((int)z_sampler.pad[z_sampler.selected].gain+delta,0,127));break;
  case U_PAD_MODE:z_sampler_set(z_sampler.selected,6,clamp((int)z_sampler.pad[z_sampler.selected].mode+delta,0,2));break;
  case U_DURATION:z_checkpoint();z_song.duration[t][z_song.step_edit]=clamp((int)z_song.duration[t][z_song.step_edit]+delta,0,63);z_touch();break;
  case U_CHORD1:case U_CHORD2:case U_CHORD3:{uint32_t at=id-U_CHORD1;int v=z_song.chord[t][z_song.step_edit][at];if(z_drum_track(t)){int idx=v?(int)z_drum_index(v-1)+1:0;idx=clamp(idx+delta,0,16);v=idx?(int)z_drum_note(idx-1)+1:0;delta=0;}else if(!v&&delta>0){static const uint8_t intervals[3]={4,7,12};v=s.note+intervals[at]+1;delta=0;}z_checkpoint();z_song.chord[t][z_song.step_edit][at]=clamp(v+delta,0,128);z_touch();break;}
  case U_MIDI_MODE:system_set(6,clamp((int)z_sys.midi_mode+delta,0,2));break;
  case U_DAW_TRANSPORT:system_set(7,delta>0);break;
  case U_CORE:system_set(0,delta>0);break;
  case U_TRIM:system_set(1,clamp((int)z_sys.trim+delta,0,127));break;
  case U_LIMITER:system_set(2,delta>0);break;
  case U_GLOW:system_set(3,clamp((int)z_sys.glow+delta,0,2));break;
  case U_CURVE:system_set(4,clamp((int)z_sys.velocity+delta,0,2));break;
  case U_START:system_set(5,clamp((int)z_sys.start+delta,0,2));break;
  case U_REC_MODE:panel_setting(9,!z_song.rec_mode);break;
  case U_REC_QUANT:panel_setting(10,!z_song.rec_quant);break;
  case U_CLICK:panel_setting(11,!z_song.metronome);break;
  case U_FX_AMOUNT:panel_setting(12,clamp(z_song.fx_amount+delta,0,127));break;
  case U_FX_RATE:panel_setting(13,clamp(z_song.fx_rate+delta,0,3));break;
  case U_TRACK:panel_setting(7,(t+6+delta%6)%6);break;
  case U_PRESET:if(z_drum_track(t)){app_say(word("ДРАМ-КИТ: 16 ЗВУКОВ","DRUM KIT: 16 SOUNDS"),0);break;}value=z_song.preset[t];value=value==255?(delta>0?0:63):(value+64+delta%64)%64;z_preset(t,value);app_say(z_preset_name(value),0);break;
  case U_OCT:panel_setting(0,clamp(z_song.octave+delta,-3,3)+3);break;
  case U_BPM:z_tempo(clamp(z_song.bpm+delta,30,300),z_song.swing);break;
  case U_SWING:z_tempo(z_song.bpm,clamp(z_song.swing+delta,0,60));break;
  case U_CLOCK:panel_setting(8,!z_song.reserved);break;
  case U_LANGUAGE:panel_setting(6,!z_song.language);break;
  case U_BANK:panel_setting(3,(z_song.step_bank+4+delta%4)%4);break;
  case U_ARP_MODE:z_arp_set(t,0,clamp(z_song.arp_mode[t]+delta,0,4));break;
  case U_ARP_RATE:z_arp_set(t,1,clamp(z_song.arp_rate[t]+delta,0,3));break;
  case U_ARP_OCT:z_arp_set(t,2,clamp(z_song.arp_oct[t]+delta,1,3));break;
  case U_ARP_HOLD:z_arp_set(t,3,!z_song.arp_hold[t]);break;
  case U_STEP:panel_setting(5,clamp(z_song.step_edit+delta,0,63));break;
  default:
   if(id==U_NOTE)s.note=(uint8_t)(z_drum_track(t)?z_drum_note(clamp((int)z_drum_index(s.note)+delta,0,15)):clamp(s.note+delta,0,127));
   else if(id==U_VELOCITY)s.velocity=(uint8_t)clamp(s.velocity+delta,0,127);
   else if(id==U_GATE)s.gate=(uint8_t)clamp(s.gate+delta,1,127);
   else if(id==U_CHANCE)s.chance=(uint8_t)clamp(s.chance+delta,0,127);
   z_checkpoint();z_song.track[t].steps[z_song.step_edit]=s;z_touch();break;
 }
}
static int active_layer=-1;
static uint8_t button_used[NB];
static uint32_t button_since[NB],layer_mask,tap_at,tap_sum,tap_count;
static uint8_t return_section,return_page,layer_page,key_layer[27];
static void panel_reset(void){uint32_t i;active_layer=-1;sampler_page=0;layer_mask=0;tap_at=tap_sum=tap_count=0;for(i=0;i<NB;i++){button_since[i]=0;button_used[i]=0;}return_section=return_page=layer_page=0;}
static const char *const fx_names[16]={"DUCK","CHOP","TREMOLO","BITS","DRIVE","MONO","SWAP","PUMP","RING","BOOST","RIGHT","LEFT","GATE","FUZZ","PHASE","CUT"};
static void tap_tempo(uint32_t now){uint32_t ms=now-tap_at;tap_at=now;if(ms<200||ms>2000){tap_sum=0;tap_count=0;return;}tap_sum+=ms;if(++tap_count>4){tap_sum=ms;tap_count=1;}z_tempo(60000u*tap_count/tap_sum,z_song.swing);}
static uint32_t layer_knob(uint32_t k){
 if(Z_SAMPLER_EDITION&&active_layer==Z_EDIT){static const uint8_t p[]={U_PAD_SOURCE,U_PAD_TUNE,U_PAD_GAIN,U_PAD_MODE};return p[k];}
 if(active_layer==Z_HOME){static const uint8_t p[]={U_BPM,U_SWING,U_CLOCK,U_LANGUAGE};return p[k];}
 if(active_layer==Z_FX){static const uint8_t p[]={U_FX_AMOUNT,U_FX_RATE,Z_DELAY,Z_REVERB};return p[k];}
 if(active_layer==Z_SECTIONS){static const uint8_t p[]={U_REC_MODE,U_REC_QUANT,U_CLICK,Z_LENGTH};return p[k];}
 return pages[active_layer][layer_page%page_count[active_layer]][k];
}
static const char *tile_name(uint32_t j,char *buf){
 if(active_layer==Z_FX)return fx_names[j];
 if(active_layer==Z_SEL||active_layer==Z_GLO){if(j<6){number(j+1,buf);return buf;}if(j<12){buf[0]='M';number(j-5,buf+1);return buf;}return j==12?"TAP":j==13?"CLICK":j==14?"UNMUTE":"PANIC";}
 if(active_layer==Z_ARP){static const char *const a[]={"OFF","UP","DOWN","UP/DN","RANDOM","1/8","1/16","1/32","1/8T","OCT 1","OCT 2","OCT 3","HOLD ON","HOLD OFF","RELEASE","TAP"};return a[j];}
 if(active_layer==Z_ENV){static const char *const a[]={"PLUCK","BASS","KEYS","PAD","BOW","ORGAN","SNAP","DRONE","A -","A +","D -","D +","S -","S +","R -","R +"};return a[j];}
 if(active_layer==Z_LFO){static const char *const a[]={"OFF","SLOW","MEDIUM","FAST","DEEP -","DEEP +","RATE -","RATE +","FM -","FM +","RATIO -","RATIO +","CUT -","CUT +","MIX -","MIX +"};return a[j];}
 if(active_layer==Z_EDIT){if(Z_SAMPLER_EDITION){static const char *const names[16]={"ZOMBIE","HURT","DEATH","CREEPER","BOOM","XP","USR1","USR2","USR3","USR4","KICK","SNARE","HAT","CLAP","REVERSE","PANIC"};return names[j];}if(j<8)return z_models[j];{static const char *const names[8]={"INIT","BASS","LEAD","PAD","DRUM","OCT -","OCT +","PANIC"};return names[j-8];}}
 if(active_layer==Z_SECTIONS){static const char *const a[]={"OVERDUB","REPLACE","NEAREST","FLOOR","CLICK ON","CLICK OFF","LEN 16","LEN 32","LEN 64","BANK 1","BANK 2","BANK 3","BANK 4","REC OFF","REC ON","PANIC"};return a[j];}
 number(z_song.step_bank*16+j+1,buf);return buf;
}
static void layer_action(uint32_t j,uint32_t on,uint32_t now){uint32_t t=z_song.selected;
 if(active_layer==Z_FX){editor_effect=0;if(on)layer_mask|=1u<<j;else layer_mask&=~(1u<<j);{uint32_t i,e=0;for(i=0;i<16;i++)if(layer_mask>>i&1)e=i+1;z_live(e);}return;}
 if(on)layer_mask|=1u<<j;else layer_mask&=~(1u<<j);
 if(!on)return;
 if(active_layer==Z_HOME){if(j==0)tap_tempo(now);else if(j==1)panel_setting(6,!z_song.language);else if(j==15)app_panic();return;}
 if(active_layer==Z_GLO||active_layer==Z_SEL){if(j<6)panel_setting(7,j);else if(j<12)z_set(j-6,Z_MUTE,!z_get(j-6,Z_MUTE));else if(j==12)tap_tempo(now);else if(j==13)panel_setting(11,!z_song.metronome);else if(j==14){for(t=0;t<6;t++)z_set(t,Z_MUTE,0);}else app_panic();return;}
 if(active_layer==Z_ARP){if(j<5)z_arp_set(t,0,j);else if(j<9)z_arp_set(t,1,j-5);else if(j<12)z_arp_set(t,2,j-8);else if(j<14)z_arp_set(t,3,j==12);else if(j==14)z_alloff(t);else tap_tempo(now);return;}
 if(active_layer==Z_ENV){if(j<8){static const uint8_t shapes[8][4]={{0,35,0,12},{1,60,75,24},{1,70,50,40},{85,70,100,90},{55,45,110,65},{0,0,127,5},{0,12,0,0},{90,100,127,110}};uint32_t k;for(k=0;k<4;k++)z_set(t,Z_ATTACK+k,shapes[j][k]);}else{uint32_t p=Z_ATTACK+(j-8)/2;z_set(t,p,z_get(t,p)+((j&1)?8:-8));}return;}
 if(active_layer==Z_LFO){if(j==0)z_set(t,Z_LFO_DEPTH,0);else if(j<4){z_set(t,Z_LFO_RATE,j==1?8:j==2?40:100);z_set(t,Z_LFO_DEPTH,64);}else{static const uint8_t p[]={Z_LFO_DEPTH,Z_LFO_RATE,Z_FM,Z_RATIO,Z_CUTOFF,Z_MIX};uint32_t p0=p[(j-4)/2];z_set(t,p0,z_get(t,p0)+((j&1)?1:-1)*(p0==Z_RATIO?1:8));}return;}
 if(active_layer==Z_EDIT){if(Z_SAMPLER_EDITION){if(j<14)z_sampler_set(z_sampler.selected,2,j);else if(j==14)z_sampler_set(z_sampler.selected,7,!z_sampler.pad[z_sampler.selected].reverse);else app_panic();}else{if(j<8)z_set(t,Z_MODEL,j);else if(j<13){static const uint8_t presets[5]={0,16,24,32,48};z_preset(t,presets[j-8]);}else if(j<15)panel_setting(0,clamp(z_song.octave+(j==13?-1:1),-3,3)+3);else app_panic();}return;}
 if(active_layer==Z_SECTIONS){if(j<2)panel_setting(9,j);else if(j<4)panel_setting(10,j-2);else if(j<6)panel_setting(11,j==4);else if(j<9)z_set(t,Z_LENGTH,j==6?16:j==7?32:64);else if(j<13)panel_setting(3,j-9);else if(j<15)panel_setting(4,j==14);else app_panic();return;}
 if(active_layer==Z_SEQ){uint32_t at=z_song.step_bank*16+j;z_step a=z_song.track[t].steps[at];panel_setting(5,at);z_step_set(t,at,a.note,a.velocity?0:100,a.chance,a.gate);}
}
static void z_text_clip(int x,int y,const felucca_font_t *font,const char *text,uint16_t color,uint32_t pixels){
 char buf[96];uint32_t n=0,glyphs=0,max=pixels/(font->h>=16?8:6);const char *p=text;
 while(*p&&glyphs<max&&n<90){uint32_t bytes=(uint8_t)*p>=0xc0?2:1;if(glyphs+1==max&&p[bytes]){buf[n++]='.';break;}while(bytes--&&*p)buf[n++]=*p++;glyphs++;}buf[n]=0;z_text(x,y,font,buf,color);
}
static int tile_lit(uint32_t at){uint32_t t=z_song.selected;
 if(layer_mask>>at&1)return 1;
 if(active_layer==Z_GLO||active_layer==Z_SEL)return at<6?at==t:at<12?z_get(at-6,Z_MUTE):at==13?z_song.metronome:0;
 if(active_layer==Z_ARP)return at<5?at==z_song.arp_mode[t]:at<9?at-5==z_song.arp_rate[t]:at<12?at-8==z_song.arp_oct[t]:at==12?z_song.arp_hold[t]:0;
 if(active_layer==Z_SEQ)return z_song.track[t].steps[z_song.step_bank*16+at].velocity!=0;
 if(active_layer==Z_SECTIONS)return at<2?at==z_song.rec_mode:at<4?at-2==z_song.rec_quant:at==4?z_song.metronome:at==14?z_song.recording:0;
 return 0;
}
static void black_action(uint32_t j){uint32_t t=z_song.selected;
 if(active_layer==Z_GLO||active_layer==Z_SEL){if(j<6)z_solo(z_solo_state()==j+1?0:j+1);return;}
 if(active_layer!=Z_SEQ)return;
 if(j<2){z_history(j);app_say(j?word("ПОВТОР","REDO"):word("ОТМЕНА","UNDO"),0);}
 else if(j<6){if(z_pattern(t,j==2?1:j==3?2:j==4?3:5,0))app_say(word("ДЛИНА УЖЕ 64","LENGTH LIMIT"),0);}
 else if(j==6)z_pattern(t,4,(t+1)%6);
 else if(j==7)z_pattern(t,0,0);
}
#include "edition_ui.inc"
static void draw_layer(void){uint32_t j;char buf[24];
 cv_begin(240,28,C_BLACK);z_text(8,2,&FONT_S,active_layer==Z_HOME?(layer_page?word("ОБО МНЕ","ABOUT ME"):word("ОБ УСТРОЙСТВЕ","ABOUT DEVICE")):active_layer==Z_SECTIONS?word("ЗАПИСЬ / REC","RECORD SETUP"):(z_song.language?section_en[active_layer]:section_ru[active_layer]),C_WHITE);cv_text(188,4,&FONT_XS,"HOLD",C_HI);cv_blit(0,0);
 if(active_layer==Z_HOME&&layer_page){
  for(j=0;j<8;j++){cv_begin(240,18,C_BLACK);if(j==0)z_text_clip(8,1,&FONT_S,z_sys.name,C_HI,224);else if(j==2||j==3){char line[64];uint32_t k=0,at=0,col=0;while(z_sys.bio[at]&&col<((j==2)?0:36)){at+=(uint8_t)z_sys.bio[at]>=0xc0?2:1;col++;}while(z_sys.bio[at]&&k<60&&col<((j==2)?36:72)){uint32_t bytes=(uint8_t)z_sys.bio[at]>=0xc0?2:1;while(bytes--)line[k++]=z_sys.bio[at++];col++;}line[k]=0;z_text_clip(8,1,&FONT_XS,line,C_WHITE,224);}
   else if(j==6)z_text(8,1,&FONT_XS,word("ИЗМЕНИ В ВЕБ-РЕДАКТОРЕ","EDIT IN WEB STUDIO"),C_GRAY);else if(j==7)cv_text(8,1,&FONT_XS,"PRESET: ABOUT DEVICE",C_GRAY);cv_blit(0,28+j*18);}
 }else if(active_layer==Z_HOME){static const char *const ru[]={"ZOMBIE FM-1","Zombie Core " Z_EDITION_VERSION,"6 ДОРОЖЕК / 8 ГОЛОСОВ","9 ДВИЖКОВ / 64 ПРЕСЕТА","44.1 kHz / USB MIDI","4 USR PCM / RU + EN","КЛ. 1: TAP / 2: ЯЗЫК","КЛ. 16: СБРОС НОТ"},*const en[]={"ZOMBIE FM-1","Zombie Core " Z_EDITION_VERSION,"6 TRACKS / 8 VOICES","9 ENGINES / 64 PRESETS","44.1 kHz / USB MIDI","4 USR PCM / RU + EN","KEY 1: TAP / 2: LANGUAGE","KEY 16: ALL NOTES OFF"};
  for(j=0;j<8;j++){cv_begin(240,18,C_BLACK);z_text(8,1,&FONT_XS,z_song.language?en[j]:ru[j],j<2?C_HI:C_GRAY);cv_blit(0,28+j*18);}
 }else for(j=0;j<4;j++){uint32_t k;cv_begin(240,36,C_BLACK);for(k=0;k<4;k++){uint32_t at=j*4+k;int lit=tile_lit(at);cv_rect(4+k*59,1,56,32,lit?C_HI:C_LINE);number(at+1,buf);cv_text(8+k*59,2,&FONT_XS,buf,lit?C_BLACK:C_GRAY);z_text_clip(8+k*59,17,&FONT_XS,tile_name(at,buf),lit?C_BLACK:C_WHITE,50);}cv_blit(0,28+j*36);}
 for(j=0;j<2;j++){uint32_t k;cv_begin(240,28,C_BLACK);for(k=0;k<2;k++){uint32_t id=layer_knob(j*2+k);z_text(6+k*120,0,&FONT_XS,id==U_REC_QUANT?word("КВАНТ. 1/16","QUANT. 1/16"):id==U_REC_MODE?word("РЕЖИМ ЗАПИСИ","REC MODE"):label(id),C_GRAY);z_text(6+k*120,13,&FONT_XS,value_text(id,buf),C_HI);}cv_blit(0,172+j*28);}
 cv_begin(240,12,C_BLACK);z_text(6,0,&FONT_XS,active_layer==Z_SEQ?"BLACK: UNDO REDO DUP < >":(active_layer==Z_GLO||active_layer==Z_SEL)?word("ЧЕРНЫЕ 1-6: СОЛО","BLACK KEYS 1-6: SOLO"):word("SELECT: ТЕМП  PRESET: СТР.","SELECT: BPM  PRESET: PAGE"),C_GRAY);cv_blit(0,228);
}
#include "daw_panel.c"
#include "sampler_panel.inc"
static void draw_ui(void){ui_prepare();
 if(active_layer>=0)draw_layer();
 else if(z_sys.midi_mode==2&&z_song.section!=Z_GLO)draw_daw();
 else if(sampler_open())draw_sampler();
 else if(!Z_SAMPLER_EDITION&&z_song.section==Z_SEQ)ui_draw_roll();
 else ui_draw_home();
 ui_transition();
}
static void input(void){uint32_t keys,buttons=plat_buttons(),k,t,now=plat_ms(),lights=0;int d;if(daw_seen&&now-daw_seen>=3500)daw_playing=daw_recording=0;touch_poll();keys=touch_keys();
 for(k=0;k<NB;k++)if((buttons>>k&1)&&!(held_buttons>>k&1)){
  uint32_t section;button_since[k]=now;button_used[k]=0;pulse_until[k]=now+180;
  if((k==B_OCTDN||k==B_OCTUP)&&(buttons>>B_EDIT&1)){button_used[B_EDIT]=1;z_history(k==B_OCTUP);app_say(k==B_OCTUP?word("ПОВТОР","REDO"):word("ОТМЕНА","UNDO"),0);}
  else if(k==B_OCTDN||k==B_OCTUP){int oct=(buttons&(1u<<B_OCTDN))&&(buttons&(1u<<B_OCTUP))?0:z_song.octave+(k==B_OCTUP?1:-1);panel_setting(0,clamp(oct,-3,3)+3);app_say(word("ОКТАВА ИЗМЕНЕНА","OCTAVE CHANGED"),0);}
  else if((k==B_PLAY&&(buttons>>B_HOME&1))||(k==B_HOME&&(buttons>>B_PLAY&1))){button_used[B_HOME]=button_used[B_PLAY]=1;app_panic();system_set(6,0);midi_local=1;active_layer=-1;panel_setting(1,Z_HOME);audio_test_phase=0;audio_test_left=22050;app_say(word("ТЕСТ 440 Гц / MASTER","440 Hz TEST / MASTER"),0);}
  else if(k==B_PLAY){if(z_sys.midi_mode==2)daw_transport(0);else{if(z_playing())all_off();z_transport(!z_playing());}}
  else if(k==B_SAVE)save_requested=1;
  else if(k==B_REC){}
  else if((k==B_SEL&&(buttons&(1u<<B_HOME)))||(k==B_HOME&&(buttons&(1u<<B_SEL)))){button_used[B_SEL]=button_used[B_HOME]=1;system_set(6,(z_sys.midi_mode+1)%3);app_say(z_sys.midi_mode==2?"DAW CONTROLLER":z_sys.midi_mode==1?"DAW SYNTH":"STANDALONE",0);}
  else if(k==B_GLO&&(buttons&(1u<<B_HOME))){button_used[B_HOME]=button_used[B_GLO]=1;panel_setting(6,!z_song.language);}
  else {(void)section;}
 }
 /* A held section is a momentary control layer. Key combinations can enter it immediately. */
 if(active_layer<0){uint32_t section;for(section=0;section<=Z_SECTIONS;section++){uint32_t b=section==Z_SECTIONS?B_REC:section_button[section];if((buttons>>b&1)&&!button_used[b]&&((now-button_since[b]>=SECTION_HOLD_MS)||(keys&~panel_keys_before))){return_section=z_song.section;return_page=z_song.subpage;button_used[b]=1;active_layer=(int)section;layer_page=section==Z_GLO&&z_sys.midi_mode?5:0;layer_mask=0;redraw_at=0;break;}}}
 if(active_layer>=0){uint32_t b=active_layer==Z_SECTIONS?B_REC:section_button[active_layer];if(!(buttons>>b&1)){z_live(0);layer_mask=0;active_layer=-1;z_song.section=return_section;z_song.subpage=return_page;redraw_at=0;z_touch();}}
 for(k=0;k<Z_SECTIONS;k++){uint32_t b=section_button[k];if(!(buttons>>b&1)&&(held_buttons>>b&1)&&!button_used[b]&&now-button_since[b]<SECTION_HOLD_MS){if(z_song.section==k)panel_setting(2,(z_song.subpage+1)%page_count[k]);else panel_setting(1,k);redraw_at=0;}}
 if(!(buttons>>B_REC&1)&&(held_buttons>>B_REC&1)&&!button_used[B_REC]&&now-button_since[B_REC]<SECTION_HOLD_MS){if(z_sys.midi_mode==2)daw_transport(1);else{panel_setting(4,!z_song.recording);if(z_song.recording)panel_setting(1,Z_SEQ);}}
 t=z_song.selected;
 for(k=0;k<27;k++){uint32_t bit=1u<<k;
  if((keys&bit)&&!(panel_keys_before&bit)){
   int note=clamp(53+(int)k+12*z_song.octave,0,127);held_notes[k]=(uint8_t)note;held_tracks[k]=(uint8_t)t;key_action[k]=0;
   if(active_layer>=0){static const uint8_t white[16]=PLAT_WHITE_KEYS;uint32_t j;key_action[k]=2;for(j=0;j<16;j++)if(white[j]==k){key_action[k]=(uint8_t)(j+3);key_layer[k]=(uint8_t)active_layer;layer_action(j,1,now);break;}if(key_action[k]==2){static const uint8_t black[11]=PLAT_BLACK_KEYS;for(j=0;j<11;j++)if(black[j]==k){black_action(j);break;}}}
   else {
    if(sampler_open()||((z_sampler.tracks>>t&1)&&z_get(t,Z_MODEL)==8)){static const uint8_t white[16]=PLAT_WHITE_KEYS,black[11]=PLAT_BLACK_KEYS;uint32_t j;
     for(j=0;j<16;j++)if(white[j]==k)break;
     if(j==16){key_action[k]=2;for(j=0;j<11;j++)if(black[j]==k){sampler_black(j);break;}continue;}
     z_sampler.selected=z_sampler.bank*16+j;note=36+z_sampler.selected;held_notes[k]=(uint8_t)note;z_touch();
    }
    if(z_drum_track(t)){static const uint8_t white[16]=PLAT_WHITE_KEYS;uint32_t j;for(j=0;j<16;j++)if(white[j]==k)break;if(j==16){key_action[k]=2;continue;}note=z_drum_note(j);held_notes[k]=(uint8_t)note;}
    uint32_t velocity=system_velocity(touch_vel(k));edition_note(note,velocity,1);key_action[k]=1;held_channels[k]=(uint8_t)(z_get(t,Z_CHANNEL)-1);held_local[k]=!z_sys.midi_mode&&midi_local;held_raw[k]=!held_local[k]||!z_song.arp_mode[t];if(held_local[k])z_key(t,note,velocity,1);if(held_raw[k]&&(z_sys.midi_mode==2||(!z_get(t,Z_MUTE)&&(!z_solo_state()||z_solo_state()==t+1))))raw_note(held_channels[k],note,velocity,1);}
  }
  if(!(keys&bit)&&(panel_keys_before&bit)&&key_action[k]>=3){if(active_layer>=0&&key_layer[k]==active_layer)layer_action(key_action[k]-3,0,now);key_action[k]=0;}
  if(!(keys&bit)&&(panel_keys_before&bit)&&key_action[k]==1){if(held_local[k])z_key(held_tracks[k],held_notes[k],0,0);if(held_raw[k])raw_note(held_channels[k],held_notes[k],0,0);key_action[k]=0;}
 }
 d=plat_enc(EN_ALGO);if(d)panel_setting(7,(z_song.selected+6+d%6)%6);t=z_song.selected;
 d=plat_enc(EN_SELECT);if(d&&active_layer<0&&z_sys.midi_mode==2&&z_song.section!=Z_GLO){daw_touch_at=now;daw_values[t][4]=(uint8_t)clamp((int)daw_values[t][4]+d,0,127);midi_cc(z_get(t,Z_CHANNEL)-1,7,daw_values[t][4]);}else if(d&&(active_layer>=0||z_song.section==Z_HOME||(!Z_SAMPLER_EDITION&&z_song.section==Z_SEQ)))knob(U_BPM,d);else if(d)panel_setting(2,(z_song.subpage+page_count[z_song.section]+d%page_count[z_song.section])%page_count[z_song.section]);
 d=plat_enc(EN_PRESET);if(d&&active_layer>=0){uint32_t count=active_layer==Z_EDIT?3:page_count[active_layer<Z_SECTIONS?active_layer:0];layer_page=(uint8_t)(layer_page+count+d%(int)count)%count;}else if(d&&!Z_SAMPLER_EDITION&&z_song.section==Z_SEQ){panel_setting(3,(z_song.step_bank+4+d%4)%4);}else if(d&&sampler_open()){sampler_page=(sampler_page+3+d%3)%3;}else if(d&&z_sys.midi_mode==2){daw_touch_at=now;daw_values[t][5]=(uint8_t)clamp((int)daw_values[t][5]+d,0,127);midi_cc(z_get(t,Z_CHANNEL)-1,10,daw_values[t][5]);}else if(d)knob(U_PRESET,d);
 for(k=0;k<4;k++){d=plat_enc(EN_K1+k);if(d&&active_layer<0&&sampler_open()&&z_sys.midi_mode!=2)sampler_knob(k,d);else if(d&&active_layer<0&&z_sys.midi_mode==2&&z_song.section!=Z_GLO){daw_values[t][k]=(uint8_t)clamp((int)daw_values[t][k]+d,0,127);midi_cc(z_get(t,Z_CHANNEL)-1,daw_cc[k],daw_values[t][k]);}else if(d)knob(active_layer>=0?layer_knob(k):pages[z_song.section][z_song.subpage%page_count[z_song.section]][k],d);}
 for(t=0;t<6;t++)for(k=0;k<128;k++)if(off_deadline[t][k]&&(int32_t)(now-off_deadline[t][k])>=0){note_packet(t,k,0,0);off_deadline[t][k]=0;}
 if(now-editor_seen>3500&&editor_effect){editor_effect=0;z_live(0);}
 if(now-editor_seen>3500)for(t=0;t<6;t++)for(k=0;k<128;k++)if(editor_notes[t][k/8]>>(k&7)&1){z_key(t,k,0,0);note_packet(t,k,0,0);if(z_song.arp_hold[t])z_alloff(t);editor_notes[t][k/8]&=(uint8_t)~(1u<<(k&7));}
 panel_keys_before=keys;held_buttons=buttons;
 lights=buttons|(active_layer>=0?(1u<<(active_layer==Z_SECTIONS?B_REC:section_button[active_layer])):(1u<<section_button[z_sys.midi_mode==2&&z_song.section!=Z_GLO?Z_HOME:z_song.section]));
 for(k=0;k<NB;k++)if((int32_t)(pulse_until[k]-now)>0)lights|=1u<<k;
 if(z_song.octave>0)lights|=1u<<B_OCTUP;if(z_song.octave<0)lights|=1u<<B_OCTDN;
 if(z_sys.midi_mode!=2&&z_song.arp_mode[z_song.selected])lights|=1u<<B_ARP;
 if(z_playing())lights|=1u<<B_PLAY;if(z_song.recording&&now/250%2)lights|=1u<<B_REC;
 if(saved_revision!=z_revision()||system_dirty)lights|=1u<<B_SAVE;
 if(z_sys.midi_mode==2){lights&=~((1u<<B_PLAY)|(1u<<B_REC));if(daw_playing)lights|=1u<<B_PLAY;if(daw_recording&&now/250%2)lights|=1u<<B_REC;}
 plat_glow(z_sys.glow);plat_play_red(z_sys.midi_mode==2?daw_recording:z_song.recording);plat_leds(lights,z_sys.glow==2?((1u<<27)-1):keys);
}

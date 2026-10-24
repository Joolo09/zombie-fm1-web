/* SPDX-License-Identifier: MIT — original factory groove, stopped until PLAY. */
static void edition_defaults(void){uint32_t t,i;
 z_song.padding[0]=Z_SAMPLER_EDITION?2:1;
 if(Z_SAMPLER_EDITION){for(t=0;t<6;t++){z_song.track[t].p[Z_MODEL]=8;z_song.track[t].p[Z_ATTACK]=0;z_song.track[t].p[Z_SUSTAIN]=127;z_song.track[t].p[Z_RELEASE]=8;}return;}
 z_song.bpm=132;z_song.swing=6;z_song.selected=3;
 for(t=0;t<6;t++){static const uint8_t levels[6]={100,84,66,94,68,58};z_song.track[t].p[Z_LEVEL]=levels[t];z_song.track[t].p[Z_LENGTH]=32;}
 for(i=0;i<32;i++){
  static const uint8_t bass[8]={48,48,51,48,55,55,51,46};
  z_step *kick=&z_song.track[0].steps[i],*snare=&z_song.track[1].steps[i],*hat=&z_song.track[2].steps[i];
  kick->note=60;kick->velocity=i%4==0?112:(i==14||i==30)?80:0;
  snare->note=60;snare->velocity=i%8==4?102:(i==27||i==31)?42:0;
  hat->note=60;hat->velocity=i%2==0?(i%4==0?65:82):(i==15||i==23||i==31)?50:0;
  if(i%2==0||i==7||i==23){z_step *s=&z_song.track[3].steps[i];s->note=bass[(i/4)%8];s->velocity=i%4==0?104:82;z_song.duration[3][i]=1;}
  if(i==3||i==10||i==19||i==26){z_step *s=&z_song.track[4].steps[i];s->note=i<16?72:75;s->velocity=74;z_song.duration[4][i]=2;}
  if(i==0||i==16){z_step *s=&z_song.track[5].steps[i];s->note=i?58:60;s->velocity=62;z_song.chord[5][i][0]=s->note+4;z_song.chord[5][i][1]=s->note+8;z_song.duration[5][i]=4;}
 }
}

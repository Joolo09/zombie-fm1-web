/* SPDX-License-Identifier: MIT — original stopped 5-synth + drum-kit groove. */
static void edition_defaults(void){uint32_t t,i;
 z_song.padding[0]=Z_SAMPLER_EDITION?2:1;
 if(Z_SAMPLER_EDITION){for(t=0;t<6;t++){z_song.track[t].p[Z_MODEL]=8;z_song.track[t].p[Z_ATTACK]=0;z_song.track[t].p[Z_SUSTAIN]=127;z_song.track[t].p[Z_RELEASE]=8;}return;}
 z_song.bpm=132;z_song.swing=6;z_song.selected=0;
 for(t=0;t<6;t++){static const uint8_t levels[6]={94,68,58,65,55,100},patches[6]={16,24,32,20,26,0};z_preset(t,patches[t]);z_song.track[t].p[Z_LEVEL]=levels[t];z_song.track[t].p[Z_LENGTH]=32;}
 for(i=0;i<32;i++){
  static const uint8_t bass[8]={48,48,51,48,55,55,51,46};
  if(i%2==0||i==7||i==23){z_step *s=&z_song.track[0].steps[i];s->note=bass[(i/4)%8];s->velocity=i%4==0?104:82;z_song.duration[0][i]=1;}
  if(i==3||i==10||i==19||i==26){z_step *s=&z_song.track[1].steps[i];s->note=i<16?72:75;s->velocity=74;z_song.duration[1][i]=2;}
  if(i==0||i==16){z_step *s=&z_song.track[2].steps[i];s->note=i?58:60;s->velocity=62;z_song.chord[2][i][0]=s->note+5;z_song.chord[2][i][1]=s->note+8;z_song.duration[2][i]=4;}
  {z_step *s=&z_song.track[5].steps[i];s->note=i%4==0?36:i%8==4?38:42;s->velocity=i%4==0?112:82;
   if(i%4==0)z_song.chord[5][i][0]=43;
   if(i%8==4)z_song.chord[5][i][1]=39;
   if(i%8==7)s->note=46;
  }
 }
}

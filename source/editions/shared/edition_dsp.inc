/* SPDX-License-Identifier: MIT — original empty five-synth + drum-kit project. */
static void edition_defaults(void){uint32_t t;
 z_song.padding[0]=Z_SAMPLER_EDITION?2:1;
 if(Z_SAMPLER_EDITION){for(t=0;t<6;t++){z_song.track[t].p[Z_MODEL]=8;z_song.track[t].p[Z_ATTACK]=0;z_song.track[t].p[Z_SUSTAIN]=127;z_song.track[t].p[Z_RELEASE]=8;}return;}
 z_song.bpm=132;z_song.swing=6;z_song.selected=0;
 for(t=0;t<6;t++){static const uint8_t levels[6]={94,68,58,65,55,100},patches[6]={16,24,32,20,26,0};z_preset(t,patches[t]);z_song.track[t].p[Z_LEVEL]=levels[t];z_song.track[t].p[Z_LENGTH]=32;}
 /* New projects are empty. Generators are explicit user actions. */
 z_song.padding[1]=0xD1;
}

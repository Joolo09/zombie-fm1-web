/* SPDX-License-Identifier: MIT — original synthesized GM drum kit, track 6.
 * No samples, heap allocation or transcendental functions in the audio loop. */
static const uint8_t drum_notes[16]={36,38,42,46,39,37,45,47,50,49,51,56,54,70,75,76};
static const char *const drum_names[16]={"KICK","SNARE","CLOSED HAT","OPEN HAT","CLAP","RIM","LOW TOM","MID TOM","HIGH TOM","CRASH","RIDE","COWBELL","TAMB","SHAKER","CLAVES","WOOD"};
static const uint16_t drum_ms[16]={420,230,100,520,220,75,320,260,210,850,620,250,180,100,80,120};
static const uint16_t drum_hz[16]={52,185,4200,3600,1400,1250,105,150,210,3200,2600,560,6200,5400,1800,900};
uint32_t z_drum_track(uint32_t t){return !Z_SAMPLER_EDITION&&t==5;}
uint32_t z_drum_index(uint32_t note){uint32_t i;for(i=0;i<16;i++)if(drum_notes[i]==note)return i;return note%16;}
uint32_t z_drum_note(uint32_t i){return drum_notes[i%16];}
const char *z_drum_name(uint32_t note){return drum_names[z_drum_index(note)];}
static float drum_signal(voice *v){
 uint32_t d=v->drum;float noise=((int32_t)(random32()>>9)-4194304)/4194304.0f,x;
 float dt=drum_hz[d]/44100.0f*bend[v->track];
 if(d==0)dt*=1+3.8f/(1+v->age*170);
 else if(d>=6&&d<=8)dt*=1+0.7f/(1+v->age*85);
 v->phase+=dt;v->phase2+=dt*1.483f;v->phase-=(int)v->phase;v->phase2-=(int)v->phase2;
 v->inc+=(noise-v->inc)*(d==2||d==3||d==12||d==13?0.42f:0.16f);
 if(d==0)x=sine(v->phase)*1.7f+noise*0.16f/(1+v->age*900);
 else if(d==1)x=(noise-v->inc)*1.15f+sine(v->phase)*0.45f;
 else if(d==2||d==3)x=(noise-v->inc)*1.4f+sine(v->phase)*sine(v->phase2)*0.22f;
 else if(d==4){float burst=v->age<0.032f?((int)(v->age*220)&1?0.15f:1.0f):0.7f;x=(noise-v->inc)*1.45f*burst;}
 else if(d==5)x=sine(v->phase)*0.65f+sine(v->phase2)*0.4f+noise*0.18f;
 else if(d>=6&&d<=8)x=sine(v->phase)*1.35f+noise*0.08f;
 else if(d==9||d==10)x=(noise-v->inc)*(d==9?1.2f:0.45f)+sine(v->phase)*sine(v->phase2)*(d==9?0.3f:0.8f);
 else if(d==11)x=(v->phase<0.5f?0.6f:-0.6f)+(v->phase2<0.5f?0.4f:-0.4f);
 else if(d==12||d==13)x=(noise-v->inc)*(d==12?1.5f:1.2f);
 else x=sine(v->phase)*(d==14?1.3f:0.8f)+sine(v->phase2)*0.3f;
 return x;
}

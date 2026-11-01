/* SPDX-License-Identifier: MIT — 0.21 "motion": per-track sound motion (glide, vibrato, tremolo, LFO shapes and
 * targets, an FM envelope, drift, bit/rate crush, humanize, rolls, random modulation) and the generative sequencer
 * (motif, Markov, Euclid, random walk, probability, arpeggio, Turing machine, chaos; style-aware; evolving).
 * No heap: fixed tables, a small state block saved apart from the project (OBJ_EXT), and stack buffers of 64 steps.
 * Generation and mutation only write steps; they run from the main loop (a key) or once per loop (evolve). */

static const uint8_t x_max[Z_X_COUNT]={127,127,127,5,4,127,127,127,127,127,127,127, 7,6,127,127,127,4,4,255};
static const uint8_t x_min[Z_X_COUNT]={0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,1,0,0};
static const uint8_t x_def[Z_X_COUNT]={0,0,40,0,0,0,0,0,0,0,0,0, 0,0,72,56,32,2,0,1};
z_ext_state z_xs;
static uint32_t g_rng=0x6d2b79f5u;
static uint32_t grand(void){g_rng^=g_rng<<13;g_rng^=g_rng>>17;g_rng^=g_rng<<5;return g_rng;}
static uint32_t grange(uint32_t n){return n?grand()%n:0;}

static void x_defaults(uint32_t t){uint32_t i;for(i=0;i<Z_X_COUNT;i++)z_xs.v[t][i]=x_def[i];z_xs.v[t][Z_G_SEED]=(uint8_t)(1+t*37);z_xs.scale[t]=0;z_xs.root[t]=0;z_xs.turing[t]=(uint16_t)(0xACE1u+t*0x1357u);}
void z_x_reset(void){uint32_t t,i;for(i=0;i<sizeof z_xs;i++)((uint8_t *)&z_xs)[i]=0;z_xs.magic=Z_EXT_MAGIC;z_xs.version=1;z_xs.size=sizeof z_xs;for(t=0;t<6;t++)x_defaults(t);}
int z_x_get(uint32_t t,uint32_t id){return t<6&&id<Z_X_COUNT?z_xs.v[t][id]:-1;}
int z_x_set(uint32_t t,uint32_t id,int v){if(t>=6||id>=Z_X_COUNT)return -1;if(v<x_min[id])v=x_min[id];if(v>x_max[id])v=x_max[id];if(z_xs.v[t][id]!=v){z_xs.v[t][id]=(uint8_t)v;if(id<Z_X_SOUND&&z_song.preset[t]!=255){z_song.preset[t]=255;}z_touch();}return v;}
/* the motion values of each track's preset (an older project without OBJ_EXT, or a bad block) */
void z_x_from_presets(void){uint32_t t,i;z_x_reset();for(t=0;t<6;t++)if(z_song.preset[t]<Z_PRESETS)for(i=0;i<Z_X_SOUND;i++)z_xs.v[t][i]=Z_PRESET_X[z_song.preset[t]][i];}
uint32_t z_x_size(void){return sizeof z_xs;}
uint8_t *z_x_data(void){return (uint8_t *)&z_xs;}
/* a saved block (OBJ_EXT or the editor) after its ranges are checked; 0 = taken */
int z_x_apply(const void *src,uint32_t n){const z_ext_state *s=(const z_ext_state *)src;uint32_t t,i;
 if(n!=sizeof z_xs||s->magic!=Z_EXT_MAGIC||s->version!=1||s->size!=sizeof z_xs)return -1;
 for(t=0;t<6;t++){for(i=0;i<Z_X_COUNT;i++)if(s->v[t][i]<x_min[i]||s->v[t][i]>x_max[i])return -1;if(s->root[t]>11)return -1;}
 z_xs=*s;return 0;}

/* ---------------------------------------------------------------- the generative sequencer --- */
/* styles: CHIP, AMBIENT, IDM, GLITCH, ACID, BREAKCORE, EXPERIMENTAL; algorithms: MOTIF, MARKOV, EUCLID, WALK, PROB,
 * ARP, TURING, CHAOS. A scale mask of 0 or 0xFFF (lock off) takes the style's own scale. */
static const uint16_t g_style_scale[7]={0xAB5,0x295,0x6AD,0x4A9,0x5AB,0x5AD,0x555};
static const uint8_t g_style_base[7]={60,48,60,60,36,48,48};
static const uint16_t g_style_rhythm[7]={0xFFFF,0x0101,0x4A53,0xB6DB,0xDDB7,0x9C9B,0x5A5A};
static const uint8_t g_style_gate[7]={55,127,70,28,45,40,90};
static const int8_t g_style_walk[7][9]={{1,2,4,3,2,3,4,2,1},{0,1,3,6,4,6,3,1,0},{2,2,3,3,2,3,3,2,2},{3,1,2,2,5,2,2,1,3},{1,1,2,4,5,4,2,1,1},{2,1,3,3,4,3,3,1,2},{3,3,3,3,1,3,3,3,3}};
typedef struct {uint8_t deg[12],n;int base;} g_scale;
static void g_make_scale(g_scale *s,uint32_t mask,uint32_t root,uint32_t style,int base){uint32_t i;
 if(!mask||(mask&0xFFF)==0xFFF)mask=g_style_scale[style%7];s->n=0;for(i=0;i<12;i++)if(mask>>i&1)s->deg[s->n++]=(uint8_t)i;if(!s->n){s->deg[0]=0;s->n=1;}s->base=base+(int)root;}
static int g_note(const g_scale *s,int d){int o=d>=0?d/s->n:-((-d+s->n-1)/s->n),i=d-o*s->n,note=s->base+12*o+s->deg[i];while(note<12)note+=12;while(note>120)note-=12;return note;}
/* the scale degree nearest to a note (for mutations of steps written by hand) */
static int g_degree(const g_scale *s,int note){int best=0,dist=999,d;for(d=-3*s->n;d<5*s->n;d++){int x=g_note(s,d)-note;if(x<0)x=-x;if(x<dist){dist=x;best=d;}}return best;}
static int g_walk(int d,uint32_t style,uint32_t complexity,int lo,int hi){const int8_t *w=g_style_walk[style%7];int sum=0,i,r;int ww[9];
 for(i=0;i<9;i++){int a=i-4;ww[i]=w[i]+(int)(complexity*(uint32_t)(a<0?-a:a))/64;sum+=ww[i];}
 r=(int)grange((uint32_t)sum);for(i=0;i<9;i++){r-=ww[i];if(r<0)break;}d+=i-4;
 if(d<lo)d=lo+(lo-d);if(d>hi)d=hi-(d-hi);if(d<lo)d=lo;if(d>hi)d=hi;return d;}
static int g_hit(uint32_t style,uint32_t i,uint32_t density){uint32_t on=g_style_rhythm[style%7]>>(i&15)&1;int p=on?40+(int)density*55/127:(int)density*50/127-12;if(style==6)p=(int)density*80/127;return (int)grange(100)<p;}
typedef struct {uint8_t note,vel,gate,chance,dur,chord[3];} g_cell;
/* write one track (or, for an anchor, only fill buf) from the track's settings; returns the hits */
static uint32_t g_compose(uint32_t t,g_cell *buf,uint32_t len,uint32_t seed_mix){
 uint8_t *v=z_xs.v[t];uint32_t algo=v[Z_G_ALGO],style=v[Z_G_STYLE],dens=v[Z_G_DENSITY],cx=v[Z_G_COMPLEX],oct=v[Z_G_OCTAVES],i,hits=0;
 g_scale s;int lo,hi,d,base=g_style_base[style%7];g_cell motif[16];uint32_t m,b;
 if(t==1)base-=12;   /* the second track sits an octave down: a bass by default */
 g_make_scale(&s,z_xs.scale[t],z_xs.root[t],style,base);lo=0;hi=(int)(s.n*oct);
 g_rng=0x9E3779B9u*(uint32_t)(v[Z_G_SEED]+1)^(algo*0x85EBCA6Bu)^(style*0xC2B2AE35u)^(t*0x27D4EB2Fu)^seed_mix;if(!g_rng)g_rng=1;
 for(i=0;i<len;i++){buf[i].note=0;buf[i].vel=0;buf[i].gate=g_style_gate[style%7];buf[i].chance=127;buf[i].dur=0;buf[i].chord[0]=buf[i].chord[1]=buf[i].chord[2]=0;}
 d=(int)grange(3)*2;if(d>hi)d=0;
 if(algo==0||algo==1){ /* MOTIF and MARKOV: a phrase, repeated and developed; MARKOV walks longer before it repeats */
  m=algo==0?(cx<40?4:cx<90?8:16):(cx<64?8:16);if(m>len)m=len;
  for(i=0;i<m;i++){motif[i].vel=(uint8_t)(g_hit(style,i,dens)||i==0?(i%4==0?118:92):0);motif[i].note=(uint8_t)d;if(motif[i].vel)d=g_walk(d,style,cx,lo,hi);}
  for(b=0;b*m<len;b++){int shift=0;uint32_t kind=b%4;if(kind==2)shift=(int)(grange(2)?2:-1);if(algo==1&&b>0&&grange(128)<cx)shift=(int)grange(5)-2;
   for(i=0;i<m&&b*m+i<len;i++){g_cell *c=&buf[b*m+i];int dd=motif[i].note+shift;if(motif[i].vel){
     if(kind==1&&grange(160)<cx){dd=g_walk(dd,style,cx,lo,hi);}            /* a small variation on the repeat */
     if(kind==3&&i+1>=m-2&&b*m+i+1>=len-2)dd=grange(2)?0:(int)s.n;          /* the phrase ends home */
     if(dd<lo)dd=lo;if(dd>hi)dd=hi;c->note=(uint8_t)g_note(&s,dd);c->vel=motif[i].vel;}}}
 }else if(algo==2){ /* EUCLID: k hits spread over the loop, a short pitch cycle of its own length (polymetric) */
  uint32_t k=1+dens*(len-1)/127,rot=cx%len,p=3+cx*5/127,j=0;int cyc[8];for(i=0;i<p&&i<8;i++){cyc[i]=d;d=g_walk(d,style,cx,lo,hi);}
  for(i=0;i<len;i++){uint32_t at=(i+rot)%len;if(((i*k)%len)<k){buf[at].note=(uint8_t)g_note(&s,cyc[j%p]);buf[at].vel=(uint8_t)(i==0?118:96);j++;}}
 }else if(algo==3){ /* WALK: one step up or down the scale at a time, sometimes two */
  for(i=0;i<len;i++){if(g_hit(style,i,dens)||i==0){buf[i].note=(uint8_t)g_note(&s,d);buf[i].vel=(uint8_t)(i%4==0?112:88+grange(16));int step=grange(128)<cx?2:1;d+=grange(2)?step:-step;if(d<lo)d=lo+1;if(d>hi)d=hi-1;if(d<lo)d=lo;}}
 }else if(algo==4){ /* PROB: every step has a note; chance decides each loop (downbeats always play) */
  for(i=0;i<len;i++){buf[i].note=(uint8_t)g_note(&s,d);buf[i].vel=(uint8_t)(64+grange(60));buf[i].chance=(uint8_t)(i%4==0?127:12+dens*90/127+grange(20));d=g_walk(d,style,cx,lo,hi);}
 }else if(algo==5){ /* ARP: chord tones over a progression, a bar per chord; the direction comes from the seed */
  static const int8_t prog7[4]={0,5,3,4},prog5[4]={0,3,1,4};uint32_t dir=v[Z_G_SEED]%6,rate=dens>90?1:dens>40?2:4,tones=3+(cx>60)+(cx>100);
  for(i=0;i<len;i+=rate){uint32_t bar=i/16,k=i/rate,n=tones*oct,cycle=n>1?2*n-2:1,q=k%cycle,j=k%n,idx,pos2;int root=(s.n>=7?prog7:prog5)[bar%4],deg;
   pos2=dir==0?j:dir==1?n-1-j:dir==2?(q<n?q:cycle-q):dir==3?grange(n):dir==4?(j%2?n-1-j/2:j/2):(k%2==0||n<2?0:1+(k/2)%(n-1));idx=pos2;
   if(pos2>=n)pos2=n-1;deg=root+(int)(pos2%tones)*2+(int)(pos2/tones)*(int)s.n;
   buf[i].note=(uint8_t)g_note(&s,deg);buf[i].vel=(uint8_t)(idx==0?116:90);buf[i].gate=(uint8_t)(rate>1?90:60);}
 }else if(algo==6){ /* TURING: a 16-bit loop register; evolving flips its bits */
  uint32_t reg=z_xs.turing[t];for(i=0;i<len;i++){uint32_t bit=reg&1;reg=(reg>>1)|(bit<<15);
   if(((reg>>8)&7)<1+dens*7/127){int dd=(int)(((reg>>3)&31)*(uint32_t)(hi+1)/32);buf[i].note=(uint8_t)g_note(&s,dd);buf[i].vel=(uint8_t)(84+((reg>>12)&3)*12);}}
 }else { /* CHAOS: anything goes, chromatic, wide, chords, odd lengths */
  for(i=0;i<len;i++)if((int)grange(128)<(int)dens){buf[i].note=(uint8_t)(s.base-12+grange(12*(oct+1)));buf[i].vel=(uint8_t)(50+grange(78));buf[i].gate=(uint8_t)(10+grange(117));buf[i].chance=(uint8_t)(60+grange(68));buf[i].dur=(uint8_t)(grange(4)==0?grange(4):0);if(grange(128)<cx/2){buf[i].chord[0]=(uint8_t)(buf[i].note+3+grange(5)+1);}}
 }
 /* the style's touch: ghosts (IDM), stutters (GLITCH), slides (ACID), held notes and chords (AMBIENT) */
 for(i=0;i<len;i++){g_cell *c=&buf[i];if(!c->vel)continue;hits++;
  if(style==2&&grange(10)<3&&i%4)c->vel=58;
  if(style==3&&grange(128)<cx/2){c->gate=(uint8_t)(8+grange(30));if(i+1<len&&!buf[i+1].vel&&grange(2)){buf[i+1]=*c;buf[i+1].vel=(uint8_t)(c->vel*3/4);}}
  if(style==4&&grange(10)<2)c->dur=1;
  if(style==1){c->dur=(uint8_t)(2+grange(6));c->gate=127;if(grange(128)<cx){c->chord[0]=(uint8_t)(g_note(&s,g_degree(&s,c->note)+2)+1);c->chord[1]=(uint8_t)(g_note(&s,g_degree(&s,c->note)+4)+1);}}
  if(style==6&&grange(128)<cx/3)c->chord[0]=(uint8_t)(g_note(&s,g_degree(&s,c->note)+3)+1);
  if(c->note>127)c->note=127;}
 return hits;}
static void g_write(uint32_t t,const g_cell *buf,uint32_t len){uint32_t i,j;z_track *tr=&z_song.track[t];
 for(i=0;i<64;i++){z_step *st=&tr->steps[i];if(i<len&&buf[i].vel){st->note=buf[i].note;st->velocity=buf[i].vel;st->chance=buf[i].chance;st->gate=buf[i].gate?buf[i].gate:1;z_song.duration[t][i]=buf[i].dur;for(j=0;j<3;j++)z_song.chord[t][i][j]=buf[i].chord[j];}
  else if(i<len){st->velocity=0;z_song.duration[t][i]=0;z_song.chord[t][i][0]=z_song.chord[t][i][1]=z_song.chord[t][i][2]=0;}}
 z_touch();}

/* drums: a kit pattern in the style (0 kick, 1 snare, 2 closed hat, 3 open hat, 4 clap, 5 rim, 6-8 toms, 9 crash,
 * 10 ride, 11 cowbell, 12 tambourine, 13 shaker, 14 claves, 15 wood) */
static void g_dm(uint32_t d,uint32_t at,uint32_t vel,uint32_t chance,uint32_t rat){if(d<16&&at<64)z_dm.step[d][at]=(z_dm_step_t){(uint8_t)(vel>127?127:vel),(uint8_t)(chance>127?127:chance),(uint8_t)(rat<1?1:rat>4?4:rat)};}
static uint32_t g_drums(uint32_t style,uint32_t dens,uint32_t cx,uint32_t seed,uint32_t len){uint32_t d,i,hits=0;
 g_rng=0x2545F491u*(seed+1)^(style*0x9E3779B9u);if(!g_rng)g_rng=1;if(len<8)len=16;
 for(d=0;d<16;d++){for(i=0;i<64;i++)z_dm.step[d][i]=(z_dm_step_t){0,127,1};z_dm.sound[d].length=(uint8_t)len;}
 for(i=0;i<len;i++){uint32_t b=i%16;
  switch(style%7){
   case 0: /* CHIP: a march: kick on the beat, snare on 2 and 4, eighth hats, arpeggio rims */
    if(b%4==0&&(b%8==0||dens>70))g_dm(0,i,118,127,1);if(b==4||b==12)g_dm(1,i,112,127,1);if(b%2==0)g_dm(2,i,b%4?70:96,127,1);if(cx>60&&b%4==3)g_dm(5,i,80,90,1);break;
   case 1: /* AMBIENT: a heartbeat, a shaker that comes and goes, a far ride */
    if(b==0)g_dm(0,i,96,127,1);if(b==10&&dens>50)g_dm(0,i,60,70,1);g_dm(13,i,40+grange(30),dens/2+20,1);if(b==8)g_dm(10,i,60,80,1);break;
   case 2: /* IDM: euclids of different lengths, ghosts and rolls */
    if(((b*5)%16)<5)g_dm(0,i,110,127,1);if((((i+3)*3)%13)<3)g_dm(1,i,100,110,grange(128)<cx?2+grange(2):1);if(((i*7)%12)<7)g_dm(2,i,50+grange(50),100,grange(128)<cx/2?2:1);if(((i*5)%9)<2)g_dm(5,i,74,100,1);
    if(grange(128)<cx/3)g_dm(14,i,70,80,1);break;
   case 3: /* GLITCH: broken kicks, stutters, rolls everywhere */
    if(grange(128)<dens/2+(b==0?60:0))g_dm(0,i,100+grange(28),110,1);if(grange(128)<dens/3)g_dm(4,i,90,100,2+grange(3));if(grange(128)<dens/2)g_dm(5,i,60+grange(60),90,grange(128)<cx?1+grange(4):1);if(grange(128)<dens/3)g_dm(15,i,80,90,3);break;
   case 4: /* ACID / ELECTRO: four on the floor, clap, open hat off the beat, accented sixteenths */
    if(b%4==0)g_dm(0,i,120,127,1);if(b==4||b==12)g_dm(4,i,108,127,1);if(b%4==2)g_dm(3,i,96,127,1);g_dm(2,i,b%4==0?90:55+grange(25),dens+20,1);if(cx>70&&(b==7||b==14))g_dm(5,i,80,100,1);break;
   case 5: /* BREAKCORE / JUNGLE: an Amen-style break, ghost snares, rolled snares, crash on the one */
    {static const uint16_t kick=0x0C05,snare=0x9290,ghost=0x2448;if(kick>>b&1)g_dm(0,i,118,127,1);if(snare>>b&1)g_dm(1,i,116,127,b==15&&grange(128)<cx?3:1);if(ghost>>b&1)g_dm(1,i,52,dens,1);if(b%2==0||dens>90)g_dm(2,i,b%2?60:84,127,grange(128)<cx/3?2:1);if(i==0)g_dm(9,i,100,127,1);
     if(grange(128)<cx/4)g_dm(grange(2)?1:0,i,90,90,2+grange(3));break;}
   default: /* EXPERIMENTAL: random sounds on euclids of random lengths */
    {uint32_t s2=4+grange(12);if(grange(128)<dens)g_dm(s2,i,60+grange(68),60+grange(68),1+grange(3));if(b%4==0&&grange(2))g_dm(0,i,110,127,1);break;}
  }}
 if(style==2&&cx>50){z_dm.sound[1].length=13;z_dm.sound[2].length=12;z_dm.sound[5].length=9;}  /* IDM polymeters */
 if(style==6)for(d=4;d<16;d++)z_dm.sound[d].length=(uint8_t)(5+grange(12));
 for(d=0;d<16;d++)for(i=0;i<64;i++)hits+=z_dm.step[d][i].velocity!=0;
 z_touch();return hits;}

/* generate a track from its settings; the scale lock and root come from the panel (0: the style's scale) */
uint32_t z_gen(uint32_t t,uint32_t scale_mask,uint32_t root){g_cell buf[64];uint32_t len,hits;
 if(t>=6)return 0;z_xs.scale[t]=(uint16_t)(scale_mask&0xFFF);z_xs.root[t]=(uint8_t)(root%12);
 if(z_drum_track(t)){z_dm_checkpoint();len=z_dm_common_length();return g_drums(z_xs.v[t][Z_G_STYLE],z_xs.v[t][Z_G_DENSITY],z_xs.v[t][Z_G_COMPLEX],z_xs.v[t][Z_G_SEED],len?len:16);}
 z_checkpoint();len=(uint32_t)z_song.track[t].p[Z_LENGTH];if(z_xs.v[t][Z_G_ALGO]==6)z_xs.turing[t]=(uint16_t)(0xACE1u^(z_xs.v[t][Z_G_SEED]*0x9E37u));
 hits=g_compose(t,buf,len,0);g_write(t,buf,len);return hits;}
/* a gradual change that keeps the shape: a few notes move along the scale, rest or return to the original
 * (regenerated from the seed); TURING flips register bits instead. From the sequencer, once per evolve period. */
uint32_t z_gen_mutate(uint32_t t){uint32_t len,i,n,changes=0;uint8_t *v;g_scale s;g_cell anchor[64];
 if(t>=6)return 0;v=z_xs.v[t];
 if(z_drum_track(t)){uint32_t d;g_rng^=0x1234567u+audio_frame;len=z_dm_common_length();if(!len)len=16;n=1+v[Z_G_MUTATE]*len/300;
  for(i=0;i<n;i++){uint32_t at=grange(len);d=(uint32_t)(grange(4)==0?1:grange(2)?2:5+grange(11));if(at%4==0&&d<2)continue;
   if(z_dm.step[d][at].velocity){if(grange(3))z_dm.step[d][at].ratchet=(uint8_t)(1+grange(3));else z_dm.step[d][at].velocity=0;}else z_dm.step[d][at]=(z_dm_step_t){(uint8_t)(50+grange(60)),100,1};changes++;}
  z_touch();return changes;}
 len=(uint32_t)z_song.track[t].p[Z_LENGTH];
 if(v[Z_G_ALGO]==6){n=1+v[Z_G_MUTATE]/24;for(i=0;i<n;i++)z_xs.turing[t]^=(uint16_t)(1u<<grange(16));g_compose(t,anchor,len,0);g_write(t,anchor,len);return n;}
 g_compose(t,anchor,len,0);g_rng^=0x9E3779B9u+audio_frame*2654435761u;if(!g_rng)g_rng=1;
 g_make_scale(&s,z_xs.scale[t],z_xs.root[t],v[Z_G_STYLE],g_style_base[v[Z_G_STYLE]%7]-(t==1?12:0));
 n=1+v[Z_G_MUTATE]*len/400;
 for(i=0;i<n;i++){uint32_t at=grange(len),r=grange(10);z_step *st=&z_song.track[t].steps[at];
  if(r>=8||(!st->velocity&&anchor[at].vel&&r>=5)){if(anchor[at].vel){st->note=anchor[at].note;st->velocity=anchor[at].vel;st->gate=anchor[at].gate;}else st->velocity=0;}
  else if(!st->velocity){if(at%4&&grange(128)<v[Z_G_DENSITY]){int d=g_degree(&s,anchor[at].vel?anchor[at].note:s.base);st->note=(uint8_t)g_note(&s,d+(int)grange(3)-1);st->velocity=(uint8_t)(70+grange(40));st->gate=anchor[at].gate?anchor[at].gate:60;st->chance=127;}}
  else if(r<4){int d=g_degree(&s,st->note)+(grange(2)?1:-1);st->note=(uint8_t)g_note(&s,d);}
  else if(r<5&&at%4){st->velocity=0;}
  else if(r<6){int nn=st->note+(grange(2)?12:-12);if(nn>=24&&nn<=108)st->note=(uint8_t)nn;}
  else st->velocity=(uint8_t)(st->velocity>100?84:118);
  changes++;}
 z_touch();return changes;}
/* JAM: the whole project in one style, with that style's bank and roles: lead, bass, pad, arpeggio, texture, drums */
void z_gen_all(uint32_t style,uint32_t seed,uint32_t scale_mask,uint32_t root){uint32_t t;
 static const uint8_t bank_of[7]={0,2,1,7,4,3,8},bpm[7]={140,84,124,132,128,172,104};
 static const uint8_t algo[7][5]={{0,3,5,5,2},{3,3,1,5,2},{1,3,4,5,2},{7,3,2,6,7},{0,3,4,5,6},{1,3,2,5,7},{6,3,7,5,7}};
 static const uint8_t dens[7][5]={{96,70,30,110,40},{40,30,24,70,20},{80,60,40,100,50},{90,70,50,110,90},{110,100,40,110,70},{100,90,40,120,90},{70,60,50,90,80}};
 style%=7;z_checkpoint();z_dm_checkpoint();z_song.bpm=bpm[style];z_song.swing=(uint8_t)(style==2||style==1?12:style==5?0:6);
 for(t=0;t<5;t++){uint8_t *v=z_xs.v[t];uint32_t p=Z_PRESETS>=249?128+bank_of[style]*11+t:255;
  if(p<Z_PRESETS)z_preset(t,p);
  v[Z_G_ALGO]=algo[style][t];v[Z_G_STYLE]=(uint8_t)style;v[Z_G_DENSITY]=dens[style][t];v[Z_G_COMPLEX]=(uint8_t)(40+((seed*7+t*13)%60));v[Z_G_SEED]=(uint8_t)(seed+t*31);v[Z_G_OCTAVES]=(uint8_t)(t==2||t==1?1:2);
  v[Z_G_EVOLVE]=(uint8_t)(t==0||t==4?2:0);v[Z_G_MUTATE]=(uint8_t)(style==3||style==6?64:28);
  z_song.track[t].p[Z_LENGTH]=(int16_t)(style==1||t==2?32:16);z_song.track[t].p[Z_MUTE]=0;
  z_xs.scale[t]=(uint16_t)(scale_mask&0xFFF);z_xs.root[t]=(uint8_t)(root%12);
  if(z_xs.v[t][Z_G_ALGO]==6)z_xs.turing[t]=(uint16_t)(0xACE1u^(v[Z_G_SEED]*0x9E37u));
  {g_cell buf[64];uint32_t len=(uint32_t)z_song.track[t].p[Z_LENGTH];if(t==2){v[Z_G_STYLE]=1;} /* the pad holds chords in every style */
   g_compose(t,buf,len,0);g_write(t,buf,len);v[Z_G_STYLE]=(uint8_t)style;}}
 z_xs.v[5][Z_G_STYLE]=(uint8_t)style;z_xs.v[5][Z_G_SEED]=(uint8_t)seed;z_xs.v[5][Z_G_DENSITY]=80;z_xs.v[5][Z_G_COMPLEX]=(uint8_t)(50+seed%60);z_xs.v[5][Z_G_EVOLVE]=(uint8_t)(style==3||style==5?2:0);
 g_drums(style,80,z_xs.v[5][Z_G_COMPLEX],seed,16);
 z_touch();}

/* evolve: once every 1, 2, 4 or 8 loops of a track while the song plays */
static uint8_t g_loops[6];
static void g_evolve_at(uint32_t t){uint32_t e=z_xs.v[t][Z_G_EVOLVE];if(!e)return;if(++g_loops[t]>=(1u<<(e-1))){g_loops[t]=0;z_gen_mutate(t);}}

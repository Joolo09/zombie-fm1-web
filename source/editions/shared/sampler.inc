/* SPDX-License-Identifier: MIT. Included in our DSP translation unit. */
z_sampler_state z_sampler;
static z_sampler_state sampler_transfer;
static void sampler_init(void){uint32_t i;uint8_t *p=(uint8_t *)&z_sampler;
 for(i=0;i<sizeof z_sampler;i++)p[i]=0;
 z_sampler.magic=Z_SAMPLER_MAGIC;z_sampler.version=1;
 for(i=0;i<Z_PADS;i++){z_pad *pad=&z_sampler.pad[i];pad->end=65535;pad->source=i%16<4?10+i%16:(i%16-4)%10;pad->gain=100;}
}
int z_sampler_valid(const z_sampler_state *s){uint32_t i;
 if(s->magic!=Z_SAMPLER_MAGIC||s->version!=1||s->tracks>63||s->bank>1||s->selected>=32||s->selected/16!=s->bank||s->padding[0]||s->padding[1]||s->padding[2])return 0;
 for(i=0;i<32;i++){const z_pad *p=&s->pad[i];if(p->start>=p->end||p->source>13||p->tune < -24||p->tune>24||p->gain>127||p->pan < -64||p->pan>63||p->mode>2||p->reverse>1||p->choke>8||p->quality>2)return 0;}
 return 1;
}
uint32_t z_sampler_snapshot(void){sampler_transfer=z_sampler;return (uint32_t)(uintptr_t)&sampler_transfer;}
int z_sampler_apply(void){uint32_t t;if(!z_sampler_valid(&sampler_transfer))return -1;for(t=0;t<6;t++)if((z_sampler.tracks^sampler_transfer.tracks)>>t&1)z_alloff(t);z_sampler=sampler_transfer;z_sampler.tracks=Z_SAMPLER_EDITION?63:0;z_touch();return 0;}
int z_sampler_enable(uint32_t t,uint32_t on){if(t>=6||on>1)return -1;
 if(!Z_SAMPLER_EDITION)return -1;
 on=1;
 if(((z_sampler.tracks>>t)&1)==on&&(!on||z_get(t,Z_MODEL)==8))return 0;
 z_alloff(t);
 if(on){z_sampler.tracks|=1u<<t;z_set(t,Z_MODEL,8);z_set(t,Z_ATTACK,0);z_set(t,Z_SUSTAIN,127);z_set(t,Z_RELEASE,8);z_arp_set(t,0,0);}
 else z_sampler.tracks&=~(1u<<t);
 z_touch();return 0;
}
int z_sampler_set(uint32_t i,uint32_t field,int value){z_pad next;if(i>=32||field>10)return -1;next=z_sampler.pad[i];
 switch(field){
 case 0:if(value<0||value>65534||value>=next.end)return -1;next.start=value;break;
 case 1:if(value<1||value>65535||value<=next.start)return -1;next.end=value;break;
 case 2:if(value<0||value>13)return -1;next.source=value;break;
 case 3:if(value < -24||value>24)return -1;next.tune=value;break;
 case 4:if(value<0||value>127)return -1;next.gain=value;break;
 case 5:if(value < -64||value>63)return -1;next.pan=value;break;
 case 6:if(value<0||value>2)return -1;next.mode=value;break;
 case 7:if(value<0||value>1)return -1;next.reverse=value;break;
 case 8:if(value<0||value>8)return -1;next.choke=value;break;
 case 9:if(value<0||value>2)return -1;next.quality=value;break;
 case 10:if(value<0||value>1)return -1;z_sampler.bank=value;z_sampler.selected=value*16+(i%16);z_touch();return 0;
 }
 z_sampler.pad[i]=next;z_touch();return 0;
}
int z_sampler_chop(uint32_t index,uint32_t slices){uint32_t i,base,span;z_pad source;
 if(index>=32||(slices!=4&&slices!=8&&slices!=16))return -1;
 source=z_sampler.pad[index];span=source.end-source.start;if(span<slices)return -1;
 base=index&16;for(i=0;i<slices;i++){z_pad p=source;p.start=source.start+span*i/slices;p.end=source.start+span*(i+1)/slices;z_sampler.pad[base+i]=p;}
 z_sampler.selected=base;z_sampler.bank=base/16;z_touch();return 0;
}
uint32_t z_sample_frames(uint32_t source){return source<14?pcm_n[source]:0;}
uint32_t z_sample_rate(uint32_t source){return source<14?pcm_rate[source]:0;}
int z_sample_peak(uint32_t source,uint32_t column,uint32_t columns){uint32_t i,a,b;int peak=0;
 if(source>=14||!pcm[source]||!columns||columns>240||column>=columns)return 0;
 a=pcm_n[source]*column/columns;b=pcm_n[source]*(column+1)/columns;
 for(i=a;i<b;i++){int v=pcm[source][i];if(v<0)v=-v;if(v>peak)peak=v;}return peak;
}

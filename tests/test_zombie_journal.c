/* SPDX-License-Identifier: MIT — actual device journal with the Zombie object map, simulated NOR. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"
static uint8_t nor[0x100000],baseline[0x100000];static int fail_after=-1;
static int st_read(uint32_t off,void *dst,uint32_t n){if(off+n>sizeof nor)abort();memcpy(dst,nor+off,n);return 0;}
static int st_erase(uint32_t off){if(off%4096||off+4096>sizeof nor)abort();memset(nor+off,255,4096);return 0;}
static int st_prog(uint32_t off,const void *src,uint32_t n){if(fail_after==0)return -1;if(fail_after>0)fail_after--;if((off&255)+n>256||off+n>sizeof nor)abort();const uint8_t *b=src;for(uint32_t i=0;i<n;i++)nor[off+i]&=b[i];return 0;}
#include "../source/platform-081/device/storage.c"
static void check(int ok,const char *s){if(!ok){printf("FAIL %s\n",s);exit(1);}}
int main(void){uint8_t a[3452],b[3452],got[3452];memset(a,0x31,sizeof a);memset(b,0x62,sizeof b);memset(nor,255,sizeof nor);
 for(uint32_t o=0;o<OBJ_NOBJ;o++){uint32_t lo=st_sector(o,0),hi=st_sector(o,1)+4096;check(lo>=0x97000&&hi<=0xE0000,"journal range");check(hi<=0xA0000||lo>=0xD0000,"journal must not overlap user PCM slots");for(uint32_t p=0;p<o;p++)check(hi<=st_sector(p,0)||lo>=st_sector(p,1)+4096,"journal overlap");}
 for(uint32_t object=0;object<2;object++){uint32_t obj=object?OBJ_DRUM:OBJ_SETTINGS,n=object?3208:3452;memset(nor,255,sizeof nor);check(st_save(obj,a,n)==0,"first save");memcpy(baseline,nor,sizeof nor);uint32_t pages=(n+255)/256;
  for(uint32_t cut=0;cut<=pages;cut++){memcpy(nor,baseline,sizeof nor);fail_after=cut;st_save(obj,b,n);fail_after=-1;check(st_load(obj,got,sizeof got)==(int)n&&!memcmp(got,a,n),"torn payload/header must retain previous snapshot");}
  memcpy(nor,baseline,sizeof nor);check(st_save(obj,b,n)==0,"complete new save");check(st_load(obj,got,sizeof got)==(int)n&&!memcmp(got,b,n),"new snapshot selected");st_hdr_t h;int current=st_current(obj,&h);nor[st_sector(obj,current)+ST_PAYLOAD_OFF]^=1;check(st_load(obj,got,sizeof got)==(int)n&&!memcmp(got,a,n),"corrupt latest snapshot falls back");printf("PASS object %u: all interrupted-write boundaries, CRC fallback and full-size payload\n",obj);
 }
 puts("PASS actual Zombie flash map is disjoint; no hardware was written");return 0;}

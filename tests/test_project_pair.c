/* SPDX-License-Identifier: MIT — paired project commits on the actual NOR journal. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "model.h"
#include "drum_machine.h"
static uint8_t nor[0x100000],baseline[0x100000];
static int fail_after=-1,tear,drop,operations;
static int st_read(uint32_t off,void *dst,uint32_t n){if(off+n>sizeof nor)abort();memcpy(dst,nor+off,n);return 0;}
static int st_erase(uint32_t off){if(off%4096||off+4096>sizeof nor)abort();operations++;if(fail_after==0){if(tear)memset(nor+off,255,2048);return -1;}if(fail_after>0)fail_after--;memset(nor+off,255,4096);return 0;}
static int st_prog(uint32_t off,const void *src,uint32_t n){if((off&255)+n>256||off+n>sizeof nor)abort();operations++;uint32_t amount=n;int failed=fail_after==0;if(failed){if(!tear)return -1;amount=tear==1?n/2:n;}else if(fail_after>0)fail_after--;if(!drop){const uint8_t *p=src;for(uint32_t i=0;i<amount;i++)nor[off+i]&=p[i];}return failed?-1:0;}
#include "../source/platform-081/device/storage.c"
int plat_store_load(uint32_t obj,void *dst,uint32_t max){return st_load(obj,dst,max);}
int plat_store_save(uint32_t obj,const void *src,uint32_t n){return st_save(obj,src,n);}
static uint32_t crc32(const uint8_t *p,uint32_t n){return st_crc32(p,n);}
#include "../source/editions/shared/project_store.inc"
static void check(int ok,const char *s){if(!ok){printf("FAIL %s\n",s);exit(1);}}
static z_project root,loaded_root;static z_dm_state kit,loaded_kit;
static void fill(uint8_t generation){memset(&root,generation,sizeof root);memset(&kit,generation,sizeof kit);}
static void legacy(void){memset(nor,255,sizeof nor);fail_after=-1;tear=drop=0;fill(0x31);check(!st_save(OBJ_SETTINGS,&root,sizeof root)&&!st_save(OBJ_DRUM,&kit,sizeof kit),"legacy save");project_pair_boot();}
static uint8_t restore(void){project_pair_boot();check(project_store_load(OBJ_SETTINGS,&loaded_root,sizeof loaded_root)==sizeof loaded_root,"complete root restored");check(project_store_load(OBJ_DRUM,&loaded_kit,sizeof loaded_kit)==sizeof loaded_kit,"complete kit restored");uint8_t generation=((uint8_t *)&loaded_root)[0];check(generation==((uint8_t *)&loaded_kit)[0],"no mixed musical generations");return generation;}
static void commit(uint8_t generation){fill(generation);check(project_pair_save(&root,&kit)==0,"paired save");check(restore()==generation,"new complete pair restored");}
int main(void){
 legacy();fill(0x62);check(!st_save(OBJ_SETTINGS,&root,sizeof root),"baseline independent root save");fail_after=0;st_save(OBJ_DRUM,&kit,sizeof kit);fail_after=-1;st_load(OBJ_SETTINGS,&loaded_root,sizeof loaded_root);st_load(OBJ_DRUM,&loaded_kit,sizeof loaded_kit);check(((uint8_t *)&loaded_root)[0]!=((uint8_t *)&loaded_kit)[0],"old split-save failure reproduced");puts("PASS baseline exposes independently valid but mixed project journals");
 uint32_t boundaries=0;
 for(uint32_t phase=0;phase<3;phase++){
  legacy();for(uint32_t j=0;j<phase;j++)commit((uint8_t)(0x40+j));uint8_t previous=restore();memcpy(baseline,nor,sizeof nor);fill(0x62);operations=0;check(!project_pair_save(&root,&kit),"count complete save operations");int count=operations;
  for(int mode=0;mode<3;mode++)for(int cut=0;cut<=count;cut++){
   memcpy(nor,baseline,sizeof nor);fail_after=-1;tear=0;project_pair_boot();fill(0x62);tear=mode;fail_after=cut;project_pair_save(&root,&kit);fail_after=-1;tear=0;uint8_t generation=restore();check(generation==previous||generation==0x62,"power cut restores old or complete new project");commit(0x73);boundaries++;
  }
 }
 printf("PASS %u interruption/partial-erase/partial-or-complete-program boundaries, first migration and both banks; retry succeeds\n",boundaries);
 legacy();commit(0x40);commit(0x41);commit(0x42);uint32_t bank=pair_manifest.active;st_hdr_t h;uint32_t obj=pair_objects[bank][0];int copy=st_current(obj,&h);nor[st_sector(obj,(uint32_t)copy)+ST_PAYLOAD_OFF]^=1;check(restore()==0x41,"corrupt active root falls back to a whole previous pair");
 obj=pair_objects[pair_manifest.active][1];copy=st_current(obj,&h);nor[st_sector(obj,(uint32_t)copy)+ST_PAYLOAD_OFF]^=1;project_pair_boot();check(project_store_load(OBJ_SETTINGS,&loaded_root,sizeof loaded_root)<0&&project_store_load(OBJ_DRUM,&loaded_kit,sizeof loaded_kit)<0,"no intact pair fails closed");puts("PASS component CRC fallback selects a whole pair; no intact pair cannot resurrect mixed data");
 legacy();commit(0x40);commit(0x41);for(uint32_t c=0;c<2;c++)nor[st_sector(OBJ_PROJECT_COMMIT,c)+ST_PAYLOAD_OFF]^=1;project_pair_boot();check(project_store_load(OBJ_SETTINGS,&loaded_root,sizeof loaded_root)<0&&project_store_load(OBJ_DRUM,&loaded_kit,sizeof loaded_kit)<0,"lost manifests cannot reinterpret tagged staging data as legacy");puts("PASS lost commit records cannot load an uncommitted tagged legacy slot");
 legacy();commit(0x40);commit(0x41);uint32_t corrupt=pair_objects[pair_manifest.active][0];int badcopy=st_current(corrupt,&h);nor[st_sector(corrupt,(uint32_t)badcopy)+ST_PAYLOAD_OFF]^=1;fill(0x62);fail_after=16;check(project_pair_save(&root,&kit)!=0,"interrupted save after live corruption");fail_after=-1;check(restore()==0x40,"refresh before save protects surviving whole bank");puts("PASS corruption after boot cannot make a failed save overwrite the sole surviving pair");
 legacy();commit(0x40);fill(0x62);drop=1;check(project_pair_save(&root,&kit)!=0,"silent NOR write failure detected");drop=0;check(restore()==0x40,"silent write failure preserves committed pair");puts("PASS silent failed programming rejected before pair commit");return 0;
}

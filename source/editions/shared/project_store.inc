/* SPDX-License-Identifier: MIT
 * Commit melodies and the authoritative drum kit together. Each musical bank
 * uses the platform's existing A/B object journals. Only the inactive bank is
 * written; the small commit manifest is written after both payloads verify.
 * System preferences and the disabled sampler config retain their own journals.
 */
#if !Z_SAMPLER_EDITION
#define Z_PAIR_MAGIC 0x3150585au
#define Z_PAIR_TAG 0xa6u
typedef struct {uint32_t magic;uint8_t version,active,valid,reserved;uint32_t crc[2][2];} z_pair_manifest;
_Static_assert(sizeof(z_pair_manifest)==24,"Paired project commit layout");
static z_pair_manifest pair_manifest;
static z_project pair_root;
static z_dm_state pair_drums;
static uint8_t pair_boot_status; /* 1: whole-pair recovery, 2: no intact commit */
static int pair_mode; /* 0: legacy, 1: verified pair, -1: fail closed */
static const uint8_t pair_objects[2][2]={{OBJ_SETTINGS,OBJ_DRUM},{OBJ_PROJECT_ALT,OBJ_DRUM_ALT}};
static int pair_manifest_valid(const z_pair_manifest *m){return m->magic==Z_PAIR_MAGIC&&m->version==1&&m->active<2&&m->valid&&!(m->valid&~3u)&&(m->valid&(1u<<m->active))&&!m->reserved;}
static int pair_verify(uint32_t bank,const z_pair_manifest *m){
 if(plat_store_load(pair_objects[bank][0],&pair_root,sizeof pair_root)!=sizeof pair_root||plat_store_load(pair_objects[bank][1],&pair_drums,sizeof pair_drums)!=sizeof pair_drums)return 0;
 return pair_root.padding[2]==Z_PAIR_TAG&&crc32((const uint8_t *)&pair_root,sizeof pair_root)==m->crc[bank][0]&&crc32((const uint8_t *)&pair_drums,sizeof pair_drums)==m->crc[bank][1];
}
static void project_pair_boot(void){
 pair_boot_status=0;pair_mode=0;int n=plat_store_load(OBJ_PROJECT_COMMIT,&pair_manifest,sizeof pair_manifest);
 if(n<0){
  /* A tagged legacy slot was written by a paired save. Without its manifest it
   * may be an uncommitted bank; never combine it with an unrelated kit. */
  n=plat_store_load(OBJ_SETTINGS,&pair_root,sizeof pair_root);
  if(n==sizeof pair_root&&pair_root.padding[2]==Z_PAIR_TAG){pair_mode=-1;pair_boot_status=2;}
  return;
 }
 pair_mode=-1;pair_boot_status=2;if(n!=sizeof pair_manifest||!pair_manifest_valid(&pair_manifest))return;
 uint32_t bank=pair_manifest.active;
 for(uint32_t j=0;j<2;j++,bank^=1)if((pair_manifest.valid&(1u<<bank))&&pair_verify(bank,&pair_manifest)){pair_manifest.active=(uint8_t)bank;pair_mode=1;pair_boot_status=(uint8_t)(j?1:0);return;}
}
static int project_store_load(uint32_t obj,void *dst,uint32_t max){
 if(obj!=OBJ_SETTINGS&&obj!=OBJ_DRUM)return plat_store_load(obj,dst,max);
 if(pair_mode<0)return -1;
 if(pair_mode==1)obj=pair_objects[pair_manifest.active][obj==OBJ_DRUM];
 return plat_store_load(obj,dst,max);
}
static int project_pair_save(const z_project *root,const z_dm_state *drums){
 z_pair_manifest next;uint32_t bank;int rc;
 /* A previous commit may have reached NOR even if its final readback failed;
  * also protect the surviving bank if corruption happened since startup. */
 project_pair_boot();
 if(pair_mode==1){next=pair_manifest;bank=next.active^1u;}
 else {for(uint32_t i=0;i<sizeof next;i++)((uint8_t *)&next)[i]=0;next.magic=Z_PAIR_MAGIC;next.version=1;bank=1;}
 pair_root=*root;pair_root.padding[2]=Z_PAIR_TAG;pair_drums=*drums;
 next.crc[bank][0]=crc32((const uint8_t *)&pair_root,sizeof pair_root);next.crc[bank][1]=crc32((const uint8_t *)&pair_drums,sizeof pair_drums);
 rc=plat_store_save(pair_objects[bank][0],&pair_root,sizeof pair_root);if(rc)return rc;
 rc=plat_store_save(pair_objects[bank][1],&pair_drums,sizeof pair_drums);if(rc)return rc;
 if(!pair_verify(bank,&next))return -7;
 next.active=(uint8_t)bank;next.valid|=(uint8_t)(1u<<bank);
 rc=plat_store_save(OBJ_PROJECT_COMMIT,&next,sizeof next);if(rc)return rc;
 pair_manifest=next;pair_mode=1;pair_boot_status=0;return 0;
}
#else
static void project_pair_boot(void){}
static int project_store_load(uint32_t obj,void *dst,uint32_t max){return plat_store_load(obj,dst,max);}
#endif

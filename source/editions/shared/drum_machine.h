/* SPDX-License-Identifier: MIT — original Zombie drum mechanics, track six only. */
#pragma once
#define Z_DM_MAGIC 0x314D445Au
#define Z_DM_SOUNDS 16u
#define Z_DM_STEPS 64u
typedef struct {uint8_t velocity,chance,ratchet;} z_dm_step_t;
typedef struct {uint8_t level,decay;int8_t tune,pan;uint8_t mute,length,reserved[2];} z_dm_sound_t;
typedef struct {uint32_t magic;uint16_t version;uint8_t selected,reserved;z_dm_sound_t sound[16];z_dm_step_t step[16][64];} z_dm_state;
_Static_assert(sizeof(z_dm_state)==3208,"Zombie drum state layout");
extern z_dm_state z_dm;
void z_dm_init(void);
void z_dm_migrate(void);
int z_dm_valid(const z_dm_state *);
uint32_t z_dm_size(void);
uint32_t z_dm_snapshot(void);
int z_dm_apply(uint32_t);
int z_dm_param(uint32_t,uint32_t,int32_t);
int z_dm_step(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
void z_dm_select(uint32_t);
uint32_t z_dm_position(uint32_t);
void z_dm_key(uint32_t,uint32_t,uint32_t);
void z_dm_transport(uint32_t);
void z_dm_songpos(uint32_t);
void z_dm_clock(void);
void z_dm_audio(float *,float *,float *,float *,float);
uint32_t z_dm_history(uint32_t);
uint32_t z_dm_history_state(void);
void z_dm_history_reset(void);

void z_dm_panic(void);
int z_dm_pattern(uint32_t);

void z_dm_checkpoint(void);

int z_dm_erase(uint32_t,uint32_t);

void z_dm_authoritative(void);

/* SPDX-License-Identifier: MIT — original pad sampler, no external sampler code. */
#pragma once
#include <stdint.h>
#define Z_PADS 32
#define Z_SAMPLER_MAGIC 0x3150535au
typedef struct {uint16_t start,end;uint8_t source;int8_t tune;uint8_t gain;int8_t pan;
 uint8_t mode,reverse,choke,quality;} z_pad;
typedef struct {uint32_t magic;uint16_t version;uint8_t tracks,bank,selected,padding[3];z_pad pad[Z_PADS];} z_sampler_state;
_Static_assert(sizeof(z_pad)==12,"pad wire layout");
_Static_assert(sizeof(z_sampler_state)==396,"sampler wire layout");
extern z_sampler_state z_sampler;
int z_sampler_valid(const z_sampler_state *state);
uint32_t z_sampler_snapshot(void);
int z_sampler_apply(void);
int z_sampler_set(uint32_t pad,uint32_t field,int value);
int z_sampler_enable(uint32_t track,uint32_t on);
int z_sampler_chop(uint32_t pad,uint32_t slices);
uint32_t z_sample_frames(uint32_t source);
uint32_t z_sample_rate(uint32_t source);
int z_sample_peak(uint32_t source,uint32_t column,uint32_t columns);

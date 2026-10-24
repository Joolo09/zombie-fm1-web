/* SPDX-License-Identifier: MIT */
#pragma once
#include "model.h"
#include "sampler.h"
void z_init(void);
void z_reset_audio(void);
void z_note(uint32_t track,uint32_t note,uint32_t velocity,uint32_t on);
void z_key(uint32_t track,uint32_t note,uint32_t velocity,uint32_t on);
void z_record(uint32_t on);
void z_live(uint32_t effect);
uint32_t z_live_state(void);
void z_apply_state(void);
void z_clear_track(uint32_t track);
void z_checkpoint(void);
uint32_t z_history(uint32_t redo);
uint32_t z_history_state(void);
void z_history_reset(void);
int z_pattern(uint32_t track,uint32_t action,uint32_t target);
void z_solo(uint32_t track_plus_one);
uint32_t z_solo_state(void);
int z_arp_set(uint32_t track,uint32_t field,uint32_t value);
void z_touch(void);
uint32_t z_revision(void);
const char *z_preset_name(uint32_t preset);
uint32_t z_state_size(void);
void z_bend(uint32_t track,uint32_t value14);
void z_sustain(uint32_t track,uint32_t on);
void z_alloff(uint32_t track);
int z_set(uint32_t track,uint32_t param,int value);
int z_get(uint32_t track,uint32_t param);
void z_preset(uint32_t track,uint32_t preset);
void z_step_set(uint32_t track,uint32_t index,uint32_t note,uint32_t velocity,uint32_t chance,uint32_t gate);
void z_transport(uint32_t on);
void z_clock(uint32_t status);
void z_songpos(uint32_t sixteenths);
void z_tempo(uint32_t bpm,uint32_t swing);
void z_render(float *stereo,uint32_t frames);
void z_sample(uint32_t slot,const int16_t *pcm,uint32_t length,uint32_t rate);
uint32_t z_position(void);
uint32_t z_playing(void);
uint32_t z_external(uint32_t on);
void z_set_midi_out(void (*callback)(uint32_t,uint32_t,uint32_t,uint32_t));
uint32_t z_snapshot(void);
float *z_buffer(void);
uint8_t *z_transfer(void);
void z_preview_sample(uint32_t slot,uint32_t length,uint32_t rate);
uint32_t z_voice_count(void);

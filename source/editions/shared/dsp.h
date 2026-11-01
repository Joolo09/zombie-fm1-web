/* SPDX-License-Identifier: MIT */
#pragma once
#include "model.h"
#include "sampler.h"
#include "drum_machine.h"
void z_init(void);
uint32_t z_drum_track(uint32_t track);
uint32_t z_drum_index(uint32_t note);
uint32_t z_drum_note(uint32_t index);
const char *z_drum_name(uint32_t note);
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
void z_countin(uint32_t beats);       /* 0 cancels; else clicks that many beats, then starts playing and recording */
uint32_t z_countin_state(void);       /* beats left in the count-in, 0 when none */
void z_rec_click(uint32_t on);        /* metronome while recording */
void z_clock(uint32_t status);
void z_songpos(uint32_t sixteenths);
void z_tempo(uint32_t bpm,uint32_t swing);
void z_render(float *stereo,uint32_t frames);
void z_sample(uint32_t slot,const int16_t *pcm,uint32_t length,uint32_t rate);
uint32_t z_position(void);
uint32_t z_playing(void);
uint32_t z_external(uint32_t on);
void z_set_midi_out(void (*callback)(uint32_t,uint32_t,uint32_t,uint32_t));
void z_set_clock_out(void (*callback)(uint32_t status));   /* 0xF8 clock, 0xFA start, 0xFC stop */
uint32_t z_snapshot(void);
float *z_buffer(void);
uint8_t *z_transfer(void);
void z_preview_sample(uint32_t slot,uint32_t length,uint32_t rate);
uint32_t z_voice_count(void);

int z_erase(uint32_t,uint32_t,uint32_t);

uint32_t z_track_hits(uint32_t);

void z_new_project(void);

uint32_t z_note_lights(uint32_t track,int32_t base);
void z_audition_step(uint32_t track,uint32_t step);

/* 0.21 motion and generator (gen.inc). Per track: twelve sound values (presets carry them) and eight for the
 * generator. Saved as one block apart from the project (OBJ_EXT); the editor reads and sets it by id. */
enum {Z_X_GLIDE,Z_X_VIB,Z_X_VIB_RATE,Z_X_SHAPE,Z_X_TARGET,Z_X_FM_ENV,Z_X_DRIFT,Z_X_CRUSH,Z_X_HUMAN,Z_X_ROLL,Z_X_TREM,Z_X_RANDOM,
      Z_X_SOUND,Z_G_ALGO=Z_X_SOUND,Z_G_STYLE,Z_G_DENSITY,Z_G_COMPLEX,Z_G_MUTATE,Z_G_OCTAVES,Z_G_EVOLVE,Z_G_SEED,Z_X_COUNT};
#define Z_EXT_MAGIC 0x3158545Au
typedef struct {uint32_t magic;uint16_t version,size;uint8_t v[6][Z_X_COUNT];uint16_t scale[6],turing[6];uint8_t root[6],reserved[6];} z_ext_state;
extern z_ext_state z_xs;
void z_x_reset(void); void z_x_from_presets(void);
int z_x_get(uint32_t track,uint32_t id);
int z_x_set(uint32_t track,uint32_t id,int value);
uint32_t z_x_size(void);
uint8_t *z_x_data(void);
int z_x_apply(const void *src,uint32_t n);
uint32_t z_gen(uint32_t track,uint32_t scale_mask,uint32_t root);
uint32_t z_gen_mutate(uint32_t track);
void z_gen_all(uint32_t style,uint32_t seed,uint32_t scale_mask,uint32_t root);

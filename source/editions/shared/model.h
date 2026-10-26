/* SPDX-License-Identifier: MIT
 * Zombie Core: original application and DSP. No SLOOP/Skiff engine code. */
#pragma once
#include <stdint.h>
#include "edition.h"
#define Z_TRACKS 6
#define Z_VOICES 8
#define Z_STEPS 64
#define Z_PARAMS 27
#ifndef Z_EDITION_PRESETS
#define Z_EDITION_PRESETS 64
#endif
#define Z_PRESETS Z_EDITION_PRESETS
#define Z_SLOT_BYTES 49152u
#define Z_PCM_MAX 24448u
#define Z_RATE 44100
enum { Z_MODEL,Z_WAVE,Z_MIX,Z_DETUNE,Z_CUTOFF,Z_RESONANCE,Z_ATTACK,Z_DECAY,Z_SUSTAIN,Z_RELEASE,
       Z_FM,Z_RATIO,Z_DRIVE,Z_DELAY,Z_REVERB,Z_LEVEL,Z_PAN,Z_LFO_RATE,Z_LFO_DEPTH,Z_SAMPLE,
       Z_ROOT,Z_MUTE,Z_CHANNEL,Z_LENGTH,Z_GATE,Z_TRANSPOSE,Z_PROBABILITY };
typedef struct { uint8_t note,velocity,chance,gate; } z_step;
typedef struct { int16_t p[Z_PARAMS]; z_step steps[Z_STEPS]; } z_track;
typedef struct { uint32_t magic; uint16_t version,bpm; uint8_t swing,selected,language,reserved;
                 z_track track[Z_TRACKS];
                 int8_t octave;
                 uint8_t section,subpage,step_bank;
                 uint8_t arp_mode[6],arp_rate[6],arp_oct[6],arp_hold[6],preset[6];
                 uint8_t recording,step_edit;
                 /* v3 extension: four-note steps and measured note lengths. */
                 uint8_t chord[6][64][3],duration[6][64];
                 uint8_t rec_mode,rec_quant,metronome,fx_amount,fx_rate,padding[3];
               } z_project;
enum { Z_HOME,Z_FX,Z_SEL,Z_ENV,Z_LFO,Z_EDIT,Z_GLO,Z_ARP,Z_SEQ,Z_SECTIONS };
_Static_assert(sizeof(z_project)==3452,"Project v3 wire layout");
typedef struct { const char *name; int16_t min,max,def; } z_descriptor;
extern const z_descriptor z_desc[Z_PARAMS];
extern z_project z_song;
extern const char *const z_models[9];

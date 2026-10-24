/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#define Z_SYSTEM_MAGIC 0x3153595au
typedef struct {uint32_t magic;uint16_t version;uint8_t dual,trim,limiter,glow,velocity,start;
 char name[32],bio[64];uint8_t midi_mode,transport,padding[2];} z_system_settings;
_Static_assert(sizeof(z_system_settings)==112,"System wire layout");
enum {Z_CORE_OFF,Z_CORE_RUNNING,Z_CORE_START_FAILED,Z_CORE_TEST_FAILED,Z_CORE_TIMEOUT,Z_CORE_SIMULATED};

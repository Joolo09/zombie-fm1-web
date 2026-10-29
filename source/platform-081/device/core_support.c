/* SPDX-License-Identifier: MPL-2.0 */
#ifdef APP_SECOND_CORE
#include "fm1_core_worker.h"
volatile uint32_t plat_core_ready_flag;
int plat_core_start(void){return fm1_core_worker_start();}
void plat_core_stop(void){fm1_core_worker_stop();}
uint32_t plat_core_ticks(void){return fm1_ticks();}
#endif

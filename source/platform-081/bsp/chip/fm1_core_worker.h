/* SPDX-License-Identifier: MPL-2.0
 * Original optional CPU1 adapter for Zombie Core, using felucca-bsp register definitions.
 * Peripheral addresses and boot sequence follow the public AC79 SDK/BSP.
 * No SLOOP/X0X application or HAL source is included. */
#pragma once
#ifdef APP_SECOND_CORE
#include "fm1_cpu1.h"
extern volatile uint32_t plat_core_ready_flag;
/* Cutting a core's bus access can set bus-invalid latches. Mask only those traps
 * for the bounded halt operation, clear their latches, and restore the guards. */
static void plat_core_halt(void){
 uint32_t enabled;volatile uint32_t mask=0x003f0030u;fm1__dbg_unlock();enabled=FM1_DBG_EN;
 FM1_DBG_EN=enabled&~mask;__asm__ volatile("csync":::"memory");
 FM1_CPU1_CORE_CON=(FM1_CPU1_CORE_CON|2u)&~8u;
 __asm__ volatile("csync":::"memory");fm1_delay_us(3);
 FM1_DBG_MSG_CLR=mask;__asm__ volatile("csync":::"memory");FM1_DBG_EN=enabled;fm1__dbg_lock();
}
static inline void fm1_core_worker_stop(void){plat_core_halt();plat_core_ready_flag=0;}
static inline int fm1_core_worker_start(void){
 uint32_t pc[4],limits,clock,t0,i;plat_core_halt();plat_core_ready_flag=0;
 fm1__dbg_unlock();pc[0]=FM1_PC_LIMIT0_L;pc[1]=FM1_PC_LIMIT0_H;pc[2]=FM1_PC_LIMIT1_L;pc[3]=FM1_PC_LIMIT1_H;limits=FM1_C0_WR_LIMIT_EN;
 FM1_PC_LIMIT0_L=FM1_PC_LIMIT1_L=0;FM1_PC_LIMIT0_H=FM1_PC_LIMIT1_H=0xffffffffu;FM1_C0_WR_LIMIT_EN=limits&~4u;fm1__dbg_lock();
 for(i=0;i<32;i++)*(volatile uint32_t *)(0x1eef300u+4*i)=0;
 FM1_CPU1_ENTRY_WORD=(uint32_t)(uintptr_t)fm1_cpu1_entry;
 clock=FM1_CPU1_SYS_DIV;FM1_CPU1_SYS_DIV=clock|8u;__asm__ volatile("csync":::"memory");
 FM1_CPU1_CORE_CON=(FM1_CPU1_CORE_CON|8u)&~2u;t0=fm1_ticks();
 while(!plat_core_ready_flag&&(uint32_t)(fm1_ticks()-t0)<480000u)__asm__ volatile("csync":::"memory");
 FM1_CPU1_SYS_DIV=clock;
 fm1__dbg_unlock();FM1_PC_LIMIT0_L=pc[0];FM1_PC_LIMIT0_H=pc[1];FM1_PC_LIMIT1_L=pc[2];FM1_PC_LIMIT1_H=pc[3];FM1_C0_WR_LIMIT_EN=limits;fm1__dbg_lock();
 if(!plat_core_ready_flag){plat_core_halt();return -1;}return 0;
}
static inline __attribute__((always_inline)) void fm1_core_worker_fence(void){__asm__ volatile("csync":::"memory");}
#endif

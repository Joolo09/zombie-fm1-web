/* SPDX-License-Identifier: GPL-3.0-only
 * Integration checks run solely in the host simulator, against real engine state.
 */
static void h_test_blocks(uint32_t count){float out[512];while(count--)h_render(out,256);}
static void h_test_pattern(void){h_init();for(uint32_t k=0;k<8;k++){trk[k].p[P_SRC]=0;trk[k].p[P_SLEN]=16;memset(trk[k].step,0,sizeof trk[k].step);}song.g[G_CLOCK]=0;song.g[G_BPM]=120;}
int h_host_check(void){
 h_test_pattern();trk[0].step[0].on=1;h_transport(1);h_test_blocks(20);uint32_t plain=dvage;if(!plain)return 1;
 h_test_pattern();trk[0].step[0].on=1;trk[0].step[0].rat=3;h_transport(1);h_test_blocks(20);if(dvage<=plain)return 2;
 h_test_pattern();trk[0].step[0].on=1;trk[0].step[0].cond=1;h_transport(1);h_test_blocks(20);if(dvage)return 3;
 h_test_pattern();trk[0].p[P_SRC]=1;song.g[G_GMODE]=1;song.g[G_GFILL1]=127;h_transport(1);h_test_blocks(80);if(!dvage)return 4;
 h_test_pattern();int base=trk[0].p[P_E0];mo.s.count=1;mo.s.on=1;mo.s.ev[0]=(motion_event_t){0,0,P_E0,7};h_transport(1);h_test_blocks(1);if(trk[0].p[P_E0]!=7)return 5;h_transport(0);h_test_blocks(1);if(trk[0].p[P_E0]!=base)return 6;
 h_test_pattern();if(h_set(0,P_MODEL,DM_ACID)||h_set(1,P_MODEL,DM_ACID)||h_set(2,P_MODEL,DM_ACID)!=-1)return 7;
 for(uint32_t i=0;i<4;i++)if(h_set(i,P_RMODEL,RS_STRNG))return 8;
 if(h_set(4,P_RMODEL,RS_STRNG)!=-1)return 9;
 h_test_pattern();if(preset_user_save(127,&trk[0]))return 10;h_init();preset_t user;if(!preset_get(PRE_NFACT+127,&user))return 11;
 h_test_pattern();for(uint32_t slot=0;slot<4;slot++){trk[0].p[P_E0]=(int16_t)(slot+3);project_save(slot);if(!project_used(slot))return 12;}
 for(uint32_t slot=0;slot<4;slot++){project_load(slot);if(trk[0].p[P_E0]!=(int)(slot+3))return 13;transport_req=0;}
 chain.cfg.count=2;chain.cfg.loop=0;chain.cfg.row[0]=(chain_row_t){0,1};chain.cfg.row[1]=(chain_row_t){1,1};
 if(chain_prepare())return 14;h_test_blocks(1);if(!chain.running)return 15;h_test_blocks(1500);if(chain.running||song.playing)return 16;
 h_init();ui.menu=1;ui.menu_sel=MI_PANEL;menu_input(1u<<panel.btn[B_OCTUP]);ui.menu=0;return 0;
}

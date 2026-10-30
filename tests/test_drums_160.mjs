/* SPDX-License-Identifier: MIT — 0.16.0 drum grid (Felucca-style): toggle on release, hold a step + knob edits it, beat marks. */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208),hit=(e,d,at)=>kit(e).slice(136+d*192+at*3,139+d*192+at*3);
const drums=()=>{const e=create();e.turn(1,5);e.tap('SEQ');return e;};
const down=(e,k)=>{e.w.web_key(k,100,0);e.run(60);},up=(e,k)=>{e.w.web_key(k,0,0);e.run(40);};

check('a tap toggles the step on release, a second tap clears it',()=>{const e=drums();down(e,WHITE[2]);assert.equal(hit(e,0,2)[0],0,'not yet on press');
 up(e,WHITE[2]);assert.equal(hit(e,0,2)[0],100);e.key(WHITE[2]);assert.equal(hit(e,0,2)[0],0);});

check('pressing a step plays the selected sound at once',()=>{const e=drums();down(e,WHITE[0]);assert(e.run(60)>.001,'kick heard while the key is down');up(e,WHITE[0]);});

check('holding a lit step and turning K3 changes its velocity and keeps the hit',()=>{const e=drums();e.key(WHITE[4]);assert.equal(hit(e,0,4)[0],100);
 down(e,WHITE[4]);e.turn(5,-30);up(e,WHITE[4]);assert.equal(hit(e,0,4)[0],70);});

check('holding an empty step and turning K4 creates the hit with that roll',()=>{const e=drums();down(e,WHITE[6]);e.turn(6,2);up(e,WHITE[6]);
 assert.deepEqual(Array.from(hit(e,0,6)),[100,127,3]);});

check('one undo step puts a held-step edit back',()=>{const e=drums();e.key(WHITE[4]);down(e,WHITE[4]);e.turn(5,-50);up(e,WHITE[4]);assert.equal(hit(e,0,4)[0],50);
 e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.equal(hit(e,0,4)[0],100);});

check('steps 1, 5, 9 and 13 glow softly on the drum grid and on the piano roll (the lit cursor key wins)',()=>{for(const track of [5,0]){const e=create();e.turn(1,track);e.tap('SEQ');e.run(100);
 const dim=e.w.web_dim_keys();for(const j of (track===5?[0,4,8,12]:[4,8,12]))assert(dim&(1<<WHITE[j]),'track '+(track+1)+' beat '+(j+1));for(const j of [1,2,3,5])assert(!(dim&(1<<WHITE[j])),'off-beat '+(j+1));}});

check('beat marks stop at the loop end',()=>{const e=drums();e.hold('SEQ');e.run(450);e.turn(3,-24);e.up('SEQ');e.run(100);   // 8 steps
 const dim=e.w.web_dim_keys();assert(dim&(1<<WHITE[0])&&dim&(1<<WHITE[4]));assert(!(dim&(1<<WHITE[8])));});

if(failed)process.exit(1);

/* SPDX-License-Identifier: MIT — 0.16.0 recording control (Felucca-style): count-in, live loop from any page, deferred SAVE. */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const SECTION=1873,RECORDING=1906;
const barMs=e=>4*60000/(new DataView(e.root().buffer,e.root().byteOffset).getUint16(6,true));

check('REC on HOME while stopped counts in one bar, then plays and records live',()=>{const e=create();assert.equal(e.root()[SECTION],0);
 e.tap('REC');assert.equal(e.w.z_countin_state(),4);assert.equal(e.w.z_playing(),0);assert.equal(e.root()[RECORDING],0);assert.equal(e.root()[SECTION],8,'shows the piano roll');
 e.run(barMs(e)/2);assert(e.w.z_countin_state()<=2&&e.w.z_countin_state()>=1,'counting down');
 e.run(barMs(e)/2+80);assert.equal(e.w.z_countin_state(),0);assert.equal(e.w.z_playing(),1);assert.equal(e.root()[RECORDING],1);
 e.key(WHITE[0]);e.run(300);assert(e.w.z_track_hits(0)>=1,'a live note was recorded');
 e.rec();assert.equal(e.root()[RECORDING],0,'REC ends the take');assert.equal(e.w.z_playing(),1,'and the loop keeps playing');e.tap('PLAY');});

check('REC during the count-in cancels it; nothing plays or records',()=>{const e=create();e.tap('REC');e.run(300);e.tap('REC');
 assert.equal(e.w.z_countin_state(),0);e.run(barMs(e));assert.equal(e.w.z_playing(),0);assert.equal(e.root()[RECORDING],0);});

check('PLAY during the count-in starts the take at once',()=>{const e=create();e.tap('REC');e.run(200);e.tap('PLAY');
 assert.equal(e.w.z_countin_state(),0);assert.equal(e.w.z_playing(),1);assert.equal(e.root()[RECORDING],1);e.tap('PLAY');assert.equal(e.root()[RECORDING],0);});

check('REC on the SEQ page while stopped is still step entry at the cursor',()=>{const e=create();e.tap('SEQ');e.tap('REC');
 assert.equal(e.w.z_countin_state(),0);assert.equal(e.root()[RECORDING],1);assert.equal(e.w.z_playing(),0);e.key(WHITE[0]);assert.equal(e.root()[1907],1);});

check('REC while playing overdubs immediately, no count-in',()=>{const e=create();e.tap('PLAY');e.run(200);e.tap('REC');
 assert.equal(e.w.z_countin_state(),0);assert.equal(e.root()[RECORDING],1);e.tap('PLAY');});

check('drum track: REC on HOME counts in and records pads live',()=>{const e=create();e.turn(1,5);e.tap('REC');assert.equal(e.w.z_countin_state(),4);
 e.run(barMs(e)+100);assert.equal(e.root()[RECORDING],1);e.key(WHITE[0]);e.run(200);assert(e.w.z_track_hits(5)>=1);e.tap('PLAY');});

check('COUNT-IN off in the REC layer starts the take at once; the setting survives SAVE and power-cycle',()=>{const e=create();
 e.hold('REC');e.tap('HOME');e.up('REC');e.key(WHITE[13]);e.tap('HOME');   // COUNT-IN: 1 bar -> 2 bars
 e.hold('REC');e.tap('HOME');e.up('REC');e.key(WHITE[13]);e.tap('HOME');   // 2 bars -> off
 e.tap('HOME');e.tap('REC');assert.equal(e.w.z_countin_state(),0);assert.equal(e.root()[RECORDING],1);assert.equal(e.w.z_playing(),1);e.tap('PLAY');
 e.tap('SAVE');e.run(300);e.w.web_reset();e.run(200);e.tap('REC');assert.equal(e.w.z_countin_state(),0,'still off after reboot');e.tap('PLAY');});

check('two-bar count-in lasts eight beats',()=>{const e=create();e.hold('REC');e.tap('HOME');e.up('REC');e.key(WHITE[13]);e.tap('HOME');
 e.tap('HOME');e.tap('REC');assert.equal(e.w.z_countin_state(),8);e.tap('REC');});

check('REC + HOME opens the REC layer without leaving a count-in running',()=>{const e=create();e.hold('REC');e.tap('HOME');e.up('REC');
 assert.equal(e.w.z_countin_state(),0);e.run(barMs(e)+100);assert.equal(e.w.z_playing(),0);e.tap('HOME');});

check('SAVE while playing keeps the music going and saves on STOP',()=>{const e=create();e.tap('SEQ');e.key(WHITE[0]);e.tap('PLAY');e.run(200);
 const writes=e.w.web_store_writes();e.tap('SAVE');e.run(300);assert.equal(e.w.z_playing(),1,'still playing');assert.equal(e.w.web_store_writes(),writes,'nothing written yet');
 e.tap('PLAY');e.run(400);assert(e.w.web_store_writes()>writes,'saved after STOP');e.w.web_reset();e.run(200);assert.equal(e.w.z_track_hits(0),1);});

if(failed)process.exit(1);

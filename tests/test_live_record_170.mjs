/* SPDX-License-Identifier: MIT — live recording lands on the right steps: nearest/floor quantise, early and late hits, count-in start. */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const STEP_MS=125;   // 120 BPM, 1/16
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208);
const steps=(e,t)=>Array.from({length:64},(_,i)=>e.root()[12+t*310+54+i*4+1]?1:0);
const lane=(e,d)=>Array.from({length:64},(_,i)=>kit(e)[136+d*192+i*3]?1:0);
const startTake=(e,quant)=>{e.w.z_tempo(120,0);e.root()[1909]=0;                // metronome field irrelevant here
 if(quant!==undefined){e.hold('REC');e.tap('HOME');e.up('REC');e.key(WHITE[quant?2:3]);e.tap('HOME');}   // REC layer: tile 3 = NEAREST, 4 = FLOOR (index 2/3)
 e.tap('REC');for(let i=0;i<4000&&!(e.w.z_playing()&&e.root()[1906]);i++)e.run(10);assert(e.w.z_playing()&&e.root()[1906],'take started after the count-in');};
const untilStep=(e,s)=>{for(let i=0;i<4000&&e.w.z_position()%64!==s;i++)e.run(1);assert.equal(e.w.z_position()%64,s,'reached step '+s);};
const hitAt=(e,s,offsetMs,drumKey)=>{untilStep(e,(s+63)%64);if(offsetMs>=0){e.run(STEP_MS+offsetMs-8);}else{e.run(STEP_MS+offsetMs-8);}   // -8 ms: the loop's own granularity
 e.w.web_key(drumKey,100,0);e.run(40);e.w.web_key(drumKey,0,0);};

check('hits exactly on the beat land on their steps (melody)',()=>{const e=create();e.root();startTake(e);for(const s of [4,8,12,16])hitAt(e,s,0,WHITE[0]);e.tap('REC');
 const got=steps(e,0);const on=got.map((v,i)=>v?i:-1).filter(i=>i>=0);assert.equal(on.length,4,'four notes: '+on);assert.deepEqual(on,[4,8,12,16]);});

check('with NEAREST a hit 30 ms early or late still lands on the beat; 70 ms late goes to the next step',()=>{
 for(const [off,expectDelta] of [[-30,0],[30,0],[70,1]]){const e=create();startTake(e);hitAt(e,8,off,WHITE[0]);e.tap('REC');const on=steps(e,0).map((v,i)=>v?i:-1).filter(i=>i>=0);
  assert.deepEqual(on,[8+expectDelta],'offset '+off+' ms');}});

check('with FLOOR a hit never moves forward: 70 ms late stays on the step, 30 ms early drops to the step before',()=>{
 for(const [off,want] of [[70,8],[30,8],[-30,7]]){const e=create();protocol(e)(21,[10,1]);startTake(e);hitAt(e,8,off,WHITE[0]);e.tap('REC');
  assert.deepEqual(steps(e,0).map((v,i)=>v?i:-1).filter(i=>i>=0),[want],'offset '+off+' ms');}});

check('the same on the drum track: kick hits within 25 ms of the beat land on 1, 5, 9, 13',()=>{const e=create();e.turn(1,5);startTake(e);
 for(const [s,off] of [[4,-20],[8,15],[12,0],[16,25]])hitAt(e,s,off,WHITE[0]);e.tap('REC');const on=lane(e,0).map((v,i)=>v?i:-1).filter(i=>i>=0);
 assert.deepEqual(on,[4,8,12,16]);});

check('a take started from the count-in begins at step 1: the first beat after it is step 1, not step 2',()=>{const e=create();startTake(e);
 for(let i=0;i<300&&e.w.z_position()===0xffffffff;i++)e.run(5);const p=e.w.z_position();assert(p<=1,'first position '+p);});

check('the loop wraps: a second pass overdubs on top of the first without losing it',()=>{const e=create();startTake(e);e.turn(5,0);
 hitAt(e,5,0,WHITE[0]);hitAt(e,9+32,0,WHITE[2]);   // the second lap of the 32-step loop
 e.tap('REC');const on=steps(e,0).map((v,i)=>v?i:-1).filter(i=>i>=0);assert.deepEqual(on,[5,9],'both laps kept');});

if(failed)process.exit(1);

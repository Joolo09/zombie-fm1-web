/* SPDX-License-Identifier: MIT — 0.15.0: SEQ+K1 sets the step count of a track, and of the whole drum kit. */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208),lengths=e=>Array.from({length:16},(_,d)=>kit(e)[8+d*8+5]);
const seqK1=(e,n)=>{e.hold('SEQ');e.run(450);e.turn(3,n);e.up('SEQ');};
const LENGTH=23;

check('holding SEQ and turning K1 changes the selected synth track step count only',()=>{const e=create();e.tap('SEQ');assert.equal(e.w.z_get(0,LENGTH),32);const step=e.root()[1907];
 seqK1(e,8);assert.equal(e.w.z_get(0,LENGTH),40);assert.equal(e.root()[1907],step,'K1 under SEQ must not move the cursor');for(let t=1;t<5;t++)assert.equal(e.w.z_get(t,LENGTH),32);
 seqK1(e,-100);assert.equal(e.w.z_get(0,LENGTH),1);seqK1(e,200);assert.equal(e.w.z_get(0,LENGTH),64);});

check('on track 6 SEQ+K1 sets one length for all 16 drum sounds',()=>{const e=create();e.turn(1,5);e.tap('SEQ');assert.deepEqual(lengths(e),Array(16).fill(32));
 seqK1(e,-16);assert.deepEqual(lengths(e),Array(16).fill(16));seqK1(e,48);assert.deepEqual(lengths(e),Array(16).fill(64));});

check('drum kit length is one undo step and redo restores it',()=>{const e=create();e.turn(1,5);e.tap('SEQ');seqK1(e,-16);assert.deepEqual(lengths(e),Array(16).fill(16));
 e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.deepEqual(lengths(e),Array(16).fill(32));e.hold('EDIT');e.tap('UP');e.up('EDIT');assert.deepEqual(lengths(e),Array(16).fill(16));});

check('mixed per-sound lengths are unified from the selected sound',()=>{const e=create();e.turn(1,5);e.tap('SEQ');
 e.w.z_dm_param(0,5,12);e.w.z_dm_param(3,5,24);assert.equal(e.w.z_dm_common_length(),0);
 seqK1(e,4);assert.deepEqual(lengths(e),Array(16).fill(16),'kick (selected) 12 + 4');assert.equal(e.w.z_dm_common_length(),16);});

check('REC layer LEN 16/32/64 tiles set the whole kit and synth tracks',()=>{for(const track of [0,5]){const e=create();e.turn(1,track);e.tap('SEQ');
 for(const [tile,len] of [[6,16],[8,64],[7,32]]){e.hold('REC');e.tap('HOME');e.up('REC');e.key(WHITE[tile]);e.tap('HOME');
  if(track===5)assert.deepEqual(lengths(e),Array(16).fill(len));else assert.equal(e.w.z_get(0,LENGTH),len);}}});

check('a 16-step kit loops every drum sound on 16 steps while playing',()=>{const e=create();e.turn(1,5);e.tap('SEQ');seqK1(e,-16);e.w.z_tempo(240,0);e.tap('PLAY');e.run(5200);
 const at=Array.from({length:16},(_,d)=>e.w.z_dm_position(d));assert(at.every(p=>p===at[0]&&p<16),'positions '+at);e.tap('PLAY');});

check('step count survives SAVE and power-cycle',()=>{const e=create();seqK1(e,-8);e.turn(1,5);seqK1(e,32);e.tap('SAVE');e.w.web_reset();e.run(150);
 assert.equal(e.w.z_get(0,LENGTH),24);assert.deepEqual(lengths(e),Array(16).fill(64));});

check('a step past the loop can still be placed for a later, longer loop',()=>{const e=create();e.tap('SEQ');seqK1(e,-16);e.turn(2,1);e.key(WHITE[0]);
 assert.equal(e.root()[1907],16);assert.equal(e.w.z_track_hits(0),1);});

if(failed)process.exit(1);

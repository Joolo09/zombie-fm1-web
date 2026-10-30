/* SPDX-License-Identifier: MIT — 0.17.0: undo and redo go eight steps deep (was four), as a ring that wraps correctly. */
import assert from 'node:assert/strict';import {create,WHITE,BLACK} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const undo=e=>{e.hold('EDIT');e.tap('DOWN');e.up('EDIT');},redo=e=>{e.hold('EDIT');e.tap('UP');e.up('EDIT');};
const hits=(e,t)=>e.w.z_track_hits(t);

for(const track of [0,5])check('track '+(track+1)+': twenty edits, exactly eight undos reach back, the ninth does nothing',()=>{const e=create();e.turn(1,track);e.tap('SEQ');
 for(let i=0;i<16;i++)e.key(WHITE[i]);e.turn(2,1);for(let i=0;i<4;i++)e.key(WHITE[i]);   // twenty hits in two banks
 assert.equal(hits(e,track),20);for(let i=0;i<8;i++)undo(e);assert.equal(hits(e,track),12);for(let i=0;i<4;i++)undo(e);assert.equal(hits(e,track),12,'nothing beyond eight steps');});

for(const track of [0,5])check('track '+(track+1)+': undo 8, redo 8 returns to the exact state',()=>{const e=create();e.turn(1,track);e.tap('SEQ');for(let i=0;i<12;i++)e.key(WHITE[i]);
 const snap=track===5?Array.from(new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208)):Array.from(e.root());
 for(let i=0;i<8;i++)undo(e);for(let i=0;i<8;i++)redo(e);
 const now=track===5?Array.from(new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208)):Array.from(e.root());
 // the cursor and transient fields may differ; the pattern data must not
 const same=track===5?JSON.stringify(now)===JSON.stringify(snap):JSON.stringify(now.slice(12,1860))===JSON.stringify(snap.slice(12,1860));assert(same,'pattern restored');});

for(const track of [0,5])check('track '+(track+1)+': a new edit after an undo drops the redo branch',()=>{const e=create();e.turn(1,track);e.tap('SEQ');for(let i=0;i<5;i++)e.key(WHITE[i]);undo(e);undo(e);const was=hits(e,track);e.key(WHITE[10]);redo(e);redo(e);assert.equal(hits(e,track),was+1,'redo is gone');});

check('SEQ tools share the undo ring: three HALF presses undo one at a time',()=>{const e=create();e.tap('SEQ');e.hold('SEQ');e.run(450);e.turn(2,3);
 const lens=[];for(let i=0;i<3;i++){e.key(BLACK[2]);lens.push(e.w.z_get(0,23));}assert.deepEqual(lens,[16,8,4]);
 e.up('SEQ');undo(e);assert.equal(e.w.z_get(0,23),8);undo(e);undo(e);assert.equal(e.w.z_get(0,23),32);});

if(failed)process.exit(1);

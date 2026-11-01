/* SPDX-License-Identifier: MIT — 0.20.1: a held step turned by the knobs is one undo step, on the roll and on the drum grid. */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const dm=e=>[...new Uint8Array(e.memory.buffer,e.w.z_dm_data(),e.w.z_dm_size())];
const undo=e=>{e.hold('EDIT');e.tap('DOWN');e.up('EDIT');e.run(100);},redo=e=>{e.hold('EDIT');e.tap('UP');e.up('EDIT');e.run(100);};
const setup=drum=>{const e=create();e.run(300);if(drum){e.turn(1,5);e.run(100);}e.tap('SEQ');e.run(100);return e;};
const holdTurn=e=>{e.w.web_key(WHITE[2],100,0);e.run(60);e.turn(4,5);e.turn(5,3);e.turn(6,1);e.w.web_key(WHITE[2],0,0);e.run(100);};
for(const drum of [false,true]){const name=drum?'drum grid':'piano roll',get=e=>drum?dm(e):[...e.root()].slice(12,322); /* track 1 (params and steps); undo keeps the cursor where it is */
 check(name+': an empty step held and turned comes back empty with one undo, and redo brings the edit back',()=>{const e=setup(drum);const a=get(e);holdTurn(e);const b=get(e);assert.notDeepEqual(a,b);undo(e);assert.deepEqual(get(e),a);redo(e);assert.deepEqual(get(e),b);});
 check(name+': an existing step held and turned gets its old values back with one undo',()=>{const e=setup(drum);e.key(WHITE[2]);const a=get(e);holdTurn(e);assert.notDeepEqual(get(e),a);undo(e);assert.deepEqual(get(e),a);});
 check(name+': a plain tap is still one undo step',()=>{const e=setup(drum);const a=get(e);e.key(WHITE[6]);undo(e);assert.deepEqual(get(e),a);});}
if(failed)process.exit(1);

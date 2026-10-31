/* SPDX-License-Identifier: MIT — 0.17.0 SEQ tools page (hold SEQ, PRESET to page 4): EUCLID, RANDOM/BEAT, HALF, COOK. */
import assert from 'node:assert/strict';import {create,BLACK} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const LEN=23,melody=(e,t=0)=>Array.from({length:64},(_,i)=>e.root()[12+t*310+54+i*4+1]!==0?1:0);
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208),lane=(e,d=0)=>Array.from({length:64},(_,i)=>kit(e)[136+d*192+i*3]?1:0);
const lengths=e=>Array.from({length:16},(_,d)=>kit(e)[8+d*8+5]);
const toolsOpen=e=>{e.hold('SEQ');e.run(450);e.turn(2,3);};            // page 4 of the SEQ layer
const tool=(e,j)=>{e.key(BLACK[j]);};
const sample=(len,hits,rot)=>Array.from({length:len},(_,i)=>{const k=(i+len-rot%len)%len;return hits&&((k*hits)%len)<hits?1:0;});

check('EUCLID spreads the hits evenly: E(3,8) is x..x..x.',()=>{const e=create();e.tap('SEQ');toolsOpen(e);e.turn(5,-24);e.turn(3,-1);   // 8 steps, 3 hits
 tool(e,0);assert.deepEqual(melody(e).slice(0,8),[1,0,0,1,0,0,1,0]);assert.equal(e.w.z_get(0,LEN),8);});

check('EUCLID matches the formula for every hits/steps/rotation, on melody and on the drum lane',()=>{
 for(const [len,hits,rot] of [[16,4,0],[16,5,3],[8,5,2],[12,7,5],[32,9,1],[13,5,0],[16,0,0],[16,16,0]]){
  const e=create();e.tap('SEQ');toolsOpen(e);e.turn(5,len-32);e.turn(3,hits-4);e.turn(4,rot);tool(e,0);
  assert.deepEqual(melody(e).slice(0,len),sample(len,hits,rot),'melody '+len+','+hits+','+rot);
  const d=create();d.turn(1,5);d.tap('SEQ');toolsOpen(d);d.turn(5,len-32);d.turn(3,hits-4);d.turn(4,rot);tool(d,0);
  assert.deepEqual(lane(d).slice(0,len),sample(len,hits,rot),'drums '+len+','+hits+','+rot);}});

check('EUCLID keeps the cursor step\'s pitch for new hits and clears the rest',()=>{const e=create();e.tap('SEQ');e.key(8);e.key(0);   // two notes on the roll
 toolsOpen(e);tool(e,0);const notes=Array.from({length:32},(_,i)=>e.root()[12+54+i*4]);const on=melody(e);assert.equal(on.reduce((a,b)=>a+b),4);
 for(let i=0;i<32;i++)if(on[i])assert.equal(notes[i],notes[Array.from(on).findIndex(x=>x)],'step '+i);});

check('K1/K2 on the tools page are HITS and ROTATE, not the cursor',()=>{const e=create();e.tap('SEQ');const cursor=e.root()[1907];toolsOpen(e);e.turn(3,2);e.turn(4,1);assert.equal(e.root()[1907],cursor);
 e.up('SEQ');e.run(100);});

check('the first pages of the SEQ layer are unchanged (K1 is still STEPS)',()=>{const e=create();e.tap('SEQ');e.hold('SEQ');e.run(450);e.turn(3,-8);assert.equal(e.w.z_get(0,LEN),24);e.up('SEQ');});

check('HALF cuts the loop to its first half: melody 32 -> 16, kit 32 -> 16',()=>{const e=create();e.tap('SEQ');toolsOpen(e);tool(e,2);assert.equal(e.w.z_get(0,LEN),16);tool(e,2);assert.equal(e.w.z_get(0,LEN),8);
 const d=create();d.turn(1,5);d.tap('SEQ');toolsOpen(d);tool(d,2);assert.deepEqual(lengths(d),Array(16).fill(16));});

check('HALF stops at 1 step',()=>{const e=create();e.tap('SEQ');toolsOpen(e);e.turn(5,-31);tool(e,2);assert.equal(e.w.z_get(0,LEN),1);tool(e,2);assert.equal(e.w.z_get(0,LEN),1);});

check('RANDOM writes a melodic pattern; BEAT on the drum track writes the preset beat',()=>{const e=create();e.tap('SEQ');toolsOpen(e);tool(e,1);assert(melody(e).reduce((a,b)=>a+b)>0);
 const d=create();d.turn(1,5);d.tap('SEQ');toolsOpen(d);tool(d,1);assert(lane(d).reduce((a,b)=>a+b)>0);});

check('COOK changes at most two steps per press, keeps pitches from the pattern and is one undo step',()=>{const e=create();e.tap('SEQ');toolsOpen(e);e.turn(3,0);tool(e,0);const before=melody(e),notes0=new Set(Array.from({length:32},(_,i)=>e.root()[12+54+i*4]));
 let changed=0;for(let n=0;n<12;n++){const was=melody(e);tool(e,3);const now=melody(e);const diff=now.reduce((a,v,i)=>a+(v!==was[i]?1:0),0);assert(diff<=2,'press '+n+' changed '+diff);changed+=diff;}
 assert(changed>0,'12 presses changed something');
 for(let i=0;i<32;i++)if(melody(e)[i])assert(notes0.has(e.root()[12+54+i*4]),'new step has a pitch from the pattern');
 const after=melody(e);let prev;for(let n=0;n<6&&!prev;n++){const was=melody(e);tool(e,3);if(JSON.stringify(melody(e))!==JSON.stringify(was))prev=was;}
 const now=melody(e);e.up('SEQ');e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.deepEqual(melody(e),prev,'undo reverts exactly the last COOK');void after;void now;});

check('COOK on the drum lane flips steps and accents without leaving the lane',()=>{const d=create();d.turn(1,5);d.tap('SEQ');toolsOpen(d);tool(d,0);const before=lane(d);for(let n=0;n<10;n++)tool(d,3);
 assert.notDeepEqual(lane(d),before);for(let l=1;l<16;l++)assert.equal(lane(d,l).reduce((a,b)=>a+b),0,'only the selected sound changed');});

check('EUCLID is one undo step',()=>{const e=create();e.tap('SEQ');e.key(0);toolsOpen(e);tool(e,0);assert(melody(e).reduce((a,b)=>a+b)>1);e.up('SEQ');e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.equal(melody(e).reduce((a,b)=>a+b),1);});

if(failed)process.exit(1);

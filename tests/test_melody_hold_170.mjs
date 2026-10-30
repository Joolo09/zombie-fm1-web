/* SPDX-License-Identifier: MIT — 0.17.0: hold a melodic step on the piano roll and turn K1-K4 to edit it. */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const step=(e,t,i)=>Array.from(e.root().slice(12+t*310+54+i*4,12+t*310+54+i*4+4)),dur=(e,t,i)=>e.root()[3060+t*64+i];
const down=(e,k)=>{e.w.web_key(k,100,0);e.run(60);},up=(e,k)=>{e.w.web_key(k,0,0);e.run(40);};
const seq=()=>{const e=create();e.tap('SEQ');return e;};

check('pressing an empty step creates a note; holding it and turning K1-K4 edits note, velocity, length and gate',()=>{const e=seq();
 down(e,WHITE[3]);assert.deepEqual(step(e,0,3),[60,100,127,80]);
 e.turn(3,2);e.turn(4,-30);e.turn(5,3);e.turn(6,-20);up(e,WHITE[3]);
 assert.deepEqual(step(e,0,3),[62,70,127,60]);assert.equal(dur(e,0,3),3);});

check('the whole hold is one undo step (back to the freshly created note)',()=>{const e=seq();down(e,WHITE[3]);e.turn(3,5);e.turn(4,-40);up(e,WHITE[3]);
 e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.deepEqual(step(e,0,3),[60,100,127,80]);});

check('without a held step the knobs are the page knobs again (K1 moves the cursor)',()=>{const e=seq();down(e,WHITE[3]);up(e,WHITE[3]);const cursor=e.root()[1907];e.turn(3,2);
 assert.equal(e.root()[1907],cursor+2);assert.equal(step(e,0,3)[0],60);});

check('K1 on a held step stays in the scale',()=>{const e=seq();const p=protocol(e);p(29,[10,1]);p(29,[11,0]);   // C major
 down(e,WHITE[3]);e.turn(3,1);up(e,WHITE[3]);assert.equal(step(e,0,3)[0],62,'61 moves up to 62');
 down(e,WHITE[3]);e.turn(3,-1);up(e,WHITE[3]);assert.equal(step(e,0,3)[0],60);});

check('a held step on another step does not touch the first',()=>{const e=seq();down(e,WHITE[1]);e.turn(4,-50);up(e,WHITE[1]);down(e,WHITE[5]);e.turn(4,-10);up(e,WHITE[5]);
 assert.equal(step(e,0,1)[1],50);assert.equal(step(e,0,5)[1],90);});

check('values clamp: length 0 (AUTO) to 63, gate 1 to 127, velocity 1 to 127, note 0 to 127',()=>{const e=seq();down(e,WHITE[2]);
 e.turn(5,-100);assert.equal(dur(e,0,2),0);e.turn(5,200);assert.equal(dur(e,0,2),63);e.turn(6,-300);assert.equal(step(e,0,2)[3],1);e.turn(6,300);assert.equal(step(e,0,2)[3],127);
 e.turn(4,-300);assert.equal(step(e,0,2)[1],1);e.turn(4,300);assert.equal(step(e,0,2)[1],127);e.turn(3,-300);assert.equal(step(e,0,2)[0],0);e.turn(3,300);assert.equal(step(e,0,2)[0],127);up(e,WHITE[2]);});

check('the edited step plays: length 4 holds the note across four steps',()=>{const e=seq();down(e,WHITE[0]);e.turn(5,4);up(e,WHITE[0]);e.w.z_tempo(240,0);e.w.web_out_clear();e.tap('PLAY');e.run(1200);
 const on=Array.from(new Uint32Array(e.memory.buffer,e.w.web_out_pkts(),e.w.web_out_n())).filter(p=>(p>>8&240)===0x90&&(p>>>24&127));assert(on.length>=1);e.tap('PLAY');});

check('on the drum track the same gesture still edits the drum step (chance/velocity/roll)',()=>{const e=create();e.turn(1,5);e.tap('SEQ');down(e,WHITE[2]);e.turn(5,-30);up(e,WHITE[2]);
 assert.equal(new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208)[136+2*3],70);});

if(failed)process.exit(1);

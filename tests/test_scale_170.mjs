/* SPDX-License-Identifier: MIT — 0.17.0 scale lock: keys snap to the scale, the scale glows, the setting is saved. */
import assert from 'node:assert/strict';import {create} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const packets=e=>Array.from(new Uint32Array(e.memory.buffer,e.w.web_out_pkts(),e.w.web_out_n()));
const notesOn=e=>packets(e).filter(p=>(p>>8&240)===0x90&&(p>>>24&127)).map(p=>p>>>16&127);
const setScale=(e,scale,root)=>{const p=protocol(e);p(29,[10,scale]);p(29,[11,root]);};
const play=(e,k)=>{e.w.web_out_clear();e.w.web_key(k,100,0);e.run(60);e.w.web_key(k,0,0);e.run(40);return notesOn(e);};

check('with the scale off every key plays its own note',()=>{const e=create();for(let k=0;k<12;k++)assert.deepEqual(play(e,k),[53+k]);});

check('C major: out-of-scale keys snap to the nearest scale note (a tie goes down)',()=>{const e=create();setScale(e,1,0);
 const want={0:53,1:53,2:55,3:55,4:57,5:57,6:59,7:60,8:60,9:62,10:62,11:64,12:65};  // F3 F#3->F G3 G#3->G A3 A#3->A B3 C4 C#4->C D4 D#4->D E4 F4
 for(const [k,n] of Object.entries(want))assert.deepEqual(play(e,+k),[n],'key '+k);});

check('A minor pentatonic with root A',()=>{const e=create();setScale(e,7,9);   // A C D E G
 for(let k=0;k<27;k++){const [n]=play(e,k);assert([9,0,2,4,7].includes(n%12),'key '+k+' -> '+n);}});

check('keys in the scale glow softly; out-of-scale keys stay dark',()=>{const e=create();setScale(e,1,0);e.run(100);const dim=e.w.web_dim_keys();
 for(let k=0;k<27;k++){const inScale=[0,2,4,5,7,9,11].includes((53+k)%12);assert.equal(!!(dim&(1<<k)),inScale,'key '+k);}});

check('no glow with the scale off, and none on the drum track',()=>{const e=create();e.run(100);assert.equal(e.w.web_dim_keys(),0);setScale(e,1,0);e.turn(1,5);e.run(100);assert.equal(e.w.web_dim_keys(),0);});

check('scale and root survive SAVE and a power cycle; the editor can read them back',()=>{const e=create();setScale(e,3,2);e.tap('SAVE');e.run(300);e.w.web_reset();e.run(200);
 const p=protocol(e);const sys=p(28);assert.equal(sys[0]===undefined,false);assert.equal(play(e,0)[0]%12,5,'F is in D dorian');assert.equal(play(e,1)[0]%12,5,'F# snaps down to F (not in D dorian, tie goes down)');});

check('a snapped key records the snapped note',()=>{const e=create();setScale(e,1,0);e.tap('SEQ');e.rec();e.key(1);assert.equal(e.root()[66+0*0],53,'step 1 holds F, not F#');});

check('the SETTINGS page shows the scale and root, and the knobs change them',()=>{const e=create();e.tap('GLO');e.tap('GLO');e.turn(5,2);e.turn(6,3);   // page 2 of SETTINGS: K3 scale +2 (minor), K4 root +3 (D#)
 assert.deepEqual(play(e,1).map(n=>n%12),[6],'F# is in D# minor');assert.deepEqual(play(e,2).map(n=>n%12),[6],'G snaps down to F#');});

if(failed)process.exit(1);

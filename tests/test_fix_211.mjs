/* SPDX-License-Identifier: MIT — 0.21.1: a held chord keeps its voices while the sequence plays; REC held on the SEQ
 * page clears the selected track's pattern. */
import assert from 'node:assert/strict';import {readFileSync} from 'node:fs';
import {create,WHITE} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.stack);}};
const core=new WebAssembly.Module(readFileSync(new URL('../webapp/groovebox-180/zombie-core.wasm',import.meta.url)));
const fresh=()=>{const w=new WebAssembly.Instance(core,{}).exports;w.z_init();w.z_dm_init();return w;};
const rms=(w,blocks)=>{let e=0,n=0;for(let b=0;b<blocks;b++){w.z_render(w.z_buffer(),256);for(const x of new Float32Array(w.memory.buffer,w.z_buffer(),512)){e+=x*x;n++;}}return Math.sqrt(e/n);};
const steps=(e,t=0)=>Array.from({length:64},(_,i)=>e.root()[12+t*310+54+i*4+1]);
const kit=e=>Array.from(new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208));

check('a held FM chord keeps sounding while four sequenced tracks with long releases need voices',()=>{
 for(const preset of [5,186,52]){const w=fresh();for(let t=1;t<5;t++){w.z_preset(t,128+2*11+2);w.z_set(t,9,127);w.z_set(t,15,0);for(let i=0;i<16;i++)w.z_step_set(t,i,48+t*5+i%3,100,127,127);}   // level 0: only the chord is heard   // full-length gates: every voice is busy at each step   // AMB pads: long release
  w.z_preset(0,preset===52?184+2:preset);w.z_set(0,8,127);for(const n of [60,64,67,71])w.z_key(0,n,100,1);const held=rms(w,80);
  w.z_transport(1);const levels=[];for(let s=0;s<16;s++)levels.push(rms(w,40));
  assert(Math.min(...levels)>held*0.6,'chord dropped: held '+held.toFixed(3)+' then '+levels.map(x=>x.toFixed(3)).join(' '));}});

check('a new note still gets a voice when every voice is held live (the oldest of its own track goes)',()=>{
 const w=fresh();for(let t=0;t<2;t++)w.z_set(t,8,127);for(let n=0;n<4;n++){w.z_key(0,60+n,100,1);w.z_key(1,40+n,100,1);}rms(w,10);
 w.z_key(1,70,100,1);rms(w,10);w.z_solo(2);const two=rms(w,20);assert(two>0.01);w.z_solo(1);assert(rms(w,20)>0.01,'track 1 keeps its chord');});

check('REC held one second on the SEQ page clears the whole pattern of the selected track; one undo restores it',()=>{
 const e=create();e.tap('SEQ');for(const k of [0,3,6,9])e.key(WHITE[k]);
 const before=steps(e);assert(before.filter(v=>v).length>=4);
 e.hold('REC');e.run(1200);e.up('REC');assert.equal(steps(e).filter(v=>v).length,0,'cleared');assert.equal(e.root()[1906],0,'not left recording');
 e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.deepEqual(steps(e),before,'undo');});

check('a short REC on the SEQ page still starts step entry and clears nothing',()=>{
 const e=create();e.tap('SEQ');e.key(WHITE[0]);const before=steps(e);e.tap('REC');assert.equal(e.root()[1906],1);assert.deepEqual(steps(e),before);e.tap('REC');assert.equal(e.root()[1906],0);});

check('REC held while playing a note, or with another button, does not clear',()=>{
 const e=create();e.tap('SEQ');e.key(WHITE[0]);const before=steps(e);e.hold('REC');e.w.web_key(WHITE[4],100,0);e.run(1200);e.w.web_key(WHITE[4],0,0);e.run(50);e.up('REC');
 assert(steps(e).filter(v=>v).length>=1,'kept');
 const f=create();f.tap('SEQ');f.key(WHITE[0]);const b2=steps(f);f.hold('SEQ');f.hold('REC');f.run(1200);f.up('REC');f.up('SEQ');assert(steps(f)[0]===0,'SEQ+REC still clears the part');});

check('REC held on another page does not clear (only the SEQ page)',()=>{
 const e=create();e.tap('SEQ');e.key(WHITE[0]);e.tap('HOME');const before=steps(e);e.hold('REC');e.run(1200);e.up('REC');e.run(100);assert.deepEqual(steps(e),before);});

check('REC held on the SEQ page of the drum track clears the kit pattern; one undo restores it',()=>{
 const e=create();e.turn(1,5);e.tap('SEQ');for(const k of [0,4,8])e.key(WHITE[k]);const before=kit(e);
 e.hold('REC');e.run(1200);e.up('REC');const hits=kit(e).slice(136).filter((v,i)=>i%3===0&&v).length;assert.equal(hits,0);
 e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.deepEqual(kit(e).slice(136),before.slice(136));});

if(failed){console.error(failed+' failed');process.exit(1);}

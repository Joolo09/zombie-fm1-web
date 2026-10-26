import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {create,WHITE} from './native_emulator_harness.mjs';
const module=new WebAssembly.Module(readFileSync('webapp/groovebox-130/zombie-core.wasm'));
const fresh=()=>{const w=new WebAssembly.Instance(module,{}).exports;w.z_init();return w;};
const root=w=>new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size());
let w=fresh();w.z_set(0,23,4);root(w)[1907]=20;w.z_record(1);w.z_key(0,60,100,1);w.z_key(0,60,0,0);assert.equal(root(w)[67],100);assert.equal(root(w)[67+20*4],0);assert.equal(root(w)[1907],1);
console.log('PASS step recording wraps an out-of-loop cursor into the audible loop');
w=fresh();root(w)[9]=5;w.z_record(1);w.z_key(5,36,100,1);assert.equal(w.z_dm_pattern(1),0);assert.equal(root(w)[1906],0);w.z_key(5,36,0,0);assert.equal(root(w)[1907],0);
console.log('PASS transforming a held drum take finishes recording and ignores the late release');
w=fresh();w.z_key(5,36,100,1);assert.equal(w.z_voice_count(),1);w.z_key(5,38,100,1);assert.equal(w.z_voice_count(),2);w.z_transport(0);assert.equal(w.z_voice_count(),0);
console.log('PASS the voice meter includes the sixteen drum voices');
let e=create();e.tap('REC');e.w.web_key(WHITE[0],100,0);e.run(60);e.turn(3,1);assert.equal(e.root()[1906],0);const cursor=e.root()[1907];e.w.web_key(WHITE[0],0,0);e.run(60);assert.equal(e.root()[1907],cursor);
console.log('PASS moving the cursor with K1 finishes held capture before moving it');
e=create();e.tap('HOME');e.turn(2,31);assert.equal(e.root()[1900],95);e.turn(2,1);assert.equal(e.root()[1900],0);e.turn(2,-1);assert.equal(e.root()[1900],95);e.tap('SAVE');e.w.web_reset();e.run(120);assert.equal(e.root()[1900],95);
console.log('PASS native preset knob reaches and wraps all 96 slots; new preset identity survives SAVE/reboot');

for(let id=0;id<96;id++){const x=fresh();x.z_preset(0,id);for(const note of [24,48,72,96,120]){x.z_key(0,note,127,1);x.z_render(x.z_buffer(),256);for(const v of new Float32Array(x.memory.buffer,x.z_buffer(),512))assert(Number.isFinite(v)&&Math.abs(v)<=.981);x.z_key(0,note,0,0);}x.z_transport(0);assert.equal(x.z_voice_count(),0);}
console.log('PASS all 96 presets remain finite across five octaves and panic clears every voice');

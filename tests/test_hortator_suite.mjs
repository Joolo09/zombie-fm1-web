/* SPDX-License-Identifier: GPL-3.0-only — real C/WASM DSP and state checks. */
import assert from 'node:assert/strict';
import {readFile,writeFile} from 'node:fs/promises';
const bytes=await readFile(new URL('../source/output/editions-0.9.1/groovebox.wasm',import.meta.url));
const module=await WebAssembly.compile(bytes);
const make=()=>new WebAssembly.Instance(module,{}).exports;
const w=make();w.z_init();
assert.equal(w.h_models(),45);assert.equal(w.h_presets(),151);
const audio=()=>new Float32Array(w.memory.buffer,w.z_buffer(),512);
function render(frames){let peak=0,energy=0;for(let at=0;at<frames;at+=256){w.z_render(w.z_buffer(),256);for(const v of audio()){assert(Number.isFinite(v));peak=Math.max(peak,Math.abs(v));energy+=v*v;}}return {peak,energy};}
for(let model=0;model<45;model++){
 w.z_init();assert.equal(w.h_set(0,0,model),0);w.h_hit(0,100);
 const a=render(8192);assert(a.peak>1e-5,`silent model ${model}`);assert(a.peak<2,`unbounded model ${model}: ${a.peak}`);
}
console.log('PASS all 45 actual drum models render finite, audible, bounded audio');
for(let preset=0;preset<151;preset++){
 w.z_init();assert.equal(w.h_load_preset(0,preset),0);w.h_hit(0,100);
 const a=render(4096);assert(a.peak>1e-6,`silent preset ${preset}`);assert(a.peak<2,`unbounded preset ${preset}`);
}
console.log('PASS all 151 factory presets load and produce audio');

// Exercise the actual send/insert/modulation paths, not just parameter storage.
function sound(edits=[],globals=[]){const f=make();f.z_init();f.h_set(0,0,2);for(const [p,v] of edits)assert.equal(f.h_set(0,p,v),0);for(const [p,v] of globals)assert.equal(f.h_set_global(p,v),0);f.h_hit(0,100);const out=[];for(let i=0;i<128;i++){f.z_render(f.z_buffer(),256);for(const v of new Float32Array(f.memory.buffer,f.z_buffer(),512)){assert(Number.isFinite(v));assert(Math.abs(v)<2);out.push(v);}}return out;}
const dry=sound();
for(const [name,params,globals] of [
 ['distortion',[[14,100]]],['chorus',[[15,127]]],['delay',[[16,127]],[[3,0],[6,127]]],
 ['room reverb',[[17,127]]],['spring reverb',[[17,127]],[[40,1]]],
 ['filter',[[55,1],[56,40]]],['sample layer',[[27,127]]],
 ['LFO routing',[[33,1],[34,110],[36,64],[37,9]]],
 ...[1,2,3,4].map(m=>['resonator '+m,[[48,m],[51,127]]]),
 ['ghost compressor source',[],[[32,1],[39,2]]]
])assert.notDeepEqual(sound(params,globals),dry,`inactive ${name}`);
console.log('PASS distinct audible paths for inserts, sends, both reverbs, sample layer, LFO, all resonators and ghost COMP');

w.z_init();const firstFive=new Uint8Array(w.memory.buffer,w.z_snapshot()+12,5*310).slice();
assert.equal(w.h_set(0,1,7),0);assert.equal(w.h_set_step(0,63,1,1,56,3,12),0);
assert.equal(w.h_set_step(8,0,1,0,0,0,0),-1);assert.equal(w.h_set_step(0,64,1,0,0,0,0),-1);
assert.equal(w.h_set_step(0,0,1,0,57,0,0),-1);assert.equal(w.h_remote(2,7,1),-1);
assert.deepEqual(new Uint8Array(w.memory.buffer,w.z_snapshot()+12,5*310),firstFive);
const ptr=w.z_transfer(),n=w.h_pack(ptr);assert.equal(n,3288);
const packed=new Uint8Array(w.memory.buffer,ptr,n).slice();w.h_set(0,1,-7);
new Uint8Array(w.memory.buffer,ptr,n).set(packed);assert.equal(w.h_unpack(ptr,n),0);assert.equal(w.h_get(0,1),7);
new Uint8Array(w.memory.buffer,ptr,n)[100]^=1;assert.equal(w.h_unpack(ptr,n),-1);assert.equal(w.h_get(0,1),7);
assert.equal(w.h_save(),0);assert.equal(w.h_flash_dirty(),1);w.z_init();assert.equal(w.h_get(0,1),7);
assert.deepEqual(new Uint8Array(w.memory.buffer,w.z_snapshot()+12,5*310),firstFive);
console.log('PASS independent kit state, rejected malformed imports, save and restart; five synth tracks unchanged');

const old=new WebAssembly.Instance(await WebAssembly.compile(await readFile(new URL('../webapp/groovebox/zombie-core.wasm',import.meta.url))),{}).exports;
for(let track=0;track<5;track++)for(const preset of [0,8,16,24,32,40,48,56]){
 const fresh=make();old.z_init();fresh.z_init();
 old.z_preset(track,preset);fresh.z_preset(track,preset);old.z_key(track,60,100,1);fresh.z_key(track,60,100,1);
 for(let block=0;block<12;block++){
  old.z_render(old.z_buffer(),256);fresh.z_render(fresh.z_buffer(),256);
  assert.deepEqual(new Float32Array(fresh.memory.buffer,fresh.z_buffer(),512),new Float32Array(old.memory.buffer,old.z_buffer(),512),`synth changed: track ${track}, preset ${preset}`);
 }
}
console.log('PASS exact five-track synth audio against the released WASM (40 cases)');
w.z_init();w.h_ui_tick();await writeFile(new URL('../source/output/editions-0.9.1/hortator-screen.rgb565',import.meta.url),new Uint8Array(w.memory.buffer,w.h_frame(),240*240*2));
console.log('PASS native Hortator UI framebuffer generated');

let seed=0x51f15e37;const rnd=n=>{seed=(Math.imul(seed,1664525)+1013904223)>>>0;return seed%n;};
for(let test=0;test<120;test++){
 const f=make();f.z_init();
 for(let k=0;k<8;k++){f.h_set(k,0,rnd(45));for(let j=0;j<32;j++)f.h_set(k,rnd(70),rnd(256)-128);for(let j=0;j<8;j++)assert.equal(f.h_set_step(k,rnd(64),1,rnd(2),rnd(57),rnd(4),rnd(128)-60),0);}
 const ptr=f.z_transfer(),n=f.h_pack(ptr);assert.equal(f.h_unpack(ptr,n),0,`self-export rejected in case ${test}`);
 f.z_transport(1);for(let k=0;k<8;k++)f.h_hit(k,127);
 for(let b=0;b<32;b++){f.z_render(f.z_buffer(),256);for(const v of new Float32Array(f.memory.buffer,f.z_buffer(),512))assert(Number.isFinite(v));f.z_post(127,1,f.z_buffer(),256);for(const v of new Float32Array(f.memory.buffer,f.z_buffer(),512))assert(Math.abs(v)<=1);}
}
console.log('PASS 120 deterministic eight-lane parameter/pattern/import/audio stress cases without WASM bounds traps');

import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {decodeDrums,encodeDrums,wasmDrums,DRUM_BYTES} from '../webapp/groovebox-100/drum-state.js';
const create=path=>new WebAssembly.Instance(new WebAssembly.Module(readFileSync(new URL(path,import.meta.url))),{}).exports;
const fresh=()=>{const w=create('../webapp/'+(process.argv[2]||'groovebox-100')+'/zombie-core.wasm');w.z_init();w.z_dm_init();return w;};
const render=(w,n)=>{const out=[];for(let at=0;at<n;at+=256){const k=Math.min(256,n-at);w.z_render(w.z_buffer(),k);out.push(...new Float32Array(w.memory.buffer,w.z_buffer(),k*2));}return out;};
const energy=a=>a.reduce((s,x)=>{assert(Number.isFinite(x)&&Math.abs(x)<=.981);return s+x*x;},0);
for(let t=0;t<5;t++)for(let model=0;model<8;model++){
 const a=fresh(),b=create('../webapp/groovebox/zombie-core.wasm');b.z_init();for(const w of [a,b]){w.z_preset(t,0);w.z_set(t,0,model);w.z_key(t,60,100,1);}assert.deepEqual(render(a,4096),render(b,4096),`synth ${t+1} model ${model}`);
}
console.log('PASS: all five synth tracks, all eight engines, bit-identical to previous Zombie');
let w=fresh();for(let d=0;d<16;d++)assert.equal(w.z_dm_step(d,0,100,127,1),0);assert.equal(wasmDrums(w).sounds.filter(p=>p.steps[0].velocity).length,16);for(let t=0;t<5;t++)w.z_set(t,21,1);w.z_transport(1);assert(energy(render(w,6000))>1);console.log('PASS: sixteen independent parts, simultaneous sequencing, bounded audio');
w=fresh();w.z_dm_step(0,0,100,0,1);for(let t=0;t<5;t++)w.z_set(t,21,1);w.z_transport(1);assert.equal(energy(render(w,5000)),0);console.log('PASS: zero chance produces silence');
function roll(n){const x=fresh();for(let t=0;t<5;t++)x.z_set(t,21,1);x.z_dm_step(0,0,100,127,n);x.z_transport(1);return render(x,5000);}
assert.notDeepEqual(roll(1),roll(4));console.log('PASS: rolls retrigger within a step');
w=fresh();w.z_dm_param(0,5,3);w.z_dm_param(1,5,5);w.z_external(1);w.z_clock(0xfa);for(let i=0;i<7*6;i++)w.z_clock(0xf8);assert.equal(w.z_dm_position(0),0);assert.equal(w.z_dm_position(1),1);w.z_clock(0xfc);w.z_clock(0xfb);for(let i=0;i<6;i++)w.z_clock(0xf8);assert.equal(w.z_dm_position(0),1);assert.equal(w.z_dm_position(1),2);console.log('PASS: independent part lengths and MIDI stop/continue');
w=fresh();const root=()=>new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size());root()[9]=5;w.z_record(1);for(const n of [36,38,39,37,45,47])w.z_key(5,n,100,1);for(const n of [36,38,39,37,45])w.z_key(5,n,0,0);assert.equal(root()[1907],0);w.z_key(5,47,0,0);assert.equal(root()[1907],1);w.z_key(5,47,0,0);assert.equal(root()[1907],1);assert.equal(wasmDrums(w).sounds.filter(p=>p.steps[0].velocity).length,6);console.log('PASS: more than four drum parts record at once, release advances exactly once');
w=fresh();const original=wasmDrums(w),bytes=encodeDrums(original);assert.equal(bytes.length,DRUM_BYTES);assert.deepEqual(decodeDrums(bytes),original);bytes[13]=0;new Uint8Array(w.memory.buffer,w.z_transfer(),bytes.length).set(bytes);assert.equal(w.z_dm_apply(bytes.length),-1);assert.deepEqual(wasmDrums(w),original);assert.equal(w.z_dm_param(16,0,100),-1);assert.equal(w.z_dm_step(0,64,1,127,1),-1);console.log('PASS: serialization and rejected imports leave existing project intact');
w=fresh();new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size())[9]=5;w.z_dm_step(0,7,101,50,4);assert.equal(w.z_history(0),1);assert.equal(wasmDrums(w).sounds[0].steps[7].velocity,0);assert.equal(w.z_history(1),1);assert.equal(wasmDrums(w).sounds[0].steps[7].ratchet,4);console.log('PASS: drum undo and redo');
assert(!Object.keys(w).some(k=>k.startsWith('h_')));console.log('PASS: no Hortator runtime exports');

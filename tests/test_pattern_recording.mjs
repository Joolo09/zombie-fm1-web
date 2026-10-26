import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
const editor=process.argv[2]||'groovebox-101';const {decodeState}=await import('../webapp/'+editor+'/core.js');
const {wasmDrums}=await import('../webapp/'+editor+'/drum-state.js');
const module=new WebAssembly.Module(readFileSync(new URL('../webapp/'+editor+'/zombie-core.wasm',import.meta.url)));
const fresh=()=>{const w=new WebAssembly.Instance(module,{}).exports;w.z_init();w.z_dm_init();for(let t=0;t<5;t++)w.z_erase(t,0,2);w.z_tempo(120,0);return w;};
const frames=(w,n)=>{for(let i=0;i<n;i+=256)w.z_render(w.z_buffer(),Math.min(256,n-i));};
const state=w=>decodeState(new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size()).slice());
let w=fresh();w.z_record(1);w.z_key(0,60,100,1);frames(w,100);w.z_key(0,60,0,0);assert.equal(state(w).tracks[0].steps[0].gate,80);assert.equal(state(w).stepEdit,1);w.z_key(0,60,0,0);assert.equal(state(w).stepEdit,1);console.log('PASS step recording uses musical gates; release advances once');
w=fresh();w.z_external(1);w.z_clock(0xfa);w.z_clock(0xf8);w.z_record(1);w.z_key(0,60,100,1);frames(w,12000);w.z_key(0,60,0,0);assert(state(w).tracks[0].steps[0].duration>=2);w.z_record(0);w.z_clock(0xfa);w.z_clock(0xf8);w.z_record(1);w.z_key(0,60,100,1);frames(w,500);w.z_key(0,60,0,0);assert.equal(state(w).tracks[0].steps[0].duration,0);assert(state(w).tracks[0].steps[0].gate<20);console.log('PASS a new short take replaces a previously long gate');
w=fresh();w.z_external(1);w.z_clock(0xfa);w.z_set(0,23,3);w.z_step_set(0,0,60,100,127,80);for(let i=0;i<65*6;i++)w.z_clock(0xf8);assert.equal(w.z_position(),64);assert.equal(w.z_position()%3,1);console.log('PASS non power-of-two loops keep their phase after 64 steps');
w=fresh();w.z_dm_step(0,0,100,127,1);w.z_dm_step(1,0,110,127,1);new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size())[3444]=1;w.z_record(1);w.z_key(5,36,90,1);w.z_key(5,36,0,0);assert.equal(wasmDrums(w).sounds[1].steps[0].velocity,110);console.log('PASS drum replacement never clears a different instrument');
w=fresh();w.z_record(1);w.z_key(0,60,100,1);frames(w,700);w.z_erase(0,0,0);w.z_key(0,60,0,0);assert.equal(state(w).tracks[0].steps[0].velocity,0);assert.equal(state(w).tracks[0].steps[0].duration,0);assert.equal(state(w).recording,0);assert.equal(w.z_history(0),1);assert.equal(state(w).tracks[0].steps[0].velocity,100);console.log('PASS erasing finishes capture and a late key-up cannot restore a deleted note; undo restores it');
w=fresh();w.z_dm_step(0,0,100,50,4);w.z_dm_step(1,0,110,60,2);w.z_dm_select(0);w.z_erase(5,0,1);assert.equal(wasmDrums(w).sounds[0].steps[0].velocity,0);assert.equal(wasmDrums(w).sounds[1].steps[0].velocity,110);w.z_erase(5,0,2);assert.equal(wasmDrums(w).sounds[1].steps[0].velocity,0);new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size())[9]=5;w.z_history(0);assert.equal(wasmDrums(w).sounds[1].steps[0].velocity,110);console.log('PASS separate part/track erasure and drum undo');

for(const [track,note] of [[0,60],[5,36]]){
 const a=fresh(),b=fresh();for(const x of [a,b]){x.z_transport(1);frames(x,4000);}a.z_record(1);
 for(const x of [a,b]){x.z_key(track,note,100,1);frames(x,200);x.z_key(track,note,0,0);}
 const capture=x=>{const out=[];for(let i=0;i<2200;i+=256){const n=Math.min(256,2200-i);x.z_render(x.z_buffer(),n);out.push(...new Float32Array(x.memory.buffer,x.z_buffer(),n*2));}return out;};
 assert.deepEqual(capture(a),capture(b),`no replay of quantized-ahead track ${track}`);
}
console.log('PASS live recording never replays a quantized-ahead drum or melody hit after key-up');

w=fresh();new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size())[9]=5;w.z_record(1);w.z_key(5,36,100,1);w.z_key(5,38,100,1);w.z_key(5,36,0,0);w.z_key(5,38,0,0);w.z_record(0);w.z_history(0);assert.equal(wasmDrums(w).sounds[0].steps[0].velocity,0);assert.equal(wasmDrums(w).sounds[1].steps[0].velocity,0);console.log('PASS one drum recording take is one undo operation, with no per-hit history copies');

w=fresh();w.z_record(1);w.z_key(0,60,100,1);w.z_key(0,60,0,0);w.z_key(5,36,100,1);w.z_key(5,36,0,0);w.z_record(0);new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size())[9]=5;assert.equal(w.z_history(0),1);assert.equal(wasmDrums(w).sounds[0].steps[1].velocity,0);new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size())[9]=0;assert.equal(w.z_history(0),1);assert.equal(state(w).tracks[0].steps[0].velocity,0);console.log('PASS changing between melody and drum tracks within a take retains undo for both');

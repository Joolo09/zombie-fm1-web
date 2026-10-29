import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
const mod=new WebAssembly.Module(readFileSync('webapp/groovebox-140/zombie-core.wasm'));
const fresh=()=>{const w=new WebAssembly.Instance(mod,{}).exports;w.z_init();return w;};
let blocks=0;function render(w,n){w.z_render(w.z_buffer(),n);for(const x of new Float32Array(w.memory.buffer,w.z_buffer(),n*2))assert(Number.isFinite(x)&&Math.abs(x)<=.981);w.z_post(127,1,w.z_buffer(),n);for(const x of new Float32Array(w.memory.buffer,w.z_buffer(),n*2))assert(Number.isFinite(x)&&Math.abs(x)<=1);blocks++;}
for(let model=0;model<8;model++)for(const cutoff of [0,127])for(const resonance of [0,127])for(const bend of [0,16383]){const w=fresh();w.z_set(0,0,model);for(const [id,value] of [[4,cutoff],[5,resonance],[10,127],[11,16],[12,127],[13,127],[14,127],[15,127],[17,127],[18,127],[25,24]])w.z_set(0,id,value);w.z_bend(0,bend);for(const note of [0,1,36,48,60,84,126,127])w.z_key(0,note,127,1);for(const n of [1,17,128,256,256,256])render(w,n);w.z_transport(0);assert.equal(w.z_voice_count(),0);}
console.log('PASS 64 synth parameter corners: all 8 engines, filter/resonance/bend extremes, 8 voices, FX and block sizes');
for(const decay of [1,127])for(const tune of [-24,24])for(const pan of [-64,63]){const w=fresh();w.z_tempo(300,60);for(let d=0;d<16;d++){for(const [id,value] of [[0,127],[1,decay],[2,tune],[3,pan],[5,1]])assert.equal(w.z_dm_param(d,id,value),0);assert.equal(w.z_dm_step(d,0,127,127,4),0);}w.z_transport(1);for(let i=0;i<176;i++)render(w,256);w.z_transport(0);assert.equal(w.z_voice_count(),0);}
console.log('PASS all 16 drums together at 300 BPM, maximum swing and rolls across decay/tune/pan corners');
for(let effect=1;effect<=16;effect++){const w=fresh();w.z_set(0,6,0);w.z_set(0,8,127);w.z_key(0,60,127,1);w.z_live(effect);for(let i=0;i<20;i++)render(w,256);w.z_live(0);for(let i=0;i<10;i++)render(w,256);w.z_transport(0);assert.equal(w.z_live_state(),0);assert.equal(w.z_voice_count(),0);}
console.log('PASS all 16 live effects and releases keep stereo output finite and bounded');
const w=fresh();w.z_set(0,6,0);w.z_set(0,8,127);w.z_set(0,9,0);for(let note=0;note<128;note++)w.z_key(0,note,127,1);assert.equal(w.z_voice_count(),8);for(let note=0;note<128;note++)w.z_key(0,note,0,0);for(let i=0;i<20;i++)render(w,256);assert.equal(w.z_voice_count(),0);
console.log('PASS all 128 held MIDI pitches release after voice stealing; '+blocks+' actual DSP blocks checked');
import {CoreDSP} from '../webapp/groovebox-140/worklet.js';
const dsps=[new CoreDSP(mod),new CoreDSP(mod)];
for(const dsp of dsps){const w=dsp.w;w.z_tempo(120,0);w.z_dm_step(0,0,100,127,4);const kit=new Uint8Array(w.memory.buffer,w.z_dm_data(),3208).slice();dsp.drums(kit);w.z_transport(1);w.z_render(w.z_buffer(),256);w.z_render(w.z_buffer(),185);}
const [preview,reference]=dsps,kit=new Uint8Array(preview.w.memory.buffer,preview.w.z_dm_data(),3208).slice();kit[136]=0;preview.drums(kit);reference.w.z_dm_step(0,0,0,127,4);
for(let i=0;i<16;i++){for(const dsp of dsps)dsp.w.z_render(dsp.w.z_buffer(),256);assert.deepEqual(new Float32Array(preview.w.memory.buffer,preview.w.z_buffer(),512),new Float32Array(reference.w.memory.buffer,reference.w.z_buffer(),512));}
console.log('PASS offline drum-grid deletion cancels pending rolls through the same C event path');

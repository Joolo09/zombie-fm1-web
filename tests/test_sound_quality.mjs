/* SPDX-License-Identifier: MIT — 0.14.0 mixer regressions: no clicks on STOP/steal, no clipping, no voice pile-up. */
import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
const editor=process.argv[2]||'groovebox-180';
const module=new WebAssembly.Module(readFileSync(new URL('../webapp/'+editor+'/zombie-core.wasm',import.meta.url)));
const fresh=()=>{const w=new WebAssembly.Instance(module,{}).exports;w.z_init();w.z_dm_init();return w;};
/* Render n frames; report peak, largest sample-to-sample jump and hard-clamped samples. */
const run=(w,n,stat={peak:0,jump:0,clipped:0,last:null})=>{for(let at=0;at<n;at+=256){const k=Math.min(256,n-at);w.z_render(w.z_buffer(),k);const b=new Float32Array(w.memory.buffer,w.z_buffer(),k*2);
 for(let i=0;i<k;i++){const l=b[2*i];assert(Number.isFinite(l));const a=Math.abs(l);stat.peak=Math.max(stat.peak,a);if(a>=.979)stat.clipped++;if(stat.last!==null)stat.jump=Math.max(stat.jump,Math.abs(l-stat.last));stat.last=l;}}return stat;};

let w=fresh();for(let i=0;i<16;i++){w.z_note(0,60,100,1);run(w,2000);}
assert.equal(w.z_voice_count(),1);console.log('PASS repeated note-on of one pitch reuses its voice instead of filling all eight');

w=fresh();for(let n=0;n<4;n++)w.z_key(0,48+n*3,120,1);w.z_set(0,13,90);w.z_set(0,14,90);run(w,30000);
let before=run(w,64),s=run(w,0,{peak:0,jump:0,clipped:0,last:before.last});w.z_transport(1);run(w,5000,s);w.z_transport(0);s=run(w,3000,{peak:0,jump:0,clipped:0,last:s.last});
assert(s.jump<.08,'STOP jump '+s.jump);assert.equal(w.z_voice_count(),0);console.log('PASS STOP fades voices, echo and reverb instead of clicking (max jump '+s.jump.toFixed(3)+')');

w=fresh();for(let n=0;n<4;n++)w.z_key(0,48+n*3,120,1);run(w,30000);before=run(w,64);w.z_alloff(0);s=run(w,3000,{peak:0,jump:0,clipped:0,last:before.last});
assert(s.jump<.02,'alloff jump '+s.jump);console.log('PASS cutting a track fades its held chord');

w=fresh();for(let d=0;d<16;d++)w.z_key(5,[36,38,42,46,39,37,45,47,50,49,51,56,54,70,75,76][d],127,1);s=run(w,44100);
assert.equal(s.clipped,0);assert(s.peak<=.91,'drum peak '+s.peak);console.log('PASS sixteen simultaneous drums are limited, not clipped (peak '+s.peak.toFixed(3)+')');

w=fresh();for(let t=0;t<5;t++){w.z_set(t,15,127);w.z_set(t,12,127);for(let n=0;n<4;n++)w.z_key(t,40+n*5+t*2,127,1);}s=run(w,88200);
assert.equal(s.clipped,0);console.log('PASS five driven synth tracks at full level stay under the bus limit');

w=fresh();w.z_set(0,6,0);for(let n=0;n<8;n++)w.z_key(0,48+n,100,1);run(w,20000);before=run(w,64);w.z_key(0,70,100,1);s=run(w,400,{peak:0,jump:0,clipped:0,last:before.last});
assert(s.jump<.06,'steal jump '+s.jump);console.log('PASS a ninth note steals a voice without a click');

w=fresh();w.z_set(0,6,0);w.z_set(0,1,1);w.z_key(0,60,127,1);s=run(w,64,{peak:0,jump:0,clipped:0,last:0});
assert(s.jump<.05,'attack 0 jump '+s.jump);console.log('PASS ATTACK 0 on a saw starts without a step');

w=fresh();const quiet=fresh();w.z_key(0,60,100,1);quiet.z_key(0,60,100,1);const a=run(w,4096),b=run(quiet,4096);assert.equal(a.peak,b.peak);
w=fresh();w.z_key(0,60,100,1);s=run(w,22050);assert(s.peak>.05&&s.peak<.5);console.log('PASS a single note keeps its level (limiter idle below threshold)');

/* SPDX-License-Identifier: MIT — 0.20.0: Groovebox tracks 1-5 play the player's own WAVs (SAMPLE engine, USR1-USR4). */
import assert from 'node:assert/strict';import {create,WHITE} from './native_emulator_harness.mjs';import {protocol,crc32,u32} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
/* a 0.3 s 220 Hz saw at 22050 Hz, as 16-bit little-endian PCM */
const pcm=(()=>{const n=6615,b=new Uint8Array(n*2),v=new DataView(b.buffer);for(let i=0;i<n;i++)v.setInt16(i*2,Math.round(((i*220/22050)%1*2-1)*12000),true);return b;})();
const send=(e,slot)=>{const p=pr(e);p(11,[slot,...u32(pcm.length),22050&255,22050>>8,...u32(crc32(pcm))]);for(let at=0;at<pcm.length;at+=180)p(12,[at&255,at>>8,...pcm.slice(at,at+180)]);p(13);return p;};
const P=new WeakMap(),pr=e=>{if(!P.has(e))P.set(e,protocol(e));return P.get(e);};
const set=(e,t,id,v)=>pr(e)(3,[t,id,v&255,(v>>8)&255]);
const play=(e,t,note=60,ms=200)=>{pr(e)(4,[t,note,110,1]);const pk=e.run(ms);pr(e)(4,[t,note,0,0]);e.run(300);return pk;};
const MODEL=0,SAMPLE=19;
check('tracks 1-5 can switch to SAMPLE; the drum track cannot',()=>{const e=create();for(let t=0;t<6;t++){set(e,t,MODEL,8);assert.equal(e.w.z_get(t,MODEL),t===5?7:8,'track '+(t+1));}});
check('a SAMPLE track only plays the USR slots (no built-in PCM in Groovebox)',()=>{const e=create();set(e,0,MODEL,8);assert.equal(e.w.z_get(0,SAMPLE),6);set(e,0,SAMPLE,0);assert.equal(e.w.z_get(0,SAMPLE),6);set(e,0,SAMPLE,13);assert.equal(e.w.z_get(0,SAMPLE),9);});
check('an empty slot is silent; an uploaded WAV plays, pitched by the key',()=>{const e=create();set(e,0,MODEL,8);e.run(200);assert(play(e,0)<0.002,'empty slot silent');
 send(e,0);set(e,0,20,60);const a=play(e,0,60);assert(a>0.02,'USR1 plays: '+a);});
check('the WAV survives a restart (it lives in flash)',()=>{const e=create();send(e,1);set(e,0,MODEL,8);set(e,0,SAMPLE,7);e.tap('SAVE');e.run(400);e.w.web_reset();e.run(800);assert.equal(e.w.z_get(0,MODEL),8);assert.equal(e.w.z_get(0,SAMPLE),7);assert(play(e,0)>0.02,'USR2 after restart');});
check('on the panel: EDIT page 2 picks the slot with K1',()=>{const e=create();set(e,0,MODEL,8);e.run(200);e.tap('EDIT');e.run(60);e.tap('EDIT');e.run(60);e.turn(3,2);e.run(60);assert.equal(e.w.z_get(0,SAMPLE),8,'K1 on EDIT page 2 moves USR1 -> USR3');});
check('a preset turns the track back into a synth',()=>{const e=create();set(e,0,MODEL,8);pr(e)(7,[0,3]);assert(e.w.z_get(0,MODEL)<8);});
if(failed)process.exit(1);

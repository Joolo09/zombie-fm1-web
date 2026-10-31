/* SPDX-License-Identifier: MIT — 0.17.0 MIDI clock out: 24 per quarter note, start and stop, off by default. */
import assert from 'node:assert/strict';import {create} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const pkts=e=>Array.from(new Uint32Array(e.memory.buffer,e.w.web_out_pkts(),e.w.web_out_n()));
const status=p=>p>>8&255;const count=(e,s)=>pkts(e).filter(p=>(p&15)===15&&status(p)===s).length;
const on=(e)=>{protocol(e)(29,[12,1]);};
const play=(e,ms)=>{e.w.web_out_clear();e.tap('PLAY');e.run(ms);};

check('clock out is off by default: nothing is sent',()=>{const e=create();play(e,2000);assert.equal(count(e,0xF8)+count(e,0xFA)+count(e,0xFC),0);e.tap('PLAY');});

check('at 120 BPM the clock runs at 48 pulses a second, preceded by Start',()=>{const e=create();on(e);e.w.z_tempo(120,0);play(e,4000);
 const all=pkts(e).filter(p=>(p&15)===15),first=all.find(p=>[0xF8,0xFA,0xFC].includes(status(p)));assert.equal(status(first),0xFA,'Start comes first');
 const clocks=count(e,0xF8);assert(clocks>=186&&clocks<=198,'pulses in 4 s: '+clocks);e.tap('PLAY');});

check('Stop is sent when the transport stops, and the clock stops with it',()=>{const e=create();on(e);play(e,1000);e.tap('PLAY');e.run(50);const n=count(e,0xF8);
 assert.equal(count(e,0xFC),1);e.w.web_out_clear();e.run(1000);assert.equal(count(e,0xF8),0);assert.equal(n>0,true);});

check('the clock follows the tempo: 240 BPM doubles the rate',()=>{const e=create();on(e);e.w.z_tempo(240,0);play(e,2000);const c=count(e,0xF8);assert(c>=186&&c<=204,'pulses in 2 s at 240: '+c);e.tap('PLAY');});

check('pulses are evenly spaced: never a burst, 20.8 ms apart at 120 BPM (the render block is 256 frames, so gaps read 3 or 4 blocks)',()=>{const e=create();on(e);e.w.z_tempo(120,0);e.w.web_out_clear();e.tap('PLAY');
 let prev=count(e,0xF8),last=-1,gaps=[];for(let i=0;i<500;i++){e.run(1);const now=count(e,0xF8);assert(now-prev<=1,'burst of '+(now-prev)+' pulses');if(now>prev){if(last>=0)gaps.push(i-last);last=i;}prev=now;}   // one run(1) renders 128 frames = 2.9 ms
 assert(gaps.length>40);for(const g of gaps)assert(g>=5&&g<=9,'gap '+g+' loops');const mean=gaps.reduce((a,b)=>a+b)/gaps.length;assert(mean>7&&mean<7.4,'mean gap '+mean+' loops, expected 7.18');e.tap('PLAY');});

check('swing keeps the clock at 24 pulses a beat on average',()=>{const e=create();on(e);e.w.z_tempo(120,0);
 const rate=sw=>{const x=create();on(x);x.w.z_tempo(120,sw);x.w.web_out_clear();x.tap('PLAY');x.run(4000);const c=count(x,0xF8);x.tap('PLAY');return c;};
 const a=rate(0),b=rate(40);assert(Math.abs(a-b)<=8,'pulse totals '+a+' vs '+b);});

check('an external clock (USB) is never echoed back',()=>{const e=create();on(e);protocol(e)(21,[8,1]);e.run(50);e.w.web_out_clear();
 e.w.web_midi_in(0,0x0f|(0xFA<<8));for(let i=0;i<200;i++){e.w.web_midi_in(0,0x0f|(0xF8<<8));e.run(5);}assert.equal(count(e,0xF8)+count(e,0xFA),0);});

check('DAW modes send no clock',()=>{const e=create();on(e);e.hold('HOME');e.tap('SEL');e.up('HOME');   // DAW synth
 e.w.web_out_clear();e.tap('PLAY');e.run(1000);assert.equal(count(e,0xF8),0);});

check('the setting is saved and the SETTINGS page shows it',()=>{const e=create();on(e);e.tap('SAVE');e.run(300);e.w.web_reset();e.run(200);play(e,1000);assert(count(e,0xF8)>30);e.tap('PLAY');});

if(failed)process.exit(1);

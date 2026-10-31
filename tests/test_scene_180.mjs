/* SPDX-License-Identifier: MIT — 0.19.0 night screensaver, the live scene and the ZMB preset bank. */
import assert from 'node:assert/strict';import {readFileSync} from 'node:fs';import {create,WHITE} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
/* the saver's moon: its face colour, in either byte order, inside the box around (186, 52) */
const moon=e=>{const fb=new Uint16Array(e.memory.buffer,e.w.web_fb(),240*240);let n=0;for(let y=34;y<70;y++)for(let x=168;x<204;x++){const v=fb[y*240+x];if(v===0xE757||v===0x57E7)n++;}return n;};
const idle=(e,ms)=>{for(let t=0;t<ms;t+=5000)e.run(Math.min(5000,ms-t));};

check('the night scene appears after a minute untouched, not before',()=>{const e=create();idle(e,45000);assert.equal(moon(e),0,'at 45 s');idle(e,17000);assert(moon(e)>300,'moon at 62 s: '+moon(e));});
check('any touch wakes it, and the touch still works (a key plays its note)',()=>{const e=create();idle(e,62000);assert(moon(e)>300);e.w.web_key(WHITE[4],100,0);const peak=e.run(200);e.w.web_key(WHITE[4],0,0);e.run(100);assert.equal(moon(e),0);assert(peak>0.005,'note sounded: '+peak);});
check('a knob turn wakes it too, and turns the knob',()=>{const e=create();idle(e,62000);const before=e.w.z_get(0,4);e.turn(3,2);e.run(100);assert.equal(moon(e),0);assert.notEqual(e.w.z_get(0,4),before);});
check('the screensaver setting turns it off and is read back by the editor',()=>{const e=create(),p=protocol(e);p(29,[13,0]);idle(e,62000);assert.equal(moon(e),0);
 const b=p(28);assert.equal(b[110]&16,16);p(29,[13,1]);idle(e,62000);assert(moon(e)>300);});
check('never while recording: a minute of step recording keeps the editor on screen',()=>{const e=create();e.rec();idle(e,62000);assert.equal(moon(e),0);e.tap('REC');});
check('while the song plays it shows the scene and the music keeps going',()=>{const e=create();e.rec();e.key(WHITE[0]);e.key(WHITE[4]);e.key(WHITE[7]);e.tap('REC');e.tap('HOME');e.tap('PLAY');let peak=0;for(let t=0;t<62000;t+=2000)peak=Math.max(peak,e.run(2000));
 assert(moon(e)>300);assert.equal(e.w.z_playing(),1);assert(e.run(2000)>0.005,'still audible');e.tap('PLAY');});
check('asleep and stopped, the white keys glow in a slow wave',()=>{const e=create();idle(e,62000);const a=e.w.web_dim_keys();e.run(700);const b=e.w.web_dim_keys();assert(a&&b&&a!==b,'wave '+a+' '+b);e.tap('HOME');e.run(100);assert.equal(moon(e),0);});

const mod=new WebAssembly.Module(readFileSync(new URL('../webapp/groovebox-180/zombie-core.wasm',import.meta.url)));
const names=JSON.parse(readFileSync(new URL('../source/editions/zombie-presets.json',import.meta.url)));
check('the ZMB bank fills slots 97-128: every patch audible, finite, unclipped and distinct',()=>{const sums=new Set();
 for(let i=0;i<32;i++){const w=new WebAssembly.Instance(mod,{}).exports;w.z_init();w.z_preset(0,96+i);assert.equal(w.z_preset_index?.(0)??96+i,96+i);w.z_set(0,15,88);
  w.z_key(0,i<8?36:60,100,1);let e=0,pk=0,n=0,sum=0;for(let f=0;f<44100*3;f+=256){if(f>=44100*(i>=24?2.5:0.6)&&f<44100*(i>=24?2.5:0.6)+256)w.z_key(0,i<8?36:60,0,0);w.z_render(w.z_buffer(),256);for(const x of new Float32Array(w.memory.buffer,w.z_buffer(),512)){assert(Number.isFinite(x));e+=x*x;pk=Math.max(pk,Math.abs(x));n++;sum+=x*(n%97);}}
  const rms=Math.sqrt(e/n);assert(rms>0.004,names[i][0]+' rms '+rms);assert(pk<0.9,names[i][0]+' peak '+pk);sums.add(sum.toFixed(4));}
 assert.equal(sums.size,32);});

/* 0.19.0 */
const about=e=>{const fb=new Uint16Array(e.memory.buffer,e.w.web_fb(),240*240);let n=0;for(let x=6;x<76;x++){const v=fb[62*240+x];if(v===0xFCCB||v===0xCBFC)n++;}return n>50;};
const style=e=>protocol(e)(28)[110]>>5&3;
check('a lone encoder tick (a resting knob on the device) does not keep the screensaver away',()=>{const e=create();idle(e,30000);e.turn(3,1);idle(e,1500);e.turn(3,-1);idle(e,31000);assert(moon(e)>300,'saver despite two lone ticks');});
check('a real turn still counts: two detents the same way soon after each other',()=>{const e=create();idle(e,30000);e.turn(3,1);e.run(200);e.turn(3,1);idle(e,31000);assert.equal(moon(e),0);idle(e,30000);assert(moon(e)>300);});
check('hold HOME on the screensaver: the next one, and it stays on; a tap wakes without leaving the page',()=>{const e=create();idle(e,62000);assert.equal(style(e),0);e.hold('HOME');e.run(700);assert.equal(style(e),1);e.run(600);assert.equal(style(e),2);e.up('HOME');e.run(200);assert.equal(moon(e),0);
 const page=e.root()[1873];e.hold('HOME');e.up('HOME');e.run(100);assert.equal(e.root()[1873],page,'the waking tap is not a HOME press');e.tap('SEQ');e.run(100);e.hold('HOME');e.run(900);e.up('HOME');assert.notEqual(e.root()[1873],page,'awake, HOME works as before');});
check('all four screensavers draw (night, synthwave, matrix, bounce), and the setting turns through them',()=>{const e=create(),p=protocol(e);const fb=()=>Array.from(new Uint16Array(e.memory.buffer,e.w.web_fb(),240*240).slice(0,240*240));const seen=new Set();
 for(let s=0;s<4;s++){p(29,[14,s]);idle(e,62000);const f=fb();seen.add(f.slice(240*100,240*101).join(','));e.tap('HOME');e.run(100);}assert.equal(seen.size,4);});
check('HOME twice opens the About screen; any key closes it',()=>{const e=create();e.run(500);e.tap('HOME');e.run(60);e.tap('HOME');e.run(200);assert(about(e),'about open');e.run(3000);assert(about(e),'stays open');e.key(WHITE[3]);e.run(100);assert(!about(e),'closed');
 e.tap('HOME');e.run(600);e.tap('HOME');e.run(200);assert(!about(e),'two slow taps are just HOME');});
if(failed)process.exit(1);

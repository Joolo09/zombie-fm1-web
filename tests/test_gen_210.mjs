/* SPDX-License-Identifier: MIT — 0.21.0: the generative sequencer, motion (glide, vibrato, LFO shapes and targets,
 * FM envelope, drift, crush, humanize, roll, tremolo, random), the 11 new preset banks and their storage. */
import assert from 'node:assert/strict';import {readFileSync} from 'node:fs';
import {create,BLACK} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.stack);}};
const X={GLIDE:0,VIB:1,VIB_RATE:2,SHAPE:3,TARGET:4,FM_ENV:5,DRIFT:6,CRUSH:7,HUMAN:8,ROLL:9,TREM:10,RANDOM:11,ALGO:12,STYLE:13,DENSITY:14,COMPLEX:15,MUTATE:16,OCTAVES:17,EVOLVE:18,SEED:19};
const LEN=23,steps=(e,t=0)=>Array.from({length:64},(_,i)=>({note:e.root()[12+t*310+54+i*4],vel:e.root()[12+t*310+54+i*4+1]}));
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208),lane=(e,d=0)=>Array.from({length:64},(_,i)=>kit(e)[136+d*192+i*3]);
const inScale=(note,mask,root)=>(mask>>(((note-root)%12+12)%12)&1)===1;
const genOpen=(e,page=4)=>{e.hold('SEQ');e.run(450);e.turn(2,page);};   // pages 5 and 6 of the SEQ layer

/* the browser core (the same C code), for sound checks */
const core=new WebAssembly.Module(readFileSync(new URL('../webapp/groovebox-180/zombie-core.wasm',import.meta.url)));
const fresh=()=>{const w=new WebAssembly.Instance(core,{}).exports;w.z_init();w.z_dm_init();return w;};
const render=(w,blocks,off=-1)=>{const out=[];for(let b=0;b<blocks;b++){if(b===off)w.z_key(0,60,0,0);w.z_render(w.z_buffer(),256);out.push(...new Float32Array(w.memory.buffer,w.z_buffer(),512));}return out;};
const presets=JSON.parse(readFileSync(new URL('../webapp/groovebox-180/presets.js',import.meta.url),'utf8').split('PRESETS=')[1].replace(/;\s*$/,''));

check('the generator pages: hold SEQ, PRESET to page 5 and 6; K1-K4 are ALGORITHM, STYLE, DENSITY, COMPLEXITY / MUTATION, OCTAVES, SEED, EVOLVE',()=>{
 const e=create();e.tap('SEQ');genOpen(e);e.turn(3,2);e.turn(4,3);e.turn(5,-20);e.turn(6,10);
 assert.deepEqual([X.ALGO,X.STYLE,X.DENSITY,X.COMPLEX].map(i=>e.w.z_x_get(0,i)),[2,3,52,66]);
 e.turn(2,1);e.turn(3,5);e.turn(4,1);e.turn(5,7);e.turn(6,2);
 assert.deepEqual([X.MUTATE,X.OCTAVES,X.SEED,X.EVOLVE].map(i=>e.w.z_x_get(0,i)),[37,3,8,2]);
 e.turn(6,9);assert.equal(e.w.z_x_get(0,X.EVOLVE),4,'EVOLVE stops at 8 loops');e.up('SEQ');});

check('GENERATE (black 1) writes a phrase in the style\'s scale; the old steps are one undo away',()=>{
 const e=create();e.tap('SEQ');const before=steps(e);genOpen(e);e.key(BLACK[0]);const s=steps(e),len=e.w.z_get(0,LEN),on=s.slice(0,len).filter(x=>x.vel);
 assert(on.length>=4,'notes '+on.length);for(const x of on)assert(inScale(x.note,0xAB5,0),'CHIPTUNE is major: '+x.note);
 e.up('SEQ');e.w.z_history(0);assert.deepEqual(steps(e),before);});

check('the scale lock and root decide the notes (A minor) on every algorithm and style',()=>{
 for(let algo=0;algo<7;algo++)for(let style=0;style<7;style++){const w=fresh();w.z_x_set(0,X.ALGO,algo);w.z_x_set(0,X.STYLE,style);w.z_x_set(0,X.DENSITY,100);
  assert(w.z_gen(0,0x5AD,9)>0,'hits '+algo+'/'+style);const e=fresh();}
 const e=create();for(let algo=0;algo<7;algo++){e.w.z_x_set(0,X.ALGO,algo);e.w.z_x_set(0,X.STYLE,algo%6);e.w.z_gen(0,0x5AD,9);
  for(const x of steps(e).filter(x=>x.vel))assert(inScale(x.note,0x5AD,9),'algo '+algo+' note '+x.note);}});

check('the same seed gives the same pattern, another seed another one',()=>{
 const a=create(),b=create();for(const e of [a,b]){e.w.z_x_set(0,X.ALGO,1);e.w.z_x_set(0,X.SEED,42);e.w.z_gen(0,0,0);}
 assert.deepEqual(steps(a),steps(b));b.w.z_x_set(0,X.SEED,43);b.w.z_gen(0,0,0);assert.notDeepEqual(steps(a),steps(b));});

check('MOTIF repeats its rhythm every phrase and comes back to it (memorable, not random)',()=>{
 const e=create();e.w.z_x_set(0,X.ALGO,0);e.w.z_x_set(0,X.COMPLEX,50);e.w.z_set(0,LEN,32);e.w.z_gen(0,0,0);   // an 8-step phrase
 const r=steps(e).slice(0,32).map(x=>x.vel?1:0);for(let b=1;b<4;b++)assert.deepEqual(r.slice(b*8,b*8+8),r.slice(0,8),'phrase '+b);
 const n=steps(e).slice(0,32).map(x=>x.vel?x.note:0);assert.deepEqual(n.slice(0,6),n.slice(24,30),'the last phrase restates the first before its cadence');});

check('DENSITY sets how many notes; OCTAVES the range',()=>{
 const hits=d=>{const w=fresh();w.z_x_set(0,X.ALGO,3);w.z_x_set(0,X.STYLE,6);w.z_x_set(0,X.DENSITY,d);let n=0;for(let s=0;s<8;s++){w.z_x_set(0,X.SEED,s);n+=w.z_gen(0,0,0);}return n;};
 assert(hits(10)*2<hits(120),'density '+hits(10)+' vs '+hits(120));
 const span=o=>{const e=create();e.w.z_x_set(0,X.ALGO,7);e.w.z_x_set(0,X.DENSITY,127);e.w.z_x_set(0,X.OCTAVES,o);e.w.z_set(0,LEN,64);e.w.z_gen(0,0,0);const n=steps(e).filter(x=>x.vel).map(x=>x.note);return Math.max(...n)-Math.min(...n);};
 assert(span(1)<=24&&span(4)>span(1),'octaves '+span(1)+' '+span(4));});

check('ARPEGGIO plays chord tones over a progression; the seed picks the direction',()=>{
 const e=create();e.w.z_x_set(0,X.ALGO,5);e.w.z_x_set(0,X.SEED,0);e.w.z_x_set(0,X.DENSITY,127);e.w.z_x_set(0,X.COMPLEX,0);e.w.z_x_set(0,X.OCTAVES,1);e.w.z_gen(0,0xAB5,0);
 const n=steps(e).slice(0,16).map(x=>x.note);assert.deepEqual(n.slice(0,6),[60,64,67,60,64,67],'UP over C major: C E G');
 e.w.z_x_set(0,X.SEED,1);e.w.z_gen(0,0xAB5,0);assert.deepEqual(steps(e).slice(0,3).map(x=>x.note),[67,64,60],'DOWN');});

check('TURING loops its register: a stable pattern that EVOLVE changes bit by bit',()=>{
 const e=create();e.w.z_x_set(0,X.ALGO,6);e.w.z_x_set(0,X.DENSITY,100);e.w.z_gen(0,0,0);const a=steps(e);e.w.z_gen(0,0,0);assert.deepEqual(steps(e),a);
 e.w.z_gen_mutate(0);const b=steps(e);let same=0;for(let i=0;i<16;i++)same+=a[i].vel===b[i].vel&&a[i].note===b[i].note;assert(same<16,'changed');});

check('MUTATE (black 2) changes a few steps, keeps the shape and the scale, one undo',()=>{
 const e=create();e.tap('SEQ');genOpen(e);e.key(BLACK[0]);const a=steps(e);e.key(BLACK[1]);const b=steps(e);let same=0;
 for(let i=0;i<16;i++)same+=a[i].vel===b[i].vel&&(!a[i].vel||a[i].note===b[i].note);assert(same>=10&&same<16,'same '+same);
 for(const x of b.filter(x=>x.vel))assert(inScale(x.note,0xAB5,0));e.up('SEQ');e.w.z_history(0);assert.deepEqual(steps(e),a);});

check('NEW SEED (black 3) changes the seed and regenerates',()=>{const e=create();e.tap('SEQ');genOpen(e);e.key(BLACK[0]);const a=steps(e),s=e.w.z_x_get(0,X.SEED);
 e.key(BLACK[2]);assert.notEqual(e.w.z_x_get(0,X.SEED),s);assert.notDeepEqual(steps(e),a);e.up('SEQ');});

check('drums: GENERATE writes a kit pattern in the style (BREAKCORE: an Amen-style break)',()=>{
 const e=create();e.turn(1,5);e.tap('SEQ');const before=kit(e).slice();e.w.z_x_set(5,X.STYLE,5);genOpen(e);e.key(BLACK[0]);
 const kick=lane(e,0),snare=lane(e,1);for(const i of [0,2,10,11])assert(kick[i],'kick '+i);for(const i of [4,7,12])assert(snare[i],'snare '+i);
 assert.equal(kick[1],0);e.up('SEQ');e.w.z_history(0);assert.deepEqual(kit(e).slice(136),before.slice(136),'one undo restores the kit');});

check('JAM (black 4) builds a whole project: style tempo, the style\'s bank on 5 tracks, notes everywhere, drums',()=>{
 const e=create();e.tap('SEQ');e.w.z_x_set(0,X.STYLE,4);genOpen(e);e.key(BLACK[3]);e.up('SEQ');
 const r=e.root();assert.equal(r[6]|r[7]<<8,128,'ACID tempo');
 for(let t=0;t<5;t++){assert.equal(r[1900+t],128+4*11+t,'ACD preset on track '+(t+1));assert(steps(e,t).some(x=>x.vel),'notes on '+(t+1));}
 assert(lane(e,0).some(v=>v)&&lane(e,4).some(v=>v),'drums: kick and clap');});

check('EVOLVE changes the generated tracks slowly while the song plays, staying in key',()=>{
 const e=create();e.w.z_x_set(0,X.ALGO,0);e.w.z_x_set(0,X.EVOLVE,1);e.w.z_x_set(0,X.MUTATE,60);e.w.z_set(0,LEN,16);e.w.z_gen(0,0x5AD,9);const a=steps(e);
 e.tap('PLAY');e.run(8*2000);e.tap('PLAY');const b=steps(e);let same=0;for(let i=0;i<16;i++)same+=a[i].vel===b[i].vel&&a[i].note===b[i].note;
 assert(same<16,'it moved');for(const x of b.filter(x=>x.vel))assert(inScale(x.note,0x5AD,9),'note '+x.note);
 const f=create();f.w.z_gen(0,0,0);const c=steps(f);f.tap('PLAY');f.run(8*2000);f.tap('PLAY');assert.deepEqual(steps(f),c,'EVOLVE OFF leaves it alone');});

check('motion knobs: ENV page 3 HUMANIZE ROLL TREMOLO RANDOM; LFO page 3 SHAPE TARGET VIBRATO RATE, page 4 GLIDE FM-ENV DRIFT CRUSH',()=>{
 const e=create();e.tap('ENV');e.tap('ENV');e.tap('ENV');e.turn(3,10);e.turn(4,11);e.turn(5,12);e.turn(6,13);
 assert.deepEqual([X.HUMAN,X.ROLL,X.TREM,X.RANDOM].map(i=>e.w.z_x_get(0,i)),[10,11,12,13]);
 e.tap('LFO');e.tap('LFO');e.tap('LFO');e.turn(3,4);e.turn(4,2);e.turn(5,30);e.turn(6,-10);assert.deepEqual([X.SHAPE,X.TARGET,X.VIB,X.VIB_RATE].map(i=>e.w.z_x_get(0,i)),[4,2,30,30]);
 e.tap('LFO');e.turn(3,40);e.turn(4,50);e.turn(5,60);e.turn(6,70);assert.deepEqual([X.GLIDE,X.FM_ENV,X.DRIFT,X.CRUSH].map(i=>e.w.z_x_get(0,i)),[40,50,60,70]);
 assert.equal(e.root()[1900],255,'a motion edit marks the sound as edited');});

check('motion survives SAVE and a reboot; a preset brings its own motion',()=>{
 const e=create();e.w.z_x_set(2,X.GLIDE,77);e.w.z_x_set(2,X.SEED,99);e.tap('SAVE');e.run(200);e.w.web_reset();e.run(150);
 assert.equal(e.w.z_x_get(2,X.GLIDE),77);assert.equal(e.w.z_x_get(2,X.SEED),99);
 const i=presets.findIndex(n=>n==='CHP Pulse Hero');e.w.z_preset(0,i);assert.equal(e.w.z_x_get(0,X.VIB),40);assert.equal(e.w.z_x_get(0,X.CRUSH),30);});

check('the USB protocol: 50 reads the block, 51 sets and clamps, 52 generates / mutates, 53 jams; bad values refused',()=>{
 const e=create(),p=protocol(e);p(51,[0,X.GLIDE,90]);assert.equal(p(50)[8+X.GLIDE],90);p(51,[0,X.SHAPE,200]);assert.equal(e.w.z_x_get(0,X.SHAPE),5);p(51,[6,0,0],1);p(51,[0,20,0],1);
 const block=p(50);p(52,[0,0]);assert(steps(e).some(x=>x.vel));p(52,[0,1]);p(52,[0,2],1);p(53,[2,5]);assert.equal(e.root()[6],124);p(53,[7,0],1);
assert.equal(block.length,e.w.z_x_size());
 const w=fresh(),bad=block.slice();bad[8+X.SHAPE]=9;const at=w.z_transfer();new Uint8Array(w.memory.buffer,at,bad.length).set(bad);assert.equal(w.z_x_apply(at,bad.length),-1);
 new Uint8Array(w.memory.buffer,at,block.length).set(block);assert.equal(w.z_x_apply(at,block.length),0);assert.equal(w.z_x_get(0,X.GLIDE),90);});

check('motion is off by default: the classic presets play bit-identical with the 0.20 motion-free path',()=>{
 for(const p of [0,16,48,100]){const a=fresh(),b=fresh();a.z_preset(0,p);b.z_preset(0,p);for(let i=0;i<12;i++)if(i!==2)b.z_x_set(0,i,0);a.z_key(0,60,100,1);b.z_key(0,60,100,1);assert.deepEqual(render(a,40),render(b,40),'preset '+p);}});

check('every motion value changes the sound, stays finite and under the limiter',()=>{
 const base=w=>{w.z_preset(0,0);w.z_set(0,17,40);w.z_set(0,18,90);};
 const ref=(()=>{const w=fresh();base(w);w.z_key(0,60,110,1);return render(w,60);})();
 for(const [id,v] of [[X.GLIDE,100],[X.VIB,100],[X.SHAPE,3],[X.TARGET,1],[X.TARGET,3],[X.TARGET,4],[X.FM_ENV,120],[X.DRIFT,127],[X.CRUSH,120],[X.TREM,127],[X.RANDOM,127]]){
  const w=fresh();base(w);if(id===X.FM_ENV)w.z_set(0,0,1);if(id===X.GLIDE){w.z_key(0,48,110,1);render(w,4);w.z_key(0,48,0,0);}w.z_x_set(0,id,v);w.z_key(0,60,110,1);const out=render(w,60);
  assert(out.every(Number.isFinite),'finite '+id);assert(Math.max(...out.map(Math.abs))<.98,'level '+id);
  if(id!==X.FM_ENV)assert.notDeepEqual(out,ref,'audible '+id+'='+v);}});

check('121 new presets in 11 banks: names, distinct, audible and unclipped',()=>{
 assert.equal(presets.length,249);const banks=['CHP','GLT','AMB','BRK','ACD','FMK','IND','SCI','GEN','LOF','BLP'];
 for(let b=0;b<11;b++)for(let i=0;i<11;i++)assert(presets[128+b*11+i].startsWith(banks[b]+' '),presets[128+b*11+i]);
 const prints=new Set();for(let p=128;p<249;p++){const w=fresh();w.z_preset(0,p);w.z_key(0,60,110,1);const out=render(w,140,100);let peak=0,e=0;for(const x of out){peak=Math.max(peak,Math.abs(x));e+=x*x;}
  assert(peak>.015&&peak<.98,presets[p]+' peak '+peak);prints.add(e.toFixed(4));}
 assert(prints.size>=118,'distinct '+prints.size);});

check('render cost: all five synth tracks with every motion on stay within 1.6x of motion off',()=>{
 const run=on=>{const w=fresh();for(let t=0;t<5;t++){w.z_preset(t,1);if(on)for(const [id,v] of [[0,40],[1,60],[3,5],[4,2],[5,60],[6,60],[7,60],[10,60],[11,60]])w.z_x_set(t,id,v);for(let n=0;n<2;n++)w.z_key(t,48+n*7+t,100,1);}
  const t0=process.hrtime.bigint();render(w,1500);return Number(process.hrtime.bigint()-t0);};
 run(0);run(1);const off=Math.min(run(0),run(0)),on=Math.min(run(1),run(1));assert(on<off*1.6,'on '+on+' off '+off);console.log('  cost ratio '+(on/off).toFixed(2));});

check('MIDI: bank select (CC0 = 1) then Program Change reaches presets 129-249; CC5 glide, CC76/77 vibrato',()=>{
 const e=create(),cc=(n,v)=>{e.w.web_midi_in(0,11|(0xB0<<8)|(n<<16)|(v<<24));e.run(20);},pc=n=>{e.w.web_midi_in(0,12|(0xC0<<8)|(n<<16));e.run(20);};
 pc(5);assert.equal(e.root()[1900],5);cc(0,1);pc(5);assert.equal(e.root()[1900],133);pc(127);assert.equal(e.root()[1900],127,'past the last preset: the program number alone');
 cc(0,0);pc(7);assert.equal(e.root()[1900],7);cc(5,90);cc(77,40);cc(76,99);assert.deepEqual([X.GLIDE,X.VIB,X.VIB_RATE].map(i=>e.w.z_x_get(0,i)),[90,40,99]);});

if(failed){console.error(failed+' failed');process.exit(1);}

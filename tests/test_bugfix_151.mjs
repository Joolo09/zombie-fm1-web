/* SPDX-License-Identifier: MIT — 0.15.1 regressions found by tests/fuzz_panel.py and the sequencer differential check. */
import assert from 'node:assert/strict';import {create,BLACK} from './native_emulator_harness.mjs';import {protocol,upload} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),3208),lengths=e=>Array.from({length:16},(_,d)=>kit(e)[8+d*8+5]);
const packets=e=>Array.from(new Uint32Array(e.memory.buffer,e.w.web_out_pkts(),e.w.web_out_n()));

check('an imported project with its step bank away from the cursor is shown at the cursor bank',()=>{const e=create(),p=protocol(e);
 const s=e.root().slice();s[1873]=8;s[1874]=0;s[1875]=1;s[1907]=40;   // section SEQ, page 0, bank 1 (steps 17-32), cursor 41 (bank 2)
 upload(p,Array.from(s));p(18);assert.equal(e.root()[1907],40);assert.equal(e.root()[1875],2,'bank follows the cursor');});

check('imported subpage past the section page count wraps instead of selecting a missing page',()=>{const e=create(),p=protocol(e);
 const s=e.root().slice();s[1873]=1;s[1874]=3;upload(p,Array.from(s));p(18);assert.equal(e.root()[1873],1);assert(e.root()[1874]<3,'FX has 3 pages');});

check('doubling a part on a kit with one shared length doubles every drum sound',()=>{const e=create();e.turn(1,5);e.tap('SEQ');
 const k=kit(e);k[136+0*192+0*3]=100;k[136+2*192+4*3]=90;assert.deepEqual(lengths(e),Array(16).fill(32));
 e.hold('SEQ');e.run(450);e.key(BLACK[4]);e.up('SEQ');
 assert.deepEqual(lengths(e),Array(16).fill(64));assert.equal(kit(e)[136+0*192+32*3],100);assert.equal(kit(e)[136+2*192+36*3],90);
 e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.deepEqual(lengths(e),Array(16).fill(32),'one undo step');});

check('doubling a part keeps per-sound lengths when the kit is a polyrhythm',()=>{const e=create();e.turn(1,5);e.tap('SEQ');e.w.z_dm_param(0,5,12);e.w.z_dm_param(3,5,7);
 e.hold('SEQ');e.run(450);e.key(BLACK[4]);e.up('SEQ');const l=lengths(e);assert.equal(l[0],24);assert.equal(l[3],7);assert.equal(l[1],32);});

check('the sequencer plays exactly the stored steps and chords on every track and drum part',()=>{
 let seed=12345;const rnd=()=>{seed^=seed<<13;seed>>>=0;seed^=seed>>>17;seed^=seed<<5;seed>>>=0;return seed/4294967296;},ri=n=>Math.floor(rnd()*n);
 for(let run=0;run<6;run++){const e=create(),s=e.root(),dv=new DataView(s.buffer,s.byteOffset),len=[],pat=[];
  for(let t=0;t<5;t++){len[t]=1+ri(64);dv.setInt16(12+t*310+46,len[t],true);pat[t]=[];
   for(let i=0;i<64;i++){const o=12+t*310+54+i*4,on=rnd()<.35,note=24+ri(80),ch=[];s.set([note,on?1+ri(127):0,127,1+ri(100)],o);
    for(let j=0;j<3;j++){const c=rnd()<.15?note+1+ri(12):0;s[1908+(t*64+i)*3+j]=c?c+1:0;if(c)ch.push(c);}s[3060+t*64+i]=0;pat[t].push(on?[note,...ch]:[]);}}
  const k=kit(e),GM=[36,38,42,46,39,37,45,47,50,49,51,56,54,70,75,76],dl=[];
  for(let d=0;d<16;d++){dl[d]=1+ri(64);k[8+d*8+5]=dl[d];k[8+d*8+4]=0;for(let i=0;i<64;i++)k.set([rnd()<.25?1+ri(127):0,127,1],136+d*192+i*3);}
  e.w.z_tempo(300,0);e.run(50);e.w.web_out_clear();e.tap('PLAY');e.run(5000);e.tap('PLAY');
  const got=[[],[],[],[],[],[]];for(const p of packets(e)){const st=p>>8&255;if((st&0xf0)===0x90&&(p>>>24&127)&&(st&15)<6)got[st&15].push(p>>>16&127);}
  for(let t=0;t<6;t++){const exp=[];for(let n=0;exp.length<got[t].length+20&&n<4000;n++){if(t<5)exp.push(...pat[t][n%len[t]]);else for(let d=0;d<16;d++)if(k[136+d*192+(n%dl[d])*3])exp.push(GM[d]);}
   const silent=exp.length===0;assert(silent?got[t].length===0:got[t].length>3,'track '+(t+1)+' played '+got[t].length);assert.deepEqual(got[t],exp.slice(0,got[t].length),'run '+run+' track '+(t+1));}}});

if(failed)process.exit(1);

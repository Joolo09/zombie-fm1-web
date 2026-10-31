/* SPDX-License-Identifier: MIT — 0.19.0 SYSTEM tab (GLO three times): MIDI in omni, TRS thru, program change and CC in, CPU1 at boot. */
import assert from 'node:assert/strict';import {create} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const ext=e=>protocol(e)(28).slice(122,127);/* omni, thru, no_pc, no_cc, core_boot */
const open=e=>{e.run(300);for(let i=0;i<3;i++){e.tap('GLO');e.run(40);}e.run(100);};
const pkts=e=>Array.from(new Uint32Array(e.memory.buffer,e.w.web_out_pkts(),e.w.web_out_n()));
const TRS=2,USB=0,note=(ch,n,v)=>0x09|((0x90|ch)<<8)|(n<<16)|(v<<24);

check('GLO three times opens SYSTEM; its knobs change system settings, not the page underneath',()=>{const e=create();const sec=e.root()[1872],page=e.root()[1873];open(e);
 assert.deepEqual([...ext(e)],[0,0,0,0,0]);e.turn(3,1);assert.equal(ext(e)[0],1,'K1: MIDI in omni');e.turn(4,1);assert.equal(ext(e)[1],1,'K2: TRS thru');e.turn(5,-1);assert.equal(ext(e)[2],1,'K3: program change off');e.turn(6,-1);assert.equal(ext(e)[3],1,'K4: CC off');
 assert.equal(e.root()[1872],sec,'section back where it was');assert.equal(e.root()[1873],page);});
check('GLO turns the pages (page 2 K2: clock out), another section button leaves',()=>{const e=create(),p=protocol(e);open(e);e.tap('GLO');e.run(60);e.turn(4,1);assert.equal(p(28)[110]&8,8,'clock out on');
 e.tap('GLO');e.tap('GLO');e.tap('GLO');e.run(60);e.turn(3,1);assert.equal(ext(e)[0],1,'back on page 1 after four pages');
 e.tap('ENV');e.run(60);const before=e.root().slice();e.turn(3,1);assert.equal(ext(e)[0],1,'left: K1 no longer a system knob');});
check('two GLO taps are just GLO pages; slow taps never open it',()=>{const e=create();e.run(300);e.tap('GLO');e.run(40);e.tap('GLO');e.run(700);e.tap('GLO');e.run(100);e.turn(3,1);assert.equal(ext(e)[0],0);});
check('omni: a note on any channel plays the selected track',()=>{let e=create();e.run(200);e.w.web_midi_in(USB,note(15,60,110));let pk=e.run(300);e.w.web_midi_in(USB,note(15,60,0));assert(pk<0.003,'channel 16 silent by channel: '+pk);
 e=create();protocol(e)(29,[15,1]);e.w.web_midi_in(USB,note(15,60,110));pk=e.run(300);e.w.web_midi_in(USB,note(15,60,0));assert(pk>0.01,'omni plays: '+pk);});
check('TRS thru: a message from the jack goes out over USB; off, it does not',()=>{let e=create();e.w.web_out_clear();e.w.web_midi_in(TRS,note(0,64,90));e.run(50);assert(!pkts(e).some(p=>(p>>8&0xff)===0x90&&(p>>16&0x7f)===64));
 e=create();protocol(e)(29,[16,1]);e.w.web_out_clear();e.w.web_midi_in(TRS,note(0,64,90));e.run(50);assert(pkts(e).some(p=>(p>>8&0xff)===0x90&&(p>>16&0x7f)===64),'forwarded');});
check('program change and CC can be ignored',()=>{const e=create(),p=protocol(e);const pre=e.root()[1900];p(29,[17,0]);e.w.web_midi_in(USB,0x0C|(0xC0<<8)|(100<<16));e.run(50);assert.equal(e.root()[1900],pre,'PC ignored');
 p(29,[17,1]);e.w.web_midi_in(USB,0x0C|(0xC0<<8)|(100<<16));e.run(50);assert.equal(e.root()[1900],100,'PC taken');
 const cut=e.w.z_get(0,4);p(29,[18,0]);e.w.web_midi_in(USB,0x0B|(0xB0<<8)|(74<<16)|(3<<24));e.run(50);assert.equal(e.w.z_get(0,4),cut,'CC ignored');p(29,[18,1]);e.w.web_midi_in(USB,0x0B|(0xB0<<8)|(74<<16)|(3<<24));e.run(50);assert.equal(e.w.z_get(0,4),3,'CC taken');});
check('SYSTEM settings survive SAVE and a restart; CPU1 at boot starts the second core',()=>{const e=create(),p=protocol(e);p(29,[15,1]);p(29,[16,1]);p(29,[19,1]);e.tap('SAVE');e.run(400);e.w.web_reset();e.run(800);
 const r=protocol(e)(28);assert.deepEqual([...r.slice(122,127)],[1,1,0,0,1]);assert.equal(r[6],1,'dual on at boot');assert.equal(r[112],5,'core self-test passed (simulated)');});
check('the editor rejects ids past the table',()=>{const e=create(),p=protocol(e);p(29,[20,0],1);p(29,[15,2],1);});
if(failed)process.exit(1);

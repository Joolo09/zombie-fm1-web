import assert from 'node:assert/strict';
import {create,WHITE} from './native_emulator_harness.mjs';
let failed=0;
const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),e.w.z_dm_size());
const midiOn=e=>e.w.web_midi_in(0,9|(0x95<<8)|(36<<16)|(100<<24));
check('repeated MIDI drum Note On without intervening Note Off retriggers sound',()=>{const e=create();e.turn(1,5);midiOn(e);e.run(40);assert(e.w.web_lit_keys()&1);e.run(1100);midiOn(e);e.run(40);assert(e.w.web_lit_keys()&1);});
check('repeated external drum attacks record separate steps; late Note Off advances once',()=>{const e=create();e.turn(1,5);e.rec();for(let i=0;i<3;i++){midiOn(e);e.run(40);}for(let i=0;i<3;i++)assert.equal(kit(e)[136+i*3],100);e.w.web_midi_in(0,8|(0x85<<8)|(36<<16));e.run(40);assert.equal(e.root()[1907],3);});
check('REPLACE drum capture resets an old roll but keeps other instruments',()=>{const e=create();e.turn(1,5);kit(e).set([100,50,4],136);kit(e).set([110,60,2],328);e.root()[3444]=1;e.rec();e.key(WHITE[0]);assert.deepEqual(Array.from(kit(e).slice(136,139)),[100,127,1]);assert.deepEqual(Array.from(kit(e).slice(328,331)),[110,60,2]);});
check('OVERDUB drum capture preserves the existing roll',()=>{const e=create();e.turn(1,5);kit(e).set([100,50,4],136);e.rec();e.key(WHITE[0]);assert.equal(kit(e)[138],4);});
check('pitch knob rejects a duplicate chord tone',()=>{const e=create();e.tap('SEQ');e.key(WHITE[0]);e.tap('SEQ');e.turn(4,1);assert.equal(e.root()[1908],65);e.tap('SEQ');e.tap('SEQ');e.turn(4,4);assert.equal(e.root()[66],60);assert.equal(e.root()[1908],65);});
check('chord knob rejects the base pitch and other existing chord tones',()=>{const e=create();e.tap('SEQ');e.key(WHITE[0]);e.tap('SEQ');e.turn(4,1);e.turn(5,1);assert.equal(e.root()[1909],68);e.turn(5,-3);assert.equal(e.root()[1909],68);e.turn(4,-4);assert.equal(e.root()[1908],65);});
for(const [name,prepare,knob,delta,undoExpected] of [
 ['pitch upper limit',e=>e.turn(4,67),4,1,e=>assert.equal(e.root()[66],60)],
 ['duration lower limit',()=>{},6,-1,e=>assert.equal(e.w.z_track_hits(0),0)],
 ['empty chord lower limit',e=>e.tap('SEQ'),4,-1,e=>assert.equal(e.w.z_track_hits(0),0)]
])check('no-op '+name+' preserves revision and useful undo history',()=>{const e=create();e.tap('SEQ');e.key(WHITE[0]);prepare(e);const rev=e.w.z_revision();for(let i=0;i<6;i++)e.turn(knob,delta);assert.equal(e.w.z_revision(),rev);e.hold('EDIT');e.tap('DOWN');e.up('EDIT');undoExpected(e);});
if(failed)process.exitCode=1;

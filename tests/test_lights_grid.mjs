/* Real C panel + native LED output: no LED or sequencer mock. */
import assert from 'node:assert/strict';
import {create,WHITE} from './native_emulator_harness.mjs';
const leds=e=>e.w.web_lit_keys()>>>0;
const step=(e,at)=>Array.from(e.root().slice(66+at*4,70+at*4));
let e=create();e.tap('SEQ');assert.equal(e.w.web_step_grid(),1);
e.key(WHITE[0]);assert.equal(step(e,0)[1],100);e.key(WHITE[0]);assert.equal(step(e,0)[1],100,'selecting an existing step must not erase it');
e.turn(4,7);assert.equal(step(e,0)[0],67);e.key(WHITE[4]);assert.equal(step(e,4)[0],67);assert.equal(step(e,4)[1],100);
e.tap('EDIT');assert.equal(step(e,4)[1],0);assert.equal(e.root()[1873],8);e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.equal(step(e,4)[1],100);
e.tap('SAVE');e.w.web_reset();e.run(80);assert.equal(step(e,4)[0],67);
console.log('PASS direct sixteen-step melody input, pitch knobs, safe reselection, EDIT delete, undo and SAVE');

e=create();e.tap('SEQ');e.hold('EDIT');e.key(0);e.key(4);e.key(7);e.up('EDIT');e.tap('SEQ');e.turn(4,1); // Focus chord 1 with its own knob.
const base=step(e,0)[0];e.tap('EDIT');assert.equal(step(e,0)[0],base);assert.equal(e.root()[1908],61);assert.equal(e.root()[1909],0);
console.log('PASS EDIT can remove a focused chord tone without deleting the other notes');

e=create();e.turn(1,5);e.tap('SEQ');e.key(WHITE[0]);e.key(WHITE[4]);assert.equal(e.w.z_track_hits(5),2);e.turn(4,1);e.key(WHITE[0]);assert.equal(e.w.z_track_hits(5),3);e.tap('EDIT');assert.equal(e.w.z_track_hits(5),2);
e.tap('REC');assert.equal(e.w.web_step_grid(),0);e.key(WHITE[2]);assert.equal(e.w.z_track_hits(5),3);e.tap('REC');assert.equal(e.w.web_step_grid(),1);
console.log('PASS direct drum steps edit only the selected sound; REC returns all sixteen keys to drum pads');

// HOME shows generated pitches. SEQ shows the step position; both clear on STOP.
e=create();e.tap('REC');e.key(WHITE[0]);e.tap('REC');e.root()[3060]=2;e.tap('HOME');e.tap('PLAY');e.run(30);assert(leds(e)&1);e.tap('PLAY');assert.equal(leds(e),0);
e.tap('SEQ');e.tap('PLAY');e.run(20);assert(leds(e)&(1<<WHITE[0]));e.run(130);assert(leds(e)&(1<<WHITE[1]));
console.log('PASS sequencer lights sound pitches on HOME and advances white step lights on SEQ');

e=create();e.tap('ARP');e.turn(3,1);e.w.web_key(0,100,0);e.w.web_key(4,100,0);let union=0,seen=new Set();for(let i=0;i<18;i++){e.run(30);union|=leds(e);seen.add(leds(e));}assert(union&1);assert(union&(1<<4));assert(seen.size>1);e.w.web_key(0,0,0);e.w.web_key(4,0,0);e.run(150);assert.equal(leds(e),0);
console.log('PASS arp lights follow generated gates rather than remaining stuck on the held chord');

e=create();e.turn(1,5);e.w.web_key(WHITE[0],100,0);e.run(40);e.w.web_key(WHITE[0],0,0);e.run(30);assert(leds(e)&(1<<WHITE[0]));e.run(2000);assert.equal(leds(e),0);
e=create();e.w.web_midi_in(0,9|(0x90<<8)|(60<<16)|(100<<24));e.run(40);assert(leds(e)&(1<<7));e.w.web_midi_in(0,8|(0x80<<8)|(60<<16));e.run(40);assert.equal(leds(e),0);
console.log('PASS drum tails illuminate their white pad briefly; external MIDI note-on/off updates the same LEDs');

e=create();e.tap('GLO');e.tap('GLO');e.tap('GLO');e.tap('GLO');e.turn(6,-1);e.w.web_key(0,100,0);e.run(60);assert.equal(leds(e),0);
console.log('PASS glow OFF suppresses key LEDs instead of leaving bright physical-key lights');

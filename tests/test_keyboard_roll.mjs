/* Native FM-1 gestures: never replace the panel with a JS mock. */
import assert from 'node:assert/strict';
import {create,WHITE,BLACK} from './native_emulator_harness.mjs';
const step=(e,at)=>Array.from(e.root().slice(66+at*4,70+at*4));
const chord=(e,at)=>Array.from(e.root().slice(1908+at*3,1911+at*3));
const kit=e=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),e.w.z_dm_size());
let e=create();e.tap('SEQ');e.hold('EDIT');e.run(500);
for(const k of [0,4,7,12])e.key(k);
assert.equal(e.root()[1873],8);assert.equal(e.root()[1906],0);assert.equal(e.root()[1907],0);
assert.equal(step(e,0)[0],53);assert.deepEqual(chord(e,0),[58,61,66]);
const before=e.root().slice();e.key(16);assert.deepEqual(e.root(),before,'fifth chord tone is rejected without history/state damage');
e.key(0);assert.equal(step(e,0)[0],57);assert.deepEqual(chord(e,0),[61,66,0]);
e.up('EDIT');e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.equal(step(e,0)[0],53);
e.turn(3,1);e.hold('EDIT');e.key(WHITE[1]);e.up('EDIT');assert.equal(step(e,1)[0],55);assert.equal(e.root()[1907],1);
e.tap('SAVE');e.w.web_reset();e.run(100);assert.equal(step(e,1)[0],55);
console.log('PASS EDIT+keys adds/removes a four-note chord at the cursor, rejects overflow, undoes and survives SAVE/reboot');

e=create();e.turn(1,5);e.tap('SEQ');e.hold('EDIT');e.run(500);
for(const k of WHITE)e.key(k);
assert.equal(e.w.z_track_hits(5),16);assert.equal(e.root()[1907],0);
for(let d=0;d<16;d++)assert.equal(kit(e)[136+d*192],100);
e.key(WHITE[0]);assert.equal(e.w.z_track_hits(5),15);assert.equal(kit(e)[136],0);assert.equal(kit(e)[328],100);
e.up('EDIT');e.key(BLACK[4]);assert.equal(e.w.z_track_hits(5),16);e.key(BLACK[5]);assert.equal(e.w.z_track_hits(5),15);
e.key(BLACK[1]);assert.equal(e.root()[1907],1);e.key(BLACK[0]);assert.equal(e.root()[1907],0);
e.key(WHITE[1]);e.key(BLACK[3]);assert.equal(e.w.z_track_hits(5),14,'delete erases the selected sound only');
e.key(BLACK[10]);assert.equal(e.w.z_track_hits(5),15);
e.key(BLACK[2]);assert.equal(e.root()[1907],1);assert.equal(e.w.z_track_hits(5),14);
e.key(BLACK[7]);assert.equal(e.root()[1907],17);
console.log('PASS all sixteen pads write the same drum step; black keys navigate, rest, erase, undo/redo, banks and toggle');

e=create();e.turn(1,5);e.tap('REC');for(const k of WHITE)e.key(k);assert.equal(e.w.z_track_hits(5),16);assert.equal(e.root()[1907],16);
e.key(BLACK[2]);assert.equal(e.root()[1907],17);assert.equal(e.root()[1906],1);e.key(WHITE[0]);assert.equal(e.root()[1907],18);
e.tap('REC');for(let page=0;page<3;page++){const old=e.root()[1907];e.turn(3,1);assert.equal(e.root()[1907],old+1);e.tap('SEQ');}
console.log('PASS all sixteen drums record from physical keys, rest keeps REC armed, and K1 stays the step cursor on every drum page');

e=create();e.tap('SEQ');for(let page=0;page<3;page++){const old=e.root()[1907];e.turn(3,1);assert.equal(e.root()[1907],old+1);e.tap('SEQ');}
e.hold('SEQ');e.key(WHITE[7]);e.up('SEQ');assert.equal(e.root()[1907],7);assert.equal(e.w.z_track_hits(0),1);
console.log('PASS melody K1 remains the cursor on every page; held SEQ still toggles the sixteen steps');

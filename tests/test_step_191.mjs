/* SPDX-License-Identifier: MIT — 0.19.1: hold SEQ and turn K1 at once sets the step count (the layer opens on the turn). */
import assert from 'node:assert/strict';import {create} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
check('melody: SEQ held and K1 turned straight away changes the steps, not the preset',()=>{const e=create();e.run(300);const preset=e.root()[1900],len=e.w.z_get(0,23);
 e.hold('SEQ');e.turn(3,-4);e.up('SEQ');e.run(60);assert.equal(e.w.z_get(0,23),len-4);assert.equal(e.root()[1900],preset);});
check('drums: the same gesture sets the kit length',()=>{const e=create();e.run(300);e.turn(1,5);e.run(100);const before=e.w.z_dm_common_length();e.hold('SEQ');e.turn(3,-8);e.up('SEQ');e.run(60);assert.equal(e.w.z_dm_common_length(),before-8);});
check('the turn uses the button: letting SEQ go does not also open the SEQ page',()=>{const e=create();e.run(300);const sec=e.root()[1873];e.hold('SEQ');e.turn(3,1);e.up('SEQ');e.run(60);assert.equal(e.root()[1873],sec);});
check('a plain SEQ tap still opens the piano roll',()=>{const e=create();e.run(300);e.tap("SEQ");e.run(60);assert.equal(e.root()[1873],8);});
check('PRESET while SEQ is held pages the SEQ layer, not the preset',()=>{const e=create();e.run(300);const preset=e.root()[1900];e.hold('SEQ');e.turn(2,1);e.up('SEQ');e.run(60);assert.equal(e.root()[1900],preset);});
if(failed)process.exit(1);

import assert from 'node:assert/strict';
import {create} from './native_emulator_harness.mjs';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};
for(const from of [0,5])check('late release from track '+(from+1)+' does not advance the new track cursor',()=>{
 const e=create();e.turn(1,from);e.tap('REC');e.w.web_key(0,100,0);e.run(40);e.turn(1,1);e.w.web_key(4,100,0);e.run(40);
 assert.equal(e.root()[1907],0);e.w.web_key(0,0,0);e.run(40);assert.equal(e.root()[1907],0);e.w.web_key(4,0,0);e.run(40);assert.equal(e.root()[1907],1);assert.equal(e.w.z_track_hits(from),1);assert.equal(e.w.z_track_hits((from+1)%6),1);
});
for(const layer of ['ENV','LFO','FX'])check('REC exits held '+layer+' without restoring the old page on release',()=>{
 const e=create();e.hold(layer);e.run(450);e.tap('REC');e.up(layer);assert.equal(e.root()[1873],8);assert.equal(e.root()[1906],1);e.key(0);assert.equal(e.root()[1907],1);assert.equal(e.w.z_track_hits(0),1);
});
check('changing banks resets chord focus before EDIT deletes the new step base note',()=>{
 const e=create();e.tap('SEQ');e.key(0);e.tap('SEQ');e.turn(4,1);assert.equal(e.root()[1908],65);
 const s=e.root();s.set([65,100,127,80],66+16*4);s[1908+16*3]=73;e.turn(2,1);assert.equal(e.root()[1907],16);e.tap('EDIT');assert.equal(e.root()[66+16*4],72);assert.equal(e.root()[1908+16*3],0);
});
check('recording start clears held EDIT entry so the next note stays in the take',()=>{
 const e=create();e.tap('SEQ');e.hold('EDIT');e.run(450);e.tap('REC');e.key(0);assert.equal(e.root()[1906],1);assert.equal(e.root()[1907],1);e.up('EDIT');assert.equal(e.w.z_track_hits(0),1);
});
for(const track of [0,5])check('a key held before REC on track '+(track+1)+' does not advance an empty take',()=>{const e=create();e.turn(1,track);e.w.web_key(0,100,0);e.run(40);e.tap('REC');e.w.web_key(0,0,0);e.run(40);assert.equal(e.root()[1907],0);assert.equal(e.w.z_track_hits(track),0);});
check('a key held before REC does not prevent REPLACE from replacing the previous chord',()=>{const e=create();e.root().set([60,100,127,80],66);e.root()[3444]=1;e.w.web_key(4,100,0);e.run(40);e.tap('REC');e.key(0);assert.equal(e.root()[66],53);assert.equal(e.root()[1908],0);assert.equal(e.root()[1907],1);e.w.web_key(4,0,0);e.run(40);assert.equal(e.root()[1907],1);});
for(const from of [0,5])check('returning to old track '+(from+1)+' does not let its stale release skip the new cursor',()=>{const e=create();e.turn(1,from);e.tap('REC');e.w.web_key(0,100,0);e.run(40);e.turn(1,1);e.key(4);assert.equal(e.root()[1907],1);e.turn(1,-1);e.w.web_key(0,0,0);e.run(40);assert.equal(e.root()[1907],1);});
if(failed)process.exitCode=1;

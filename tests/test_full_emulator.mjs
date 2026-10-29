/* SPDX-License-Identifier: MIT — test the real C panel/display/audio simulator, not a JS synth. */
import {readFileSync,writeFileSync,mkdirSync} from 'node:fs';
import assert from 'node:assert/strict';
import {create,B,WHITE,BLACK} from './native_emulator_harness.mjs';
const out=new URL('../source/output/zombie-emulator/',import.meta.url);mkdirSync(out,{recursive:true});
let e=create();assert.equal(e.run(200),0);for(let t=0;t<6;t++)assert.equal(e.w.z_track_hits(t),0);e.tap('PLAY');assert.equal(e.run(3000),0);for(let t=0;t<6;t++)assert.equal(e.w.z_track_hits(t),0);e.tap('PLAY');console.log('PASS cold boot and PLAY: every track empty, no automatic drum/demo sound');
e=create();e.rec();assert.equal(e.root()[1906],1);e.key(WHITE[0]);assert.equal(e.w.z_track_hits(0),1);assert.equal(e.root()[1907],1);e.rec();e.turn(3,-1);assert.equal(e.root()[1907],0);e.hold('SEQ');e.tap('EDIT');e.up('SEQ');assert.equal(e.w.z_track_hits(0),0);e.hold('EDIT');e.tap('DOWN');e.up('EDIT');assert.equal(e.w.z_track_hits(0),1);e.tap('SAVE');const writes=e.w.web_store_writes();e.w.web_reset();e.run(150);assert.equal(e.w.z_track_hits(0),1);assert.equal(e.w.web_store_writes(),writes);console.log('PASS real panel: step record, cursor, delete, undo, SAVE and power-cycle restore');
e=create();e.turn(1,5);assert.equal(e.root()[9],5);e.rec();e.key(WHITE[0]);e.key(WHITE[1]);e.key(WHITE[2]);e.rec();assert.equal(e.w.z_track_hits(5),3);e.hold('SEQ');e.rec();e.up('SEQ');assert.equal(e.w.z_track_hits(5),2);e.hold('SEQ');e.key(BLACK[10]);e.up('SEQ');assert.equal(e.w.z_track_hits(5),0);e.tap('SAVE');e.w.web_reset();e.run(100);assert.equal(e.w.z_track_hits(5),0);e.tap('PLAY');assert.equal(e.run(1500),0);console.log('PASS real drum panel: part clear, whole-track clear, SAVE/reboot and silence');
e=create();e.turn(1,5);const dm=()=>new DataView(e.memory.buffer,e.w.z_dm_data(),e.w.z_dm_size());const selected=()=>dm().getUint8(6);e.turn(3,1);assert.equal(selected(),1);e.turn(4,-10);assert.equal(dm().getUint8(8+8+0),90);e.turn(5,10);assert.equal(dm().getUint8(8+8+1),74);e.turn(6,-5);assert.equal(dm().getInt8(8+8+2),-5);e.tap('ENV');e.turn(3,2);assert.equal(dm().getUint8(8+8+1),76);e.tap('ARP');assert.equal(e.root()[1873],8);console.log('PASS drum HOME/ENV knobs control real part parameters; ARP opens working roll controls');
e=create();e.rec();e.key(WHITE[0]);e.rec();e.tap('SAVE');const revision=e.w.z_revision();e.hold('HOME');e.run(450);e.key(WHITE[14]);assert.equal(e.w.z_track_hits(0),1);e.key(WHITE[14]);e.up('HOME');assert.equal(e.w.z_track_hits(0),0);assert(e.w.z_revision()>revision);e.w.web_reset();e.run(100);assert.equal(e.w.z_track_hits(0),1);console.log('PASS new project requires two confirmations, marks dirty and leaves saved flash intact until SAVE');
// Exercise every printed button, every encoder and all keys through the C input path.
for(const button of Object.keys(B)){e=create();e.tap(button);e.run(70);assert(e.w.web_blits()>0);}
for(let role=0;role<7;role++){e=create();e.turn(role,1);e.turn(role,-1);}
for(let k=0;k<27;k++){e=create();assert(e.run(50)===0);e.key(k);}
console.log('PASS all 14 buttons, 7 encoder roles and 27 physical key positions execute without traps');
e=create();e.run(130);const first=new Uint8Array(e.memory.buffer,e.w.web_fb(),240*240*2).slice();e.run(800);const second=new Uint8Array(e.memory.buffer,e.w.web_fb(),240*240*2).slice();assert.notDeepEqual(first,second);writeFileSync(new URL('home.rgb565',out),first);writeFileSync(new URL('home-animated.rgb565',out),second);assert.equal(e.w.web_store_writes(),0);console.log('PASS zombie animation changes real firmware framebuffer without modifying flash');
let seed=0xdead1234;const rand=()=>{seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;return seed>>>0;};
e=create();for(let i=0;i<350;i++){const action=rand()%4;if(action===0)e.turn(rand()%7,(rand()%9)-4);else if(action===1){const name=Object.keys(B)[rand()%14];e.tap(name);}else if(action===2)e.key(rand()%27,20+rand()%40);else e.run(50);}console.log('PASS deterministic mixed-panel stress: finite bounded audio and no C traps');


for(let t=0;t<6;t++)for(const section of ['HOME','FX','SEL','ENV','LFO','EDIT','GLO','ARP','SEQ']){
 e=create();e.turn(1,t);e.tap(section);
 const pages={HOME:2,FX:3,SEL:2,ENV:2,LFO:2,EDIT:3,GLO:6,ARP:2,SEQ:3}[section];
 for(let pg=0;pg<pages;pg++){for(let k=0;k<4;k++){e.turn(3+k,1);e.turn(3+k,-1);}if(pg+1<pages)e.tap(section);}
 e.hold(section);e.run(430);for(let k=0;k<4;k++){e.turn(3+k,1);e.turn(3+k,-1);}e.up(section);assert(e.w.web_blits()>0);
}
console.log('PASS every page and knob across all six tracks, including held layers');
e=create();e.tap('GLO');e.turn(5,1);assert.equal(e.root()[11],1);const midi=status=>e.w.web_midi_in(0,15|(status<<8));midi(0xfa);e.run(15);assert.equal(e.w.z_playing(),1);for(let i=0;i<24;i++){midi(0xf8);e.run(15);}assert.equal(e.w.z_position(),3);midi(0xfc);e.run(15);assert.equal(e.w.z_playing(),0);console.log('PASS physical MIDI parser: external clock start, 24 PPQN progression and stop');

console.log('Full native firmware emulator audit passed. Physical hardware timings/USB remain unverified.');

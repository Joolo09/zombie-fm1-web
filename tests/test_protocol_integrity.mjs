import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {create} from './native_emulator_harness.mjs';
import {protocol,upload,crc32,u32} from './native_protocol_harness.mjs';
let e=create(),p=protocol(e),bad=0;const saved=e.root().slice();
const rejectProject=bytes=>{upload(p,bytes);p(18,[],3);assert.deepEqual(e.root(),saved);bad++;};
const schema=JSON.parse(readFileSync('webapp/groovebox-140/schema.json','utf8'));
for(let t=0;t<6;t++)for(let id=0;id<27;id++)for(const v of [schema[id].min-1,schema[id].max+1]){const b=saved.slice();new DataView(b.buffer).setInt16(12+t*310+id*2,v,true);rejectProject(b);}
for(const [at,v] of [[0,0],[4,0],[6,0],[8,61],[9,6],[10,2],[11,2],[1872,4],[1873,9],[1874,6],[1875,4],[1876,5],[1882,4],[1888,0],[1894,2],[1900,96],[1906,2],[1907,64],[3444,2],[3445,2],[3446,2],[3447,128],[3448,4]]){const b=saved.slice();b[at]=v;rejectProject(b);}
for(let t=0;t<6;t++)for(const at of [0,63])for(const [off,v] of [[66+t*310+at*4,128],[67+t*310+at*4,128],[68+t*310+at*4,128],[69+t*310+at*4,0],[1908+(t*64+at)*3,129],[3060+t*64+at,64]]){const b=saved.slice();b[off]=v;rejectProject(b);}
console.log('PASS '+bad+' malformed project variants rejected without partial state changes');
const kit=()=>new Uint8Array(e.memory.buffer,e.w.z_dm_data(),e.w.z_dm_size());const oldKit=kit().slice();let invalidKit=0;
const rejectKit=b=>{upload(p,b,47);p(48,[],3);assert.deepEqual(kit(),oldKit);invalidKit++;};
for(let d=0;d<16;d++)for(const [field,values] of [[0,[128]],[1,[0,128]],[2,[-25,25]],[3,[-65,64]],[4,[2]],[5,[0,65]],[6,[1]],[7,[1]]])for(const v of values){const b=oldKit.slice();b[8+d*8+field]=v;rejectKit(b);}
for(let d=0;d<16;d++)for(const at of [0,63])for(const [field,v] of [[0,128],[1,128],[2,0],[2,5]]){const b=oldKit.slice();b[136+d*192+at*3+field]=v;rejectKit(b);}
console.log('PASS '+invalidKit+' malformed drum variants rejected without partial kit changes');
// Transfer errors must leave the current project intact and permit a fresh retry.
upload(p,saved,16,crc32(saved)^1);p(18,[],3);assert.deepEqual(e.root(),saved);
p(16,u32(crc32(saved)));p(17,[1,0,1],2);p(17,[0,0,...saved.slice(0,32)]);p(18,[],3);assert.deepEqual(e.root(),saved);
upload(p,saved);p(14);p(18,[],1);assert.deepEqual(e.root(),saved);upload(p,saved);p(18);assert.deepEqual(e.root(),saved);
p(17,[0,0,...new Uint8Array(250)],1);assert.deepEqual(e.root(),saved);
console.log('PASS bad CRC, wrong offset, truncation, cancellation, oversized input and retry');
// Snapshot chunks stay coherent even if the live state changes between requests.
const first=p(19,[0,0,128,0,0,0,0]),token=Array.from(first.slice(0,4)),snapshot=new Uint8Array(saved.length);snapshot.set(first.slice(10));p(3,[0,4,20,0]);for(let at=128;at<saved.length;at+=128){const amount=Math.min(128,saved.length-at);snapshot.set(p(19,[at&255,at>>8,amount,...token]).slice(10),at);}assert.deepEqual(snapshot,saved);p(19,[0,0,128,...u32(0xffffffff)],2);assert.equal(p(19,[0,0,128,0,0,0,0])[10+20],20);
console.log('PASS paginated snapshot remains coherent across live edits and rejects stale tokens');
for(let len=1;len<=64;len++){e=create();p=protocol(e);p(3,[0,23,len,0]);p(44,[0,5,len,0]);p(21,[8,1]);for(const status of [0xf2,0xfb,0xf8]){e.w.web_midi_in(0,status===0xf2?3|(0xf2<<8)|(2<<16)|(1<<24):15|(status<<8));e.run(4);}const live=p(20);assert.equal(e.w.z_position(),130);assert.equal(live[23],130%len);e.w.web_midi_in(0,15|(0xfc<<8));e.run(4);assert.equal(e.w.z_playing(),0);}
console.log('PASS all 64 loop lengths follow MIDI song position, continue, tick and stop');
// Exercise held keys, page transitions and combinations, checking bounded real C audio.
e=create();p=protocol(e);let seed=0x9219dead,mask=0;const rand=()=>{seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;return seed>>>0;};const buttons=['HOME','SEQ','REC','PLAY','EDIT','ARP','ENV','LFO','FX','SEL','GLO','DOWN','UP'];
for(let i=0;i<2500;i++){switch(rand()%4){case 0:e.turn(rand()%7,(rand()%15)-7);break;case 1:{const k=rand()%27;mask^=1<<k;e.w.web_keys(mask);e.run(20);break;}case 2:e.tap(buttons[rand()%buttons.length]);break;default:e.run(25);}const s=e.root(),v=new DataView(s.buffer,s.byteOffset,s.byteLength);assert(s[9]<6&&s[1907]<64);assert(v.getUint16(6,true)>=30&&v.getUint16(6,true)<=300);for(let t=0;t<6;t++)assert(v.getInt16(12+t*310+46,true)>=1&&v.getInt16(12+t*310+46,true)<=64);}
e.w.web_keys(0);e.run(50);p(5,[0]);e.run(30);assert.equal(e.w.z_voice_count(),0);p(9);const before=e.root().slice();e.w.web_reset();e.run(60);const after=e.root();assert.equal(after[1906],0);for(let t=0;t<6;t++){assert.deepEqual(after.slice(12+t*310,12+(t+1)*310),before.slice(12+t*310,12+(t+1)*310));}assert.deepEqual(after.slice(1908,3444),before.slice(1908,3444));
console.log('PASS 2500 held-key/panel/encoder transitions: bounded audio/state, panic, SAVE and reboot');

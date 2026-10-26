import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import vm from 'node:vm';
const messages=[];let Processor;const scope={WebAssembly,performance,Date,currentTime:0,registerProcessor:(_name,p)=>Processor=p,AudioWorkletProcessor:class{constructor(){this.port={postMessage:m=>messages.push(m)};}}};vm.createContext(scope);vm.runInContext(readFileSync('emulator/worklet.js','utf8'),scope);const p=new Processor();const wasm=readFileSync('emulator/app.wasm');await p.onMessage({type:'load',wasm,master:3000,store:{}});const run=n=>{for(let i=0;i<n;i++){scope.currentTime+=128/44100;p.process([],[[new Float32Array(128),new Float32Array(128)]]);}};run(30);
await p.onMessage({type:'buttons',mask:1<<11});await p.onMessage({type:'buttons',mask:0});run(40);let w=p.ex;const root=()=>new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size());assert.equal(root()[1906],1);await p.onMessage({type:'keys',mask:1});await p.onMessage({type:'keys',mask:0});run(60);assert.equal(w.z_track_hits(0),1);assert.equal(root()[1907],1);console.log('PASS browser-fast REC and note clicks preserve both physical edges');
await p.onMessage({type:'buttons',mask:1<<11});await p.onMessage({type:'buttons',mask:0});run(50);messages.length=0;await p.onMessage({type:'buttons',mask:1<<7});await p.onMessage({type:'buttons',mask:0});run(80);assert.equal(messages.filter(m=>m.store).length,1);run(200);assert.equal(messages.filter(m=>m.store).length,1);console.log('PASS SAVE publishes one persistent snapshot, without repeated localStorage writes');
await p.onMessage({type:'reset'});run(80);assert.equal(w.z_track_hits(0),1);assert.equal(root()[1906],0);await p.onMessage({type:'keys',mask:1});await p.onMessage({type:'release'});run(30);assert.equal(p.inputQueue.length,0);assert.equal(p.inputMasks.keys,0);console.log('PASS emulator power cycle restores SAVE; focus loss cancels queued input');

// A page release must be polled by C before EDIT and note events arrive.
await p.onMessage({type:'reset'});run(80);
await p.onMessage({type:'buttons',mask:1<<9});await p.onMessage({type:'buttons',mask:0});await p.onMessage({type:'buttons',mask:1<<4});await p.onMessage({type:'keys',mask:1<<4});await p.onMessage({type:'keys',mask:0});run(110);
assert.equal(root()[1873],8);assert.equal(root()[1906],0);assert.equal(root()[1907],1);assert.equal(root()[66+4],57);await p.onMessage({type:'buttons',mask:0});run(40);
console.log('PASS fast SEQ → EDIT → keyboard preserves page transitions and edits the cursor rather than opening synth controls');

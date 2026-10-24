/* SPDX-License-Identifier: GPL-3.0-only */
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
const module=await WebAssembly.compile(await readFile(new URL('../webapp/groovebox-090/zombie-core.wasm',import.meta.url)));
let Processor;
globalThis.AudioWorkletProcessor=class {constructor(){this.messages=[];this.port={postMessage:d=>this.messages.push(d)};}};
globalThis.registerProcessor=(name,type)=>{assert.equal(name,'zombie-core');Processor=type;};
const {CoreDSP}=await import('../webapp/groovebox-090/worklet.js');
const p=new Processor({processorOptions:{module}}),w=p.dsp.w;
const send=d=>p.port.onmessage({data:d});
const five=()=>new Uint8Array(w.memory.buffer,w.z_snapshot()+12,5*310).slice(),before=five();
send({type:'hFocus',on:true});send({type:'hControl',kind:2,index:4,value:1});
send({type:'hControl',kind:0,index:10,value:1});send({type:'hControl',kind:0,index:10,value:0});
const out=[[new Float32Array(128),new Float32Array(128)]];
for(let i=0;i<48;i++){assert.equal(p.process([],out),true);for(const c of out[0])for(const v of c)assert(Number.isFinite(v));}
assert.equal(w.z_playing(),1);assert(p.messages.some(d=>d.hFrame?.length===115200&&d.hProject?.length===3288));
send({type:'hSnapshot',requestId:77});const reply=p.messages.find(d=>d.requestId===77);assert.equal(reply.drum.length,3288);
send({type:'hImport',bytes:reply.drum});send({type:'hImport',bytes:new Uint8Array(3288)});
assert(p.messages.some(d=>d.error==='Invalid drum project'));
send({type:'hControl',kind:0,index:7,value:1});assert.equal(w.z_playing(),0);
for(let i=0;i<16;i++)p.process([],out);assert(p.messages.some(d=>d.hFlash?.length===0x100000));
send({type:'hFocus',on:false});assert.deepEqual(five(),before);
const dsp=new CoreDSP(module);assert.throws(()=>dsp.sample(0,new Int16Array(20000),22050),/drum suite/);
console.log('PASS actual AudioWorklet controls, transport, frame/project snapshots, save, invalid import and PCM bounds');

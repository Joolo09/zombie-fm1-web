/* SPDX-License-Identifier: MIT — USB OTA simulation; never opens real MIDI. */
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {Updater, pack7, unpack7, pace} from '../vendor/fm1ota.js';
import {checkPackage} from '../package-check.js';

const base=new URL('../',import.meta.url);
const manifest=JSON.parse(await readFile(new URL('releases.json',base),'utf8'));
const meta=manifest.releases[0];
const raw=new Uint8Array(await readFile(new URL(meta.file,base)));
const checked=await checkPackage(raw,meta);
let passed=0;
async function test(name,fn){await fn();console.log('PASS '+name);passed++;}
const frame=u=>[240,...pack7(u),247];
function identity(text){const u=Array(34).fill(0);u[1]=89;u[2]=17;u.splice(6,text.length,...new TextEncoder().encode(text));return frame(u);}
function request(addr,len){const u=[0,89,48,8,0,0,0,addr&255,(addr>>>8)&255,(addr>>>16)&255,(addr>>>24)&255,len&255,(len>>8)&255,(len>>16)&255];u.push(~u.slice(6).reduce((a,b)=>a+b,0)&255);return frame(u);}

class VirtualFM1 {
 constructor({resume=false,mismatch=false,wrongLoader=false,badRequest=false,disconnect=false,appReady=true,stall=false}={}){
  this.options={mismatch,wrongLoader,badRequest,disconnect,appReady,stall};this.access={inputs:new Map(),outputs:new Map()};this.upgrades=0;this.blocks=0;this.closed=0;this.stage=resume?'write':'verify';this.switchTo(resume?(wrongLoader?'ota-Other_1':'ota-FM-1_1'):'FM-1_61000703');
 }
 switchTo(id){
  for(const p of [...this.access.inputs.values(),...this.access.outputs.values()])p.state='disconnected';
  const name=id.startsWith('ota')?'ota-FM-1':'FM-1';
  const input={id:'in-'+id,name,state:'connected',open:async()=>{},close:async()=>{this.closed++;},onmidimessage:null};
  const output={id:'out-'+id,name,state:'connected',open:async()=>{},close:async()=>{this.closed++;},send:bytes=>this.send(input,output,bytes)};
  this.id=id;this.input=input;this.output=output;this.access.inputs=new Map([[input.id,input]]);this.access.outputs=new Map([[output.id,output]]);
 }
 emit(input,bytes){queueMicrotask(()=>input.onmidimessage?.({data:Uint8Array.from(bytes)}));}
 send(input,output,bytes){
  if(output.state==='disconnected')throw Error('USB lost');
  if(bytes[1]===0x7D){
   assert.equal(this.stage,'done');assert.equal(bytes[5],20);
   if(this.options.appReady){const data=[0,4,...Array(22).fill(0)],reply=[240,125,90,67,bytes[4],84];for(let i=0;i<data.length;i+=7)reply.push(0,...data.slice(i,i+7));reply.push(247);this.emit(input,reply);}
   return;
  }
  if(bytes.length===10&&bytes[1]===0&&bytes[2]===50&&bytes[7]===64){this.emit(input,identity(this.id));return;}
  if(bytes[1]===34){
   this.upgrades++;this.at=0;
   if(this.options.badRequest&&this.stage==='write'){this.emit(input,request(checked.image.length+1,64));return;}
   this.emit(input,request(0,Math.min(512,checked.image.length)));return;
  }
  const u=unpack7(bytes.slice(1,-1));assert.equal(u[2],48);
  const addr=(u[7]|u[8]<<8|u[9]<<16|u[10]<<24)>>>0;
  if(addr===0xE0000000){this.stage='write';this.switchTo('ota-FM-1_1');return;}
  if(addr===0xF0000000){this.stage='done';this.switchTo(this.options.mismatch?'FM-1_61000703':meta.product);return;}
  const len=u[11]|u[12]<<8;assert.deepEqual(Uint8Array.from(u.slice(14,14+len)),checked.image.slice(addr,addr+len));this.blocks++;
  if(this.options.disconnect&&this.stage==='write'){input.state=output.state='disconnected';return;}
  if(this.options.stall&&this.stage==='write'&&addr>=65536)return;   // the loader stops asking, the port stays up
  this.at=addr+len;
  if(this.stage==='verify')this.emit(input,request(0xE0000000,0));
  else if(this.at===checked.image.length)this.emit(input,request(0xF0000000,0));
  else this.emit(input,request(this.at,Math.min(512,checked.image.length-this.at)));
 }
 updater(extra={}){return new Updater(this.access,()=>{},{sleep:async()=>{},...extra});}
}

await test('real current and rollback packages pass size/SHA/identity/loader checks',async()=>{
 for(const r of manifest.releases){const result=await checkPackage(new Uint8Array(await readFile(new URL(r.file,base))),r);assert.equal(result.product,r.product);}
});
await test('corrupt package rejected before any device work',async()=>{const corrupt=raw.slice();corrupt[2000]^=1;await assert.rejects(checkPackage(corrupt,meta),{code:'badpkg'});});
await test('truncated package and wrong identity rejected',async()=>{await assert.rejects(checkPackage(raw.slice(0,900),meta),{code:'badpkg'});await assert.rejects(checkPackage(raw,{...meta,product:'Other_1'}),{code:'badpkg'});});
await test('7-bit protocol preserves all byte values',()=>{const bytes=Uint8Array.from({length:1024},(_,i)=>i&255);assert.deepEqual(Uint8Array.from(unpack7(pack7(bytes))),bytes);});
await test('normal install serves exact full image and verifies reboot identity',async()=>{
 const device=new VirtualFM1(),steps=[];assert.equal(await device.updater().install(checked.image,meta.product,k=>steps.push(k)),meta.product);assert.equal(device.upgrades,2);assert.equal(steps.at(-1),'done');assert(device.blocks>1000);assert(device.closed>=6);
});
await test('loader recovery verifies restarted identity before success',async()=>{
 const device=new VirtualFM1({resume:true}),steps=[];assert.equal(await device.updater().install(checked.image,meta.product,k=>steps.push(k)),meta.product);assert.equal(device.upgrades,1);assert.deepEqual(steps.filter(k=>k!=='write'),['loader','reboot','done']);
});
await test('identity without a running application cannot report done',async()=>{
 const device=new VirtualFM1({resume:true,appReady:false}),steps=[];
 await assert.rejects(device.updater().install(checked.image,meta.product,k=>steps.push(k)),{code:'appnotready'});
 assert(!steps.includes('done'));assert(device.closed>=4);
});
await test('normal and recovery mismatch never report done',async()=>{
 for(const resume of [false,true]){const device=new VirtualFM1({resume,mismatch:true}),steps=[];await assert.rejects(device.updater().install(checked.image,meta.product,k=>steps.push(k)),{code:'mismatch'});assert(!steps.includes('done'));}
});
await test('public resume path also checks the new version',async()=>{
 const good=new VirtualFM1({resume:true});assert.equal(await good.updater().resume(checked.image,meta.product),true);
 const bad=new VirtualFM1({resume:true,mismatch:true});await assert.rejects(bad.updater().resume(checked.image,meta.product),{code:'mismatch'});
});
await test('wrong loader model is rejected before UPGRADE',async()=>{const device=new VirtualFM1({resume:true,wrongLoader:true});await assert.rejects(device.updater().install(checked.image,meta.product),{code:'model'});assert.equal(device.upgrades,0);});
await test('out-of-range read request releases ports without success',async()=>{const device=new VirtualFM1({resume:true,badRequest:true});await assert.rejects(device.updater().install(checked.image,meta.product),{code:'badreq'});assert(device.closed>=2);});
await test('USB loss cannot report success',async()=>{const device=new VirtualFM1({resume:true,disconnect:true}),steps=[];await assert.rejects(device.updater().install(checked.image,meta.product,k=>steps.push(k)),{code:'lost'});assert(!steps.includes('done'));});
await test('failed final acknowledgement never reports a completed transfer',async()=>{
 for(const addr of [0xE0000000,0xF0000000]){
  const link={read:async()=>request(addr,0),send:()=>false};
  const result=await new Updater({}).serve(link,checked.image,addr,1000);
  assert.equal(result.finished,false);assert.equal(result.lost,true);
 }
});
await test('unrelated MIDI outputs are never probed',async()=>{
 let sent=0;const access={inputs:new Map([['x',{name:'FM-1',state:'connected'}]]),outputs:new Map([['x',{name:'Other Synth',state:'connected',send(){sent++;}}]])};assert.equal(await new Updater(access).find(),null);assert.equal(sent,0);
});
await test('Hortator-named ports are discovered by the FM-1 identity handshake',async()=>{
 const device=new VirtualFM1();device.input.name=device.output.name='Hortator';const found=await device.updater().find();assert(found);assert.equal(found.id.model,'FM-1');await found.link.close();
});
await test('partially opened MIDI ports are released after output open failure',async()=>{
 let closed=0;const input={name:'FM-1',state:'connected',open:async()=>{},close:async()=>{closed++;}},output={name:'FM-1',state:'connected',open:async()=>{throw Error('Busy');},close:async()=>{closed++;}};
 assert.equal(await new Updater({inputs:new Map([['in',input]]),outputs:new Map([['out',output]])}).find(),null);assert.equal(closed,2);
});
await test('a loader that stops asking warns the page, then fails as stopped instead of hanging',async()=>{
 const device=new VirtualFM1({resume:true,stall:true}),steps=[];
 await assert.rejects(device.updater({stallMs:60,loaderIdleMs:400}).install(checked.image,meta.product,k=>steps.push(k)),{code:'stopped'});
 assert(steps.includes('stall'),'stall reported');assert(!steps.includes('done'));
});
await test('block pacing in a hidden tab does not wait on throttled timers',async()=>{
 const realTimeout=globalThis.setTimeout;globalThis.document={hidden:true};
 globalThis.setTimeout=(f,ms,...a)=>realTimeout(f,Math.max(ms,1000),...a);   // what a hidden Chrome tab does
 try{const t=performance.now();for(let i=0;i<20;i++)await pace(10);const ms=performance.now()-t;assert(ms<1500,'20 blocks took '+Math.round(ms)+' ms');}
 finally{globalThis.setTimeout=realTimeout;delete globalThis.document;}
});
console.log(`${passed} web-installer checks passed. USB device was simulated; no hardware was flashed.`);

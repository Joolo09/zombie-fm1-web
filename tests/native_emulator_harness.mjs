import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
const mod=new WebAssembly.Module(readFileSync(new URL('../emulator/app.wasm',import.meta.url)));
const B={FX:0,SEL:1,ENV:2,LFO:3,EDIT:4,GLO:5,HOME:6,SAVE:7,ARP:8,SEQ:9,PLAY:10,REC:11,DOWN:12,UP:13};
const WHITE=[0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26];
const BLACK=[1,3,5,8,10,13,15,17,20,22,25];
function create(){let memory;const wasi=new Proxy({clock_time_get:(_id,_p,out)=>{new DataView(memory.buffer).setBigUint64(out,0n,true);return 0;},fd_write:(_fd,iov,n,out)=>{const v=new DataView(memory.buffer);let count=0;for(let i=0;i<n;i++)count+=v.getUint32(iov+i*8+4,true);v.setUint32(out,count,true);return 0;},proc_exit:n=>{throw Error('WASI exit '+n);}},{get:(t,k)=>t[k]||(()=>0)});const w=new WebAssembly.Instance(mod,{wasi_snapshot_preview1:wasi,env:new Proxy({},{get:()=>()=>0})}).exports;memory=w.memory;w._initialize?.();w.web_boot();w.web_master(3500);let buttons=0;
 const root=()=>new Uint8Array(memory.buffer,w.z_snapshot(),w.z_state_size());
 const run=ms=>{let peak=0;for(let n=0;n<Math.ceil(ms*44.1);n+=128){w.web_render(128);for(const x of new Float32Array(memory.buffer,w.web_out_l(),128)){assert(Number.isFinite(x)&&Math.abs(x)<=1.001);peak=Math.max(peak,Math.abs(x));}}return peak;};
 const hold=name=>{buttons|=1<<B[name];w.web_buttons(buttons);run(35);};const up=name=>{buttons&=~(1<<B[name]);w.web_buttons(buttons);run(35);};const tap=name=>{hold(name);up(name);};const key=(k,ms=55)=>{w.web_key(k,100,0);run(ms);w.web_key(k,0,0);run(40);};const turn=(role,n)=>{w.web_enc(role,n);run(50);};
 return {w,memory,root,run,hold,up,tap,key,turn};}

export {create,B,WHITE,BLACK};

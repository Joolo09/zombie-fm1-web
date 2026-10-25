/* SPDX-License-Identifier: GPL-3.0-only */
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {DrumStore} from '../webapp/groovebox-090/drum-store.js';
const module=await WebAssembly.compile(await readFile(new URL('../source/output/editions-0.9.1/groovebox.wasm',import.meta.url)));
const make=()=>{const w=new WebAssembly.Instance(module,{}).exports;w.z_init();return w;};
const worker=make();worker.h_set(0,1,5);worker.h_set_global(15,4);assert.equal(worker.h_save(),0);
const image=()=>new Uint8Array(worker.memory.buffer,worker.h_flash(),0x100000).slice();
const local=make(),five=new Uint8Array(local.memory.buffer,local.z_snapshot()+12,1550).slice();
let resolveRead;const read=new Promise(resolve=>{resolveRead=resolve;});let disk,writes=0;
const store=new DrumStore(local,{read:()=>read,write:async bytes=>{disk=bytes;writes++;}});
assert.equal(store.loaded,false);resolveRead(image());await store.ready;
assert.equal(local.h_get(0,1),5);assert.deepEqual(new Uint8Array(local.memory.buffer,local.z_snapshot()+12,1550),five);
worker.h_set(0,1,9);assert.equal(worker.h_save(),0);await store.accept(image());assert.equal(writes,1);
// The exact image used on audio OFF/ON must contain the worker's latest save.
const restarted=make();new Uint8Array(restarted.memory.buffer,restarted.h_flash(),disk.length).set(new Uint8Array(local.memory.buffer,local.h_flash(),disk.length));restarted.h_init();assert.equal(restarted.h_get(0,1),9);
const unchanged=new Uint8Array(local.memory.buffer,local.h_flash(),0x100000).slice();await assert.rejects(store.accept(new Uint8Array(1)),/Invalid drum flash/);assert.deepEqual(new Uint8Array(local.memory.buffer,local.h_flash(),0x100000),unchanged);
let release;const gate=new Promise(resolve=>{release=resolve;});const values=[];
const serial=new DrumStore(make(),{read:()=>null,write:async bytes=>{values.push(bytes[0]);if(values.length===1)await gate;}});await serial.ready;
const first=image(),second=image();first[0]=1;second[0]=2;const a=serial.accept(first),b=serial.accept(second);await new Promise(setImmediate);assert.deepEqual(values,[1]);release();await Promise.all([a,b]);await serial.flush();assert.deepEqual(values,[1,2]);
console.log('PASS async restore, worker flash mirroring across audio restart, invalid images and ordered durable saves');

/* SPDX-License-Identifier: MIT */
import assert from 'node:assert/strict';
const {AudioEngine}=await import('../webapp/'+(process.argv[2]||'groovebox-090')+'/core.js');
let contexts=[],fail=true;
globalThis.AudioContext=class{constructor(){this.sampleRate=44100;this.audioWorklet={addModule:async()=>{if(fail)throw Error('Worklet load failed');}};this.destination={};contexts.push(this);}async resume(){}async close(){this.closed=true;}createGain(){return {gain:{},connect(){}};}};
globalThis.AudioWorkletNode=class{constructor(){this.port={postMessage:d=>{queueMicrotask(()=>this.port.onmessage?.({data:{requestId:d.requestId,error:'Specific engine error'}}));}};}connect(gain){return gain;}disconnect(){this.disconnected=true;}};
const engine=new AudioEngine({},()=>{});
await assert.rejects(engine.start(),/Worklet load failed/);assert(contexts[0].closed);assert.equal(engine.context,null);
fail=false;await Promise.all([engine.start(),engine.start()]);assert.equal(contexts.length,2);assert(engine.node);
await assert.rejects(engine.command('flush'),/Specific engine error/);assert.equal(engine.requests.size,0);
engine.node.port.postMessage=()=>{throw Error('Clone failed');};await assert.rejects(engine.command('bad'),/Clone failed/);assert.equal(engine.requests.size,0);engine.node.port.postMessage=()=>{};await engine.close();assert(contexts[1].closed);assert.equal(engine.node,null);
console.log('PASS failed audio startup releases context, concurrent startup shares one worker, command errors reject immediately');

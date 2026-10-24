export const EDITION="groovebox";
/* SPDX-License-Identifier: MIT - original editor state, wire protocol and audio adapter. */
export const MODELS=['ANALOG','FM-2','PHASE','WAVETABLE','NOISE','KICK','SNARE','HAT','SAMPLE'];
export {PRESETS} from './presets.js';
export const SAMPLES=['MC Zombie','MC Hurt','MC Death','MC Creeper','MC Boom','MC XP','USR1','USR2','USR3','USR4','ZC Kick','ZC Snare','ZC Hat','ZC Clap'];
export const STATE_BYTES=3452;
export const LEGACY_STATE_BYTES=1872;
export function pack7(bytes){const out=[];for(let i=0;i<bytes.length;i+=7){let hi=0;const chunk=[];for(let j=0;j<7&&i+j<bytes.length;j++){hi|=(bytes[i+j]>>7)<<j;chunk.push(bytes[i+j]&127);}out.push(hi,...chunk);}return out;}
export function unpack7(bytes){const out=[];for(let i=0;i<bytes.length;){const hi=bytes[i++];for(let j=0;j<7&&i<bytes.length;j++)out.push(bytes[i++]|((hi>>j&1)<<7));}return Uint8Array.from(out);}
export const u16=n=>[n&255,(n>>8)&255];
export const u32=n=>[n&255,(n>>>8)&255,(n>>>16)&255,(n>>>24)&255];
export function crc32(bytes){let c=0xffffffff;for(const b of bytes){c^=b;for(let j=0;j<8;j++)c=(c>>>1)^((c&1)?0xedb88320:0);}return (~c)>>>0;}
export function upgradeState(input){
 let s=input;
 if(s?.format==='zombie-core/1')s={...s,format:'zombie-core/2',octave:0,section:0,subpage:0,bank:0,recording:0,stepEdit:0,presets:Array(6).fill(255),arp:Array.from({length:6},()=>({mode:0,rate:1,octaves:1,hold:0}))};
 if(s?.format==='zombie-core/2')s={...s,format:'zombie-core/3',recMode:0,recQuant:0,metronome:0,fxAmount:90,fxRate:0,tracks:s.tracks.map(t=>({...t,steps:t.steps.map(st=>({...st,chord:[0,0,0],duration:0}))}))};
 return s;
}
export function decodeState(bytes){
 if(![STATE_BYTES,1908,LEGACY_STATE_BYTES].includes(bytes.length))throw Error('State size');
 const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),version=v.getUint16(4,true);
 if(v.getUint32(0,true)!==0x31434f5a||!([1,2,3].includes(version))||bytes.length!==({1:1872,2:1908,3:STATE_BYTES})[version])throw Error('State identity');
 let s={format:'zombie-core/1',bpm:v.getUint16(6,true),swing:bytes[8],selected:bytes[9],language:bytes[10],external:bytes[11],tracks:Array.from({length:6},(_,t)=>({p:Array.from({length:27},(_,i)=>v.getInt16(12+t*310+i*2,true)),steps:Array.from({length:64},(_,i)=>{const a=12+t*310+54+i*4;return {note:bytes[a],velocity:bytes[a+1],chance:bytes[a+2],gate:bytes[a+3]};})}))};
 let out=upgradeState(s);
 if(version>=2){out.octave=(bytes[1872]<<24)>>24;out.section=bytes[1873];out.subpage=bytes[1874];out.bank=bytes[1875];out.arp=Array.from({length:6},(_,i)=>({mode:bytes[1876+i],rate:bytes[1882+i],octaves:bytes[1888+i],hold:bytes[1894+i]}));out.presets=Array.from(bytes.slice(1900,1906));out.recording=bytes[1906];out.stepEdit=bytes[1907];}
 if(version===3){out.tracks.forEach((t,k)=>t.steps.forEach((st,i)=>{st.chord=Array.from(bytes.slice(1908+(k*64+i)*3,1911+(k*64+i)*3));st.duration=bytes[3060+k*64+i];}));[out.recMode,out.recQuant,out.metronome,out.fxAmount,out.fxRate]=bytes.slice(3444,3449);}
 return out;
}
export function validateState(input,schema){
 const s=upgradeState(input),integer=(n,lo,hi)=>Number.isInteger(n)&&n>=lo&&n<=hi;
 if(!s||s.format!=='zombie-core/3'||!integer(s.bpm,30,300)||!integer(s.swing,0,60)||!integer(s.selected,0,5)||!integer(s.language,0,1)||!integer(s.external,0,1)||!Array.isArray(s.tracks)||s.tracks.length!==6)throw Error('Invalid project');
 for(const t of s.tracks){if(Array.isArray(t.p)&&((EDITION==='groovebox'&&t.p[0]>7)||(EDITION==='sampler'&&t.p[0]!==8)))throw Error('Project belongs to another edition');if(!Array.isArray(t.p)||t.p.length!==27||!Array.isArray(t.steps)||t.steps.length!==64)throw Error('Invalid track');t.p.forEach((p,i)=>{if(!integer(p,schema[i].min,schema[i].max))throw Error('Invalid parameter '+i);});for(const st of t.steps)if(!integer(st.note,0,127)||!integer(st.velocity,0,127)||!integer(st.chance,0,127)||!integer(st.gate,1,127))throw Error('Invalid step');}
 if(!integer(s.octave,-3,3)||!integer(s.section,0,8)||!integer(s.subpage,0,s.section===6?5:3)||!integer(s.bank,0,3)||!integer(s.recording,0,1)||!integer(s.stepEdit,0,63)||!Array.isArray(s.presets)||s.presets.length!==6||s.presets.some(p=>!integer(p,0,63)&&p!==255)||!Array.isArray(s.arp)||s.arp.length!==6)throw Error('Invalid performance settings');
 for(const a of s.arp)if(!integer(a.mode,0,4)||!integer(a.rate,0,3)||!integer(a.octaves,1,3)||!integer(a.hold,0,1))throw Error('Invalid arpeggiator');
 for(const t of s.tracks)for(const st of t.steps)if(!integer(st.duration,0,63)||!Array.isArray(st.chord)||st.chord.length!==3||st.chord.some(n=>!integer(n,0,128)))throw Error('Invalid chord or duration');
 if(!integer(s.recMode,0,1)||!integer(s.recQuant,0,1)||!integer(s.metronome,0,1)||!integer(s.fxAmount,0,127)||!integer(s.fxRate,0,3))throw Error('Invalid live settings');
 return s;
}
export function encodeState(input,legacy=false){
 const s=upgradeState(input),b=new Uint8Array(legacy===true?LEGACY_STATE_BYTES:legacy===2?1908:STATE_BYTES),v=new DataView(b.buffer);
 v.setUint32(0,0x31434f5a,true);v.setUint16(4,legacy===true?1:legacy===2?2:3,true);v.setUint16(6,s.bpm,true);b.set([s.swing,s.selected,s.language,s.external],8);
 s.tracks.forEach((t,k)=>{t.p.forEach((p,i)=>v.setInt16(12+k*310+i*2,p,true));t.steps.forEach((st,i)=>b.set([st.note,st.velocity,st.chance,st.gate],12+k*310+54+i*4));});
 if(legacy!==true){b.set([s.octave&255,s.section,s.subpage,s.bank],1872);s.arp.forEach((a,i)=>{b[1876+i]=a.mode;b[1882+i]=a.rate;b[1888+i]=a.octaves;b[1894+i]=a.hold;});b.set(s.presets,1900);b.set([s.recording,s.stepEdit],1906);}
 if(!legacy){s.tracks.forEach((t,k)=>t.steps.forEach((st,i)=>{b.set(st.chord,1908+(k*64+i)*3);b[3060+k*64+i]=st.duration;}));b.set([s.recMode,s.recQuant,s.metronome,s.fxAmount,s.fxRate],3444);b[3449]=EDITION==='sampler'?2:1;}
 return b;
}

export function wavPCM(buffer){const v=new DataView(buffer),tag=o=>String.fromCharCode(...new Uint8Array(buffer,o,4));if(buffer.byteLength<44||tag(0)!=='RIFF'||tag(8)!=='WAVE')throw Error('WAV RIFF required');let format,channels,rate,bits,block,offset,length;for(let p=12;p+8<=v.byteLength;){const n=v.getUint32(p+4,true),id=tag(p);if(p+8+n>v.byteLength)throw Error('Truncated WAV');if(id==='fmt '&&n>=16){format=v.getUint16(p+8,true);channels=v.getUint16(p+10,true);rate=v.getUint32(p+12,true);block=v.getUint16(p+20,true);bits=v.getUint16(p+22,true);}if(id==='data'){offset=p+8;length=n;}p+=8+n+(n&1);}if(!offset||!length||![1,3].includes(format)||channels<1||channels>8||rate<4000||rate>192000||![8,16,24,32].includes(bits)||format===3&&bits!==32||block!==channels*bits/8)throw Error('Use PCM 8/16/24/32 or float32 WAV');const frames=Math.floor(length/block),out=new Float32Array(frames);for(let i=0;i<frames;i++){let sum=0;for(let c=0;c<channels;c++){const o=offset+i*block+c*bits/8;let x;if(format===3)x=v.getFloat32(o,true);else if(bits===8)x=(v.getUint8(o)-128)/128;else if(bits===16)x=v.getInt16(o,true)/32768;else if(bits===24){let n=v.getUint8(o)|v.getUint8(o+1)<<8|v.getUint8(o+2)<<16;if(n&0x800000)n-=0x1000000;x=n/8388608;}else x=v.getInt32(o,true)/2147483648;if(!Number.isFinite(x))throw Error('Non-finite WAV');sum+=x;}out[i]=Math.max(-1,Math.min(1,sum/channels));}return {pcm:out,rate};}
/* Windowed-sinc resampler: anti-aliasing precedes downsampling. No silent truncation. */
export function resamplePCM(source,from,to){const count=Math.floor(source.length*to/from);if(count<2||count>24448)throw Error('Sample exceeds 24448 frames; trim or choose a lower rate');const out=new Int16Array(count),ratio=from/to,cutoff=Math.min(1,to/from)*.92;for(let i=0;i<count;i++){const pos=i*ratio,base=Math.floor(pos);let sum=0,weight=0;for(let k=-24;k<=24;k++){const j=base+k;if(j<0||j>=source.length)continue;const d=pos-j,sd=d*cutoff,w=(Math.abs(sd)<1e-8?1:Math.sin(Math.PI*sd)/(Math.PI*sd))*(.5+.5*Math.cos(Math.PI*d/25));sum+=source[j]*w;weight+=w;}const x=weight?sum/weight:0;out[i]=Math.round(Math.max(-1,Math.min(1,x))*32767);}return out;}
export class Link {
 constructor(onDisconnect=()=>{}){this.pending=null;this.queue=Promise.resolve();this.seq=0;this.onDisconnect=onDisconnect;this.reads=Promise.resolve();this.lastRevision=-1;this.modern=false;}
 async connect(input,output){this.input=input;this.output=output;try{
  await input.open();await output.open();input.onmidimessage=e=>this.receive(e.data);
  this.version=new TextDecoder().decode(await this.command(1));
  const version=/^ZCORE\/1\/0\.([3-8])\.(0|[1-9]\d*)(?:\/(groovebox|sampler))?$/.exec(this.version);
  if(!version||!Number.isSafeInteger(Number(version[2])))throw Error('Requires Zombie Core protocol 1, firmware 0.3.x–0.8.x');
  const minor=Number(version[1]),patch=Number(version[2]);this.edition=version[3]||'core';if(minor===8&&patch<1)throw Error('Groovebox 0.8.0: use webapp/groovebox-080/ editor.');if(this.edition!==EDITION)throw Error('Open the '+this.edition+' editor for this firmware.');
  this.sampler=minor>=7&&this.edition!=='groovebox';this.daw=minor>=6;this.system=minor>=5;this.polish=minor>4||minor===4&&patch>=1;this.live=minor>=4;this.modern=minor>3||patch>=1;this.connected=true;this.lastRevision=-1;
  return await this.readState();
 }catch(e){this.close();throw e;}}
 readState(){const read=async()=>{
  if(!this.modern)return decodeState(await this.command(2));
  const bytes=new Uint8Array(this.live?STATE_BYTES:1908);let token=0,revision=0;
  for(let at=0;at<bytes.length;at+=192){const count=Math.min(192,bytes.length-at),data=await this.command(19,[...u16(at),count,...u32(token)]),v=new DataView(data.buffer,data.byteOffset,data.byteLength);
   if(data.length!==count+10||v.getUint16(8,true)!==at)throw Error('Incomplete state chunk');
   if(at===0){token=v.getUint32(0,true);revision=v.getUint32(4,true);if(!token)throw Error('Invalid snapshot token');}
   else if(token!==v.getUint32(0,true)||revision!==v.getUint32(4,true))throw Error('Mixed state snapshots');
   bytes.set(data.slice(10),at);
  }
  this.lastRevision=revision;return decodeState(bytes);
 };const result=this.reads.then(read);this.reads=result.catch(()=>{});return result;}
 async runtime(){if(!this.modern)return null;const d=await this.command(20);if(!((d.length===16&&d[0]===2)||(d.length===20&&d[0]===3)||(d.length===23&&d[0]===4)))throw Error('Invalid runtime status');return {playing:!!d[1],position:d[2],recording:d[3],octave:d[4]-3,section:d[5],subpage:d[6],bank:d[7],stepEdit:d[8],selected:d[9],preset:d[10],dirty:!!d[11],revision:new DataView(d.buffer,d.byteOffset).getUint32(12,true),layer:d.length>=20?d[16]-1:-1,effect:d.length>=20?d[17]:0,bpm:d.length>=20?(d[18]|d[19]<<8):null,undo:d.length===23?d[20]:0,redo:d.length===23?d[21]:0,solo:d.length===23?d[22]:0};}

 command(cmd,data=[],timeout=3000){const task=()=>new Promise((resolve,reject)=>{if(!this.output||this.output.state==='disconnected')return reject(Error('MIDI disconnected'));const seq=this.seq=this.seq%127+1;const timer=setTimeout(()=>{this.pending=null;reject(Error('MIDI timeout / command '+cmd));},timeout);this.pending={seq,cmd,resolve:b=>{clearTimeout(timer);this.pending=null;resolve(b);},reject:e=>{clearTimeout(timer);this.pending=null;reject(e);}};try{this.output.send([240,125,90,67,seq,cmd,...pack7(data),247]);}catch(e){this.pending.reject(e);}});const result=this.queue.then(task);this.queue=result.catch(()=>{});return result;}
 receive(b){const p=this.pending;if(!p||b.length<8||b[0]!==240||b[1]!==125||b[2]!==90||b[3]!==67||b[4]!==p.seq||b[5]!== (p.cmd|64)||b[b.length-1]!==247)return;const data=unpack7(b.slice(6,-1));if(!data.length)return;if(data[0])p.reject(Error('Device error '+data[0]+' / command '+p.cmd));else p.resolve(data.slice(1));}
 async upload(slot,pcm,rate,progress=()=>{}){if(slot<0||slot>3||pcm.length<2||pcm.length>24448)throw Error('Invalid sample');const data=new Uint8Array(pcm.length*2),v=new DataView(data.buffer);pcm.forEach((n,i)=>v.setInt16(i*2,n,true));await this.command(11,[slot,...u32(data.length),...u16(rate),...u32(crc32(data))]);try{for(let off=0;off<data.length;off+=192){await this.command(12,[...u16(off),...data.slice(off,off+192)]);progress(Math.min(1,(off+192)/data.length));}await this.command(13,[],30000);}catch(e){await this.command(14,[],1500).catch(()=>{});throw e;}}
 close(){this.connected=false;this.pending?.reject(Error('MIDI closed'));if(this.input){this.input.onmidimessage=null;Promise.resolve(this.input.close()).catch(()=>{});}if(this.output)Promise.resolve(this.output.close()).catch(()=>{});this.input=this.output=null;this.onDisconnect();}
}
/* Single polling loop: focus never disables updates; active edits defer state replacement
 * briefly, while transport/playhead status still arrives. Tested without a browser. */
export class DeviceSync {
 constructor(link,{onState,onRuntime,onError,onHealthy=()=>{},isBusy=()=>false,isEditing=()=>false}){Object.assign(this,{link,onState,onRuntime,onError,onHealthy,isBusy,isEditing});this.polling=false;}
 async poll(){if(!this.link.connected||this.polling||this.isBusy()||this.link.pending)return;this.polling=true;
  try{const runtime=await this.link.runtime();if(runtime)this.onRuntime(runtime);
   if(!this.isEditing()&&(!runtime||runtime.revision!==this.link.lastRevision)){
    const state=await this.link.readState();if(this.isEditing())this.link.lastRevision=-1;else this.onState(state);
   }this.onHealthy();
  }catch(e){this.onError(e);}finally{this.polling=false;}
 }
 start(){if(this.timer)return;this.timer=setInterval(()=>this.poll(),200);}
 stop(){clearInterval(this.timer);this.timer=null;}
}
/* Browser history mirrors the native four-level musical-pattern history. */
export class PatternHistory {
 constructor(){this.past=[];this.future=[];this.current=null;}
 capture(s){return s.tracks.map(t=>({steps:structuredClone(t.steps),length:t.p[23]}));}
 reset(s){this.past=[];this.future=[];this.current=this.capture(s);}
 observe(s,recording=false){const next=this.capture(s);if(this.current&&JSON.stringify(next)!==JSON.stringify(this.current)&&!recording){this.past.push(this.current);this.past=this.past.slice(-4);this.future=[];}this.current=next;}
 checkpoint(s){this.past.push(this.capture(s));this.past=this.past.slice(-4);this.future=[];this.current=this.capture(s);}
 restore(s,redo=false){const from=redo?this.future:this.past,to=redo?this.past:this.future;if(!from.length)return null;to.push(this.capture(s));if(to.length>4)to.shift();const next=structuredClone(s),p=from.pop();next.recording=0;p.forEach((t,i)=>{next.tracks[i].steps=structuredClone(t.steps);next.tracks[i].p[23]=t.length;});this.current=this.capture(next);return next;}
}
export class AudioEngine {
 constructor(module,onMessage){this.module=module;this.onMessage=onMessage;this.requests=new Map();this.requestId=0;this.stateEpoch=0;}
 async start(){if(!this.context){this.context=new AudioContext({sampleRate:44100,latencyHint:'interactive'});if(this.context.sampleRate!==44100){await this.context.close();this.context=null;throw Error('44.1 kHz audio required');}await this.context.audioWorklet.addModule(new URL('./worklet.js',import.meta.url));this.node=new AudioWorkletNode(this.context,'zombie-core',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2],processorOptions:{module:this.module}});this.node.port.onmessage=e=>{const reply=e.data;if(reply.requestId){const r=this.requests.get(reply.requestId);if(r){clearTimeout(r.timer);this.requests.delete(reply.requestId);r.resolve(reply);}return;}this.onMessage(reply);};this.gain=this.context.createGain();this.gain.gain.value=.7;this.node.connect(this.gain).connect(this.context.destination);}await this.context.resume();}
 send(type,data={}){if(type==='state')this.stateEpoch++;this.node?.port.postMessage({type,...data,epoch:this.stateEpoch});}
 command(type,data={}){if(!this.node)return Promise.resolve(null);return new Promise((resolve,reject)=>{const requestId=++this.requestId,timer=setTimeout(()=>{this.requests.delete(requestId);reject(Error('Audio command timeout'));},2000);this.requests.set(requestId,{resolve,reject,timer});this.node.port.postMessage({type,...data,requestId});});}
 async close(){for(const r of this.requests.values()){clearTimeout(r.timer);r.reject(Error('Audio closed'));}this.requests.clear();this.send('panic');this.node?.disconnect();await this.context?.close();this.node=this.context=null;}
}

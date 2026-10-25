/* SPDX-License-Identifier: MIT */
export class CoreDSP {
 constructor(module){this.w=new WebAssembly.Instance(module,{}).exports;this.w.z_init();this.ptr=this.w.z_buffer();this.audio=new Float32Array(this.w.memory.buffer,this.ptr,512);this.trim=100;this.limiter=1;this.at=256;this.lastRevision=this.w.z_revision();}
 state(bytes,forcePattern=false){
  const target=new Uint8Array(this.w.memory.buffer,this.w.z_snapshot(),this.w.z_state_size());
  const recChanged=!this.previous||this.previous[1906]!==bytes[1906];if(recChanged&&!bytes[1906])this.w.z_record(0);
  // Merge only edits: a filter movement must not erase notes captured by the audio thread.
  for(let i=0;i<target.length;i++){
   const t=Math.floor((i-12)/310),offset=(i-12)%310;
   const patternByte=i>=1908&&i<3444||t>=0&&t<6&&(offset>=54||offset===46||offset===47);
   if(i!==1906&&(forcePattern&&patternByte||!this.previous||this.previous[i]!==bytes[i]))target[i]=bytes[i];
  }
  if(recChanged&&bytes[1906])this.w.z_record(1);
  this.previous=bytes.slice();this.w.z_apply_state();this.w.z_external(0);
 }
 sample(slot,pcm,rate){if(this.w.h_models)throw Error("Use the drum suite sample engine; legacy PCM slots are not available in Groovebox");const v=new DataView(this.w.memory.buffer,this.w.z_transfer(),pcm.length*2);pcm.forEach((p,i)=>v.setInt16(i*2,p,true));this.w.z_preview_sample(slot,pcm.length,rate);}
 render(left,right){for(let i=0;i<left.length;i++){if(this.at>=256){this.w.z_render(this.ptr,256);this.w.z_post(this.trim,this.limiter,this.ptr,256);this.at=0;}left[i]=this.audio[this.at*2];right[i]=this.audio[this.at*2+1];this.at++;}}
}
if(typeof registerProcessor==='function'){
 class Processor extends AudioWorkletProcessor {
  constructor(options){
   super();this.dsp=new CoreDSP(options.processorOptions.module);this.counter=0;this.epoch=0;
   this.port.onmessage=({data:d})=>this.message(d);
  }
  syncDrumTransport(){const w=this.dsp.w,wanted=w.h_requested_playing();if(wanted!==w.z_playing())w.z_transport(wanted);const v=new DataView(w.memory.buffer,w.z_snapshot(),w.z_state_size());if(w.h_bpm()!==v.getUint16(6,true))w.z_tempo(w.h_bpm(),v.getUint8(8));}
  project(){const w=this.dsp.w,n=w.h_pack(w.z_transfer());return new Uint8Array(w.memory.buffer,w.z_transfer(),n).slice();}
  flash(){const w=this.dsp.w;return w.h_flash_dirty()?new Uint8Array(w.memory.buffer,w.h_flash(),0x100000).slice():undefined;}
  message(d){
   try{
    const w=this.dsp.w;let drum,hFlash;
    if(d.type==='hControl'){
     if(w.h_remote(d.kind,d.index,d.value))throw Error('Drum control rejected');this.syncDrumTransport();
     if(d.requestId&&d.kind===0&&d.index===7&&d.value){drum=this.project();hFlash=this.flash();}
    }else if(d.type==='hFocus'){this.hFocus=!!d.on;if(!d.on){w.h_remote_release();w.h_ui_tick();}}
    else if(d.type==='hRestoreFlash'){if(d.bytes.length!==0x100000)throw Error('Invalid drum flash image');new Uint8Array(w.memory.buffer,w.h_flash(),d.bytes.length).set(d.bytes);w.h_init();}
    else if(d.type==='hImport'){
     if(d.bytes.length!==3288)throw Error('Invalid drum project');new Uint8Array(w.memory.buffer,w.z_transfer(),3288).set(d.bytes);
     if(w.h_unpack(w.z_transfer(),3288))throw Error('Invalid drum project');w.z_transport(0);w.z_tempo(w.h_bpm(),new DataView(w.memory.buffer,w.z_snapshot(),w.z_state_size()).getUint8(8));
    }else if(d.type==='hSnapshot'){drum=this.project();hFlash=this.flash();}
    else if(d.type==='state'){this.epoch=d.epoch;this.dsp.state(d.bytes,d.forcePattern);}
    else if(d.type==='system'){this.dsp.trim=d.trim;this.dsp.limiter=d.limiter;}
    else if(d.type==='note')w.z_key(d.track,d.note,d.velocity,d.on);
    else if(d.type==='play')w.z_transport(d.on);
    else if(d.type==='panic')w.z_transport(0);
    else if(d.type==='live')w.z_live(d.effect);
    else if(d.type==='sampler'){if(d.bytes.length!==396)throw Error('Invalid sampler size');new Uint8Array(w.memory.buffer,w.z_sampler_snapshot(),d.bytes.length).set(d.bytes);if(w.z_sampler_apply())throw Error('Invalid sampler config');}
    else if(d.type==='sample')this.dsp.sample(d.slot,d.pcm,d.rate);
    else if(d.type==='solo')w.z_solo(d.track);
    else if(d.type==='flush')w.z_record(0);
    if(d.requestId)this.port.postMessage({requestId:d.requestId,epoch:this.epoch,drum,hFlash,state:new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size()).slice()});
   }catch(e){this.port.postMessage({requestId:d.requestId,error:e.message});}
  }
  process(_inputs,outputs){
   this.dsp.render(outputs[0][0],outputs[0][1]);
   if(++this.counter%16===0){
    const w=this.dsp.w;let peak=0;for(const c of outputs[0])for(const x of c)peak=Math.max(peak,Math.abs(x));
    const message={epoch:this.epoch,position:w.z_position(),peak,voices:w.z_voice_count()};
    if(this.dsp.lastRevision!==w.z_revision()){this.dsp.lastRevision=w.z_revision();message.recorded=new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size()).slice();}
    w.h_service();message.hFlash=this.flash();
    if(this.hFocus){w.h_ui_tick();message.hFrame=new Uint8Array(w.memory.buffer,w.h_frame(),240*240*2).slice();message.hProject=this.project();message.hPlaying=w.z_playing();message.hBpm=w.h_bpm();}
    this.port.postMessage(message);
   }return true;
  }
 }
 registerProcessor('zombie-core',Processor);
}

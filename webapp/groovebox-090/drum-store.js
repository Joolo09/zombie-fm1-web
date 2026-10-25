/* SPDX-License-Identifier: GPL-3.0-only */
// Keep the main WASM flash mirror current when an audio worker saves a kit.
// Writes are serialized so a slow older transaction cannot replace a newer save.
export class DrumStore {
 constructor(local,{read,write,onError=()=>{}}){
  this.local=local;this.write=write;this.onError=onError;this.pending=Promise.resolve();this.loaded=false;
  this.ready=Promise.resolve().then(read).then(bytes=>{if(bytes==null)return;this.copy(bytes);local.h_init();}).catch(onError).finally(()=>{this.loaded=true;});
 }
 copy(bytes){if(!(bytes instanceof Uint8Array)||bytes.length!==0x100000)throw Error('Invalid drum flash image');new Uint8Array(this.local.memory.buffer,this.local.h_flash(),bytes.length).set(bytes);}
 async accept(bytes){await this.ready;this.copy(bytes);const snapshot=bytes.slice(),operation=this.pending.then(()=>this.write(snapshot));this.pending=operation.catch(this.onError);await operation;}
 async flush(){await this.ready;await this.pending;}
}

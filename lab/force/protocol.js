/* SPDX-License-Identifier: MIT — original Lab host report reader. No role/write commands. */
export function pack7(bytes){const out=[];for(let at=0;at<bytes.length;at+=7){const chunk=bytes.slice(at,at+7);out.push(chunk.reduce((mask,n,i)=>mask|((n>>7&1)<<i),0),...chunk.map(n=>n&127));}return out;}
export function unpack7(bytes){const out=[];for(let at=0;at<bytes.length;){const mask=bytes[at++];if(mask>127)throw Error('Invalid MIDI byte');for(let i=0;i<7&&at<bytes.length;i++){const b=bytes[at++];if(b>127)throw Error('Invalid MIDI byte');out.push(b|((mask>>i&1)<<7));}}return Uint8Array.from(out);}
export const results=['Не запускался','Дескриптор получен','Отменён','Нет ответа SIE','Host-режим не подтверждён','Нет подключённого устройства','Таймаут','Устройство ответило STALL','Ошибка шины','Неверный размер пакета','Неверный дескриптор'];
export function decode(bytes){
 if(bytes.length!==128)throw Error('Неверный размер отчёта');
 const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),f=Array.from({length:24},(_,i)=>v.getUint32(i*4,true));
 if(f[0]!==0x48505242||f[1]!==2||f[2]>1||f[4]>8||f[5]>10||f[12]>18||f[16]>1||f[17]>1||f[19]!==0||f[20]>8||f[21]>10||f[22]>255||(f[23]&255)>1||(f[23]>>>8)>3)throw Error('Неизвестный формат host-отчёта');
 const d=Array.from(bytes.slice(96,114));
 const valid=d[0]===18&&d[1]===1&&[8,16,32,64].includes(d[7])&&(d[8]||d[9])&&!(d[8]===255&&d[9]===255)&&d[17]>0;
 if(f[5]===1&&(!valid||f[4]!==8||!f[3]||!f[11]||f[12]!==18||!f[16]||!f[17]||f[13]!== (d[8]|d[9]<<8)||f[14]!== (d[10]|d[11]<<8)||f[15]!==d[7]))throw Error('Противоречивый результат USB-host');
 return {schema:'zombie-lab-force-report/1',firmware:'ZLAB/1/0.3.0',simulated:!!f[2],attempts:f[3],state:f[4],result:f[5],resultText:results[f[5]],elapsedMs:f[6],registers:{devctl:f[7],power:f[8],csr0:f[9],dpDm:f[10]},packets:f[11],receivedBytes:f[12],vid:f[13],pid:f[14],ep0MaxPacket:f[15],hostModeObserved:!!f[16],resetAttempted:!!f[17],startBlocked:f[18],lastStage:f[20],forced:true,signalingSpeed:(f[23]&1)?'low':'full',sweepState:f[23]>>>8,firstFullSpeed:{result:f[21],resultText:results[f[21]],csr0:f[22]},testsRun:false,descriptor:d,descriptorReadOnHardware:!f[2]&&f[5]===1,vbusOutputVoltage:'not-measured',storageReadWriteTested:false,fullEnumerationTested:false,limitations:['Address-zero device descriptor only','No SET_ADDRESS or SET_CONFIGURATION','No HID reports or storage commands','PHY VBUS flag is not an external voltage measurement','No response does not prove hardware lacks USB host']};
}
export class HostLink {
 constructor(){this.seq=0;this.queue=Promise.resolve();this.connected=false;}
 async connect(input,output){this.input=input;this.output=output;try{
  await input.open();await output.open();input.onmidimessage=e=>this.receive(e.data);
  const id=new TextDecoder().decode(await this.command(1));if(id!=='ZLAB/1/0.3.0')throw Error('Для этого теста нужна Zombie Force 0.3.0');
  const report=decode(await this.command(5));this.connected=true;return report;
 }catch(e){this.close();throw e;}}
 command(cmd){if(![1,5].includes(cmd))return Promise.reject(Error('Разрешено только чтение host-отчёта'));
  const task=()=>new Promise((resolve,reject)=>{if(!this.output)return reject(Error('Нет MIDI-соединения'));
   const seq=this.seq=this.seq%127+1;const timer=setTimeout(()=>{this.pending=null;reject(Error('Нет ответа. Установи Zombie Force 0.3.0 и выбери её MIDI-порты.'));},2000);
   this.pending={seq,cmd,resolve:b=>{clearTimeout(timer);this.pending=null;resolve(b);},reject:e=>{clearTimeout(timer);this.pending=null;reject(e);}};
   try{this.output.send([240,125,90,76,seq,cmd,247]);}catch(e){this.pending.reject(e);}
  });const done=this.queue.then(task);this.queue=done.catch(()=>{});return done;
 }
 receive(bytes){const p=this.pending;if(!p||bytes.length<8||bytes[0]!==240||bytes[1]!==125||bytes[2]!==90||bytes[3]!==76||bytes[4]!==p.seq||bytes[5]!== (p.cmd|64)||bytes.at(-1)!==247)return;
  try{const d=unpack7(bytes.slice(6,-1));if(!d.length)throw Error('Пустой ответ');if(d[0])throw Error('Lab: отказ '+d[0]);p.resolve(d.slice(1));}catch(e){p.reject(e);}
 }
 close(){this.connected=false;this.pending?.reject(Error('Соединение закрыто'));if(this.input){this.input.onmidimessage=null;Promise.resolve(this.input.close()).catch(()=>{});}if(this.output)Promise.resolve(this.output.close()).catch(()=>{});this.input=this.output=null;}
}

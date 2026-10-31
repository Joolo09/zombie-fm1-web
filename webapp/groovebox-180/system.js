/* SPDX-License-Identifier: MIT — independent device settings, separate from project JSON. */
export const systemDefaults=()=>({dual:0,trim:100,limiter:1,glow:1,velocity:0,start:0,midiMode:0,transport:1,countin:1,recClick:1,scale:0,root:0,clockOut:0,saver:1,saverStyle:0,name:'My FM-1',bio:'',status:0,peak:0,jobs:0,timeouts:0});
export function profileBytes(value,field){const bytes=new TextEncoder().encode(value);if(bytes.length>=(field?64:32)||!/^[\x20-\x7e\u0400-\u045f]*$/u.test(value))throw Error('Русский / латиница: имя ≤31 байт, подпись ≤63 байт. / ASCII or Cyrillic only.');return bytes;}
/* the settings the device keeps in ids 0-12 of command 29, in id order */
export const SYSTEM_KEYS=['dual','trim','limiter','glow','velocity','start','midiMode','transport','countin','recClick','scale','root','clockOut','saver','saverStyle'];
export const SYSTEM_MAX=[1,127,1,2,2,2,2,1,2,1,9,11,1,1,3];
export function decodeSystem(bytes){
 const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),version=bytes.length>=6?v.getUint16(4,true):0;
 if(!((bytes.length===118&&version===1)||(bytes.length===122&&version===2)))throw Error('System reply size/version');
 if(v.getUint32(0,true)!==0x3153595a)throw Error('System identity');const s=systemDefaults();
 ['dual','trim','limiter','glow','velocity','start'].forEach((key,i)=>{const n=bytes[6+i];if(n>[1,127,1,2,2,2][i])throw Error('System range');s[key]=n;});
 for(const [key,start,size] of [['name',12,32],['bio',44,64]]){const a=bytes.slice(start,start+size),end=a.indexOf(0);if(end<0)throw Error('Unterminated profile');s[key]=new TextDecoder('utf-8',{fatal:true}).decode(a.slice(0,end));profileBytes(s[key],key==='bio');}
 const off=version===2?112:108;if(version===2){s.midiMode=bytes[108];s.transport=bytes[109];if(s.midiMode>2||s.transport>1)throw Error('MIDI range');
  /* 0.16+: recording options (count-in bars, click while recording, MIDI clock out) and 0.17+: the scale lock; older files hold zeros = the defaults */
  const rec=bytes[110]&127,lock=bytes[111];s.countin=(rec&3)===0?1:(rec&3)===1?0:2;s.recClick=rec&4?0:1;s.clockOut=rec&8?1:0;s.saver=rec&16?0:1;s.saverStyle=rec>>5&3;s.scale=(lock&15)<10?lock&15:0;s.root=(lock>>4)<12?lock>>4:0;}else{s.midiMode=0;s.transport=0;}
 s.status=bytes[off];if(s.status>5||bytes[off+1]>100)throw Error('System status');s.peak=bytes[off+1];s.jobs=v.getUint32(off+2,true);s.timeouts=v.getUint32(off+6,true);return s;
}
export function initSystem({link,audio,perform,language}){
 let settings=systemDefaults(),lastConnected=false,polling=false,pending=0,lastRead=0,error='';
 const $=id=>document.getElementById(id),key=new URLSearchParams(location.search).has('scratch')?'zcore-system-scratch':'zcore-system-v1';
 try{const saved=JSON.parse(localStorage.getItem(key));if(saved){for(const [i,k] of SYSTEM_KEYS.entries())if(Number.isInteger(saved[k])&&saved[k]>=0&&saved[k]<=SYSTEM_MAX[i])settings[k]=saved[k];for(const k of ['name','bio'])if(typeof saved[k]==='string'){profileBytes(saved[k],k==='bio');settings[k]=saved[k];}}}catch{}settings.dual=0;
 const text=(ru,en)=>language()==='ru'?ru:en;
 const preview=()=>audio.send('system',{trim:settings.trim,limiter:settings.limiter});
 const remember=()=>{try{localStorage.setItem(key,JSON.stringify({...settings,dual:0}));}catch{}preview();};
 function render(){const active=document.activeElement,available=link.connected&&link.system;
  for(const el of document.querySelectorAll('[data-sys-ru]'))el.textContent=text(el.dataset.sysRu,el.dataset.sysEn);
  [...SYSTEM_KEYS,'name','bio'].forEach(k=>{const e=$('sys-'+k);if(e&&e!==active)e.value=String(settings[k]);});
  $('sys-dual').disabled=!available;for(const k of ['midiMode','transport'])$('sys-'+k).disabled=!(link.connected&&link.daw);$('sys-save').disabled=!available||!!pending;$('sys-trim-value').textContent=settings.trim+'%';
  const states=['CPU0','CPU1 • '+text('РАБОТАЕТ','RUNNING'),text('СБОЙ ЗАПУСКА → CPU0','START FAILED → CPU0'),text('ОШИБКА ТЕСТА → CPU0','SELF-TEST FAILED → CPU0'),text('ТАЙМАУТ → CPU0','TIMEOUT → CPU0'),'SIMULATED'];
  $('sys-core-status').textContent=available?states[settings.status]:text('FM-1 не подключён','FM-1 not connected');$('sys-core-status').classList.toggle('warn',available&&settings.status>=2&&settings.status<=4);
  $('sys-diagnostics').textContent=available?`${text('Пик','Peak')}: ${settings.peak}% · ${text('Блоки CPU1','CPU1 blocks')}: ${settings.jobs} · ${text('Таймауты','Timeouts')}: ${settings.timeouts}`:text('Предпрослушивание: уровень выхода и лимитер действуют на звук ПК.','Preview: output level and limiter affect PC audio.');
  $('sys-message').textContent=error||(!link.connected?text('Локальный черновик. Подключение прочитает настройки FM-1.','Local draft. Connecting reads settings from FM-1.'):!link.system?text('Требуется прошивка 0.5.0.','Firmware 0.5.0 required.'):text('Изменения действуют сразу. «Сохранить систему» записывает их во флеш-память.','Edits apply immediately. Save system writes them to flash.'));
 }
 async function refresh(force=false){if(polling||pending||!link.connected||!link.system||!force&&(performance.now()-lastRead<1200||$('settings').hidden||document.activeElement?.closest('#system-settings')))return;polling=true;try{const next=decodeSystem(await link.command(28));if(link.connected&&link.system&&!pending){settings=next;error='';preview();lastRead=performance.now();render();}}catch(e){error=e.message;render();}finally{polling=false;}}
 for(const [id,k] of SYSTEM_KEYS.entries())if($('sys-'+k))$('sys-'+k).onchange=()=>{const value=Number($('sys-'+k).value);perform(async()=>{pending++;try{if(link.connected){if(!link.system)throw Error('Firmware 0.5.0 required');if(id>=6&&!link.daw)throw Error('Firmware 0.6.0 required');await link.command(29,[id,value]);}settings[k]=value;error='';remember();}catch(e){error=e.message;throw e;}finally{pending--;render();}if(link.connected)await refresh(true);});};
 for(const [id,k] of ['name','bio'].entries())$('sys-'+k).onchange=()=>{const value=$('sys-'+k).value;perform(async()=>{pending++;try{const bytes=profileBytes(value,id);if(link.connected){if(!link.system)throw Error('Firmware 0.5.0 required');await link.command(30,[id,...bytes]);}settings[k]=value;error='';remember();}catch(e){error=e.message;render();throw e;}finally{pending--;render();}});};
 for(const k of ['name','bio'])$('sys-'+k).oninput=()=>{settings[k]=$('sys-'+k).value;try{profileBytes(settings[k],k==='bio');remember();}catch{}};
 $('sys-save').onclick=()=>perform(async()=>{for(const [id,k] of ['name','bio'].entries()){const value=$('sys-'+k).value;await link.command(30,[id,...profileBytes(value,id)]);settings[k]=value;}await link.command(31);error=text('Система и профиль сохранены. CPU1 включается вручную после запуска.','System and profile saved. Enable CPU1 manually after startup.');render();});
 setInterval(()=>{const connected=!!link.connected;if(connected!==lastConnected){lastConnected=connected;lastRead=0;if(connected)refresh(true);else{settings.dual=0;settings.status=0;}render();}else refresh();},500);
 remember();render();return {render,preview,refresh};
}

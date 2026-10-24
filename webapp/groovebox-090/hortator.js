/* SPDX-License-Identifier: GPL-3.0-only — controls for the ported Hortator UI. */
export function initDrumSuite({local,audio,link,getState,getLang,status,stop,onTransport}){
 const host=document.createElement('section');host.id='hortator-suite';host.className='card';host.hidden=true;
 host.innerHTML=`<div class="row"><h2>06 / HORTATOR</h2><span class="grow"></span><button id="h-save">SAVE</button><button id="h-export">↓ DRUM PROJECT</button><label class="file-button">↑ DRUM PROJECT<input id="h-import" type="file" accept=".zhkit"></label></div><p class="muted" id="h-guide"></p><div class="h-layout"><canvas id="h-screen" width="240" height="240" aria-label="Hortator FM-1 screen"></canvas><div><div id="h-buttons"></div><div id="h-knobs"></div></div></div><div id="h-keys" aria-label="FM-1 keys"></div><p class="muted">Hortator by <a href="https://deadactive.github.io/hortator/" target="_blank" rel="noopener">DEADACTIVE</a> / Hügelton · GPL-3.0 · 45 models · 151 presets · 128 user presets</p>`;
 document.getElementById('tracks').after(host);
 const $=id=>host.querySelector('#'+id),ctx=$('h-screen').getContext('2d'),image=ctx.createImageData(240,240),held=new Set();
 let focus=false,lastNode=null,wasConnected=false,pullAt=0,pulling=false,lastEdit=0;
 const active=()=>getState().selected===5;
 function paint(bytes){const p=new Uint16Array(bytes.buffer,bytes.byteOffset,240*240);for(let i=0;i<p.length;i++){const c=p[i],at=i*4;image.data[at]=(c>>11)*255/31;image.data[at+1]=(c>>5&63)*255/63;image.data[at+2]=(c&31)*255/31;image.data[at+3]=255;}ctx.putImageData(image,0,0);}
 const snapshot=()=>{const n=local.h_pack(local.z_transfer());return new Uint8Array(local.memory.buffer,local.z_transfer(),n).slice();};
 function loadLocal(bytes){if(bytes.length!==3288)throw Error('Invalid drum project size');new Uint8Array(local.memory.buffer,local.z_transfer(),3288).set(bytes);if(local.h_unpack(local.z_transfer(),3288))throw Error('Invalid drum project');}
 function localDraw(){local.h_time(Math.floor(performance.now()));local.h_ui_tick();paint(new Uint8Array(local.memory.buffer,local.h_frame(),240*240*2));}
 async function control(kind,index,value){
  lastEdit=performance.now();
  if(link.connected&&!link.hortator)throw Error(getLang()==='ru'?'Для драм-модуля нужна прошивка 0.9.0.':'Drum suite requires firmware 0.9.0.');
  if(audio.node)audio.send('hControl',{kind,index,value});
  else {local.h_time(Math.floor(performance.now()));if(local.h_remote(kind,index,value))throw Error('Drum control rejected');const on=local.h_requested_playing();if(on!==local.z_playing())local.z_transport(on);const state=new DataView(local.memory.buffer,local.z_snapshot(),local.z_state_size());if(local.h_bpm()!==state.getUint16(6,true))local.z_tempo(local.h_bpm(),state.getUint8(8));onTransport(on,local.h_bpm());localDraw();}
  if(link.connected)await link.command(42,[kind,index,value&255,value>>8&255]);
 }
 const run=fn=>Promise.resolve().then(fn).catch(e=>status(e.message,true));
 function keyButton(parent,label,kind,index,cls=''){
  const b=document.createElement('button');b.textContent=label;b.className=cls;b.setAttribute('aria-label',label);
  const up=()=>{if(!held.delete(b))return;b.classList.remove('held');run(()=>control(kind,index,0));};
  b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);held.add(b);b.classList.add('held');run(()=>control(kind,index,1));};
  b.onpointerup=b.onpointercancel=b.onlostpointercapture=up;parent.append(b);return b;
 }
 ['FX','SEL / KEYS','ENV','LFO','EDIT','GLO','HOME','SAVE','ARP / GRIDS','SEQ','PLAY','REC','OCT−','OCT+'].forEach((label,i)=>keyButton($('h-buttons'),label,0,i));
 ['SELECT / PAGE','ALGO / DRUM','PRESET','K1','K2','K3','K4'].forEach((label,i)=>{
  const box=document.createElement('div');box.className='h-knob';const name=document.createElement('span');name.textContent=label;box.append(name);
  for(const d of [-1,1]){const b=document.createElement('button');b.textContent=d<0?'−':'+';b.setAttribute('aria-label',`${label} ${d<0?'decrease':'increase'}`);b.onclick=()=>run(()=>control(2,i,d));box.append(b);}
  box.onwheel=e=>{e.preventDefault();run(()=>control(2,i,e.deltaY<0?1:-1));};$('h-knobs').append(box);
 });
 const whites=new Set([0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26]);
 for(let k=0;k<27;k++)keyButton($('h-keys'),whites.has(k)?String([...whites].indexOf(k)+1):'♯',1,k,whites.has(k)?'h-white':'h-black');
 async function save(){await control(0,7,1);await control(0,7,0);if(!audio.node)await saveFlash(new Uint8Array(local.memory.buffer,local.h_flash(),0x100000).slice());}
 $('h-save').onclick=()=>run(save);
 $('h-export').onclick=()=>run(async()=>{const data=link.connected&&link.hortator?await link.command(43):audio.node?(await audio.command('hSnapshot')).drum:snapshot();if(!data)throw Error('No drum project');const url=URL.createObjectURL(new Blob([data],{type:'application/octet-stream'})),a=document.createElement('a');a.href=url;a.download='Zombie-Hortator.zhkit';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);});
 $('h-import').onchange=()=>run(async()=>{const f=$('h-import').files[0];if(!f)return;if(link.connected&&!link.hortator)throw Error('Requires firmware 0.9.0');const bytes=new Uint8Array(await f.arrayBuffer());loadLocal(bytes);await stop();audio.send('hImport',{bytes});
  if(link.connected){if(!link.hortator)throw Error('Requires firmware 0.9.0');let crc=0xffffffff;for(const b of bytes){crc^=b;for(let i=0;i<8;i++)crc=crc>>>1^((crc&1)?0xedb88320:0);}crc=(~crc)>>>0;await link.command(47,[crc&255,crc>>>8&255,crc>>>16&255,crc>>>24&255]);for(let at=0;at<bytes.length;at+=192)await link.command(17,[at&255,at>>>8,...bytes.slice(at,at+192)]);await link.command(48);}
  localDraw();status(getLang()==='ru'?'Драм-проект загружен. Нажми SAVE для сохранения.':'Drum project loaded. Press SAVE to keep it.');
 });
 async function pull(){if(pulling||!link.connected||!link.hortator||performance.now()-lastEdit<500)return;pulling=true;try{loadLocal(await link.command(43));if(!audio.node)localDraw();}catch(e){status(e.message,true);}finally{pulling=false;}}
 function release(){for(const b of [...held])b.onpointerup();for(const code of [...keyboardHeld]){const k=keyCodes.indexOf(code),b={Space:10,Enter:11,Escape:6,ArrowLeft:12,ArrowRight:13}[code];run(()=>control(k>=0?1:0,k>=0?k:b,0));}keyboardHeld.clear();}
 window.addEventListener('blur',release);
 const keyCodes=['KeyZ','KeyS','KeyX','KeyD','KeyC','KeyF','KeyV','KeyB','KeyH','KeyN','KeyJ','KeyM','Comma','KeyL','Period','Semicolon','Slash','KeyQ','Digit2','KeyW','Digit3','KeyE','KeyR','Digit5','KeyT','Digit6','KeyY'];
 const keyboardHeld=new Set();
 for(const type of ['keydown','keyup'])document.addEventListener(type,e=>{if(!active()||e.ctrlKey||e.metaKey||e.altKey||['INPUT','TEXTAREA','SELECT'].includes(e.target.tagName))return;const k=keyCodes.indexOf(e.code),b={Space:10,Enter:11,Escape:6,ArrowLeft:12,ArrowRight:13}[e.code];if(k<0&&b==null)return;e.preventDefault();e.stopImmediatePropagation();if(type==='keydown'){if(keyboardHeld.has(e.code))return;keyboardHeld.add(e.code);}else keyboardHeld.delete(e.code);run(()=>control(k>=0?1:0,k>=0?k:b,type==='keydown'?1:0));},true);
 document.getElementById('record').addEventListener('click',e=>{if(!active())return;e.preventDefault();e.stopImmediatePropagation();run(async()=>{await control(0,11,1);await control(0,11,0);});},true);
 document.querySelectorAll('[data-tab]').forEach(b=>b.addEventListener('click',e=>{const id={sound:4,sequence:9,live:0}[b.dataset.tab];if(!active()||id==null)return;e.preventDefault();e.stopImmediatePropagation();run(async()=>{await control(0,id,1);await control(0,id,0);});},true));
 async function database(){return new Promise((resolve,reject)=>{const r=indexedDB.open('zombie-hortator-090',1);r.onupgradeneeded=()=>r.result.createObjectStore('state');r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);});}
 async function saveFlash(bytes){try{const db=await database();await new Promise((resolve,reject)=>{const t=db.transaction('state','readwrite');t.objectStore('state').put(bytes,'flash');t.oncomplete=resolve;t.onerror=()=>reject(t.error);});db.close();}catch(e){status(e.message,true);}}
 (async()=>{try{const db=await database(),data=await new Promise((resolve,reject)=>{const r=db.transaction('state').objectStore('state').get('flash');r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);});db.close();if(data?.length===0x100000){new Uint8Array(local.memory.buffer,local.h_flash(),data.length).set(data);local.h_init();localDraw();}}catch(e){status(e.message,true);}})();
 function render(){const on=active();host.hidden=!on;document.body.dataset.hortator=String(on);if(on)$('h-guide').textContent=getLang()==='ru'?'Внутри дорожки 6 — восемь барабанных партий. ALGO здесь выбирает барабан, SELECT листает страницы, EDIT выбирает модель. Удерживай FX/GLO/SEQ и играй клавишами. На физическом FM-1 ALGO сохраняет выбор внешней дорожки, SEL + PRESET выбирает барабан.':'Track 6 contains eight drums. ALGO selects the drum, SELECT changes pages, EDIT selects the model. Hold FX/GLO/SEQ while playing keys. On hardware ALGO keeps selecting the outer track; SEL + PRESET selects the drum.';
  if(focus!==on||lastNode!==audio.node){focus=on;lastNode=audio.node;audio.send('hFocus',{on});if(!on)release();}
  if(on&&!audio.node)localDraw();if(link.connected&&!wasConnected)pull();wasConnected=link.connected;
 }
 const timer=setInterval(()=>{if(active()){if(!audio.node){for(let i=0;i<8;i++)local.z_render(local.z_buffer(),256);localDraw();if(local.h_flash_dirty())saveFlash(new Uint8Array(local.memory.buffer,local.h_flash(),0x100000).slice());}if(link.connected&&performance.now()-pullAt>1200){pullAt=performance.now();pull();}}},50);
 return {render,message(d){if(d.hFrame&&active())paint(d.hFrame);if(d.hProject){try{loadLocal(d.hProject);}catch(e){status(e.message,true);}}if(d.hFlash)saveFlash(d.hFlash);if(d.hPlaying!=null)onTransport(d.hPlaying,d.hBpm);},audioChanged(){if(!audio.node&&active()){local.z_tempo(local.h_bpm(),getState().swing);local.z_transport(0);onTransport(0,local.h_bpm());}if(audio.node){audio.send('hRestoreFlash',{bytes:new Uint8Array(local.memory.buffer,local.h_flash(),0x100000).slice()});audio.send('hImport',{bytes:snapshot()});}render();},dispose(){clearInterval(timer);release();}};
}

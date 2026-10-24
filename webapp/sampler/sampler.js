/* SPDX-License-Identifier: MIT — original pad editor and independent config codec. */
import {SAMPLES,u16,u32,crc32} from './core.js';
export const SAMPLER_BYTES=396;
export const PAD_FIELDS=['start','end','source','tune','gain','pan','mode','reverse','choke','quality'];
export function decodeSampler(bytes){
 if(bytes.length!==SAMPLER_BYTES)throw Error('Sampler size');
 const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength);
 if(v.getUint32(0,true)!==0x3150535a||v.getUint16(4,true)!==1||bytes.slice(9,12).some(Boolean))throw Error('Sampler identity');
 const kit={format:'zombie-sampler/1',tracks:bytes[6],bank:bytes[7],selected:bytes[8],pads:[]};
 for(let i=0;i<32;i++){const at=12+i*12;kit.pads.push({start:v.getUint16(at,true),end:v.getUint16(at+2,true),source:bytes[at+4],tune:v.getInt8(at+5),gain:bytes[at+6],pan:v.getInt8(at+7),mode:bytes[at+8],reverse:bytes[at+9],choke:bytes[at+10],quality:bytes[at+11]});}
 return validateSampler(kit);
}
export function validateSampler(kit){
 const n=(x,min,max)=>Number.isInteger(x)&&x>=min&&x<=max;
 if(!kit||kit.format!=='zombie-sampler/1'||!n(kit.tracks,0,63)||!n(kit.bank,0,1)||!n(kit.selected,0,31)||(kit.selected>>4)!==kit.bank||!Array.isArray(kit.pads)||kit.pads.length!==32)throw Error('Invalid sampler bank');
 for(const p of kit.pads)if(!p||!n(p.start,0,65534)||!n(p.end,p.start+1,65535)||!n(p.source,0,13)||!n(p.tune,-24,24)||!n(p.gain,0,127)||!n(p.pan,-64,63)||!n(p.mode,0,2)||!n(p.reverse,0,1)||!n(p.choke,0,8)||!n(p.quality,0,2))throw Error('Invalid sampler pad');
 return kit;
}
export function encodeSampler(kit){
 validateSampler(kit);const b=new Uint8Array(SAMPLER_BYTES),v=new DataView(b.buffer);
 v.setUint32(0,0x3150535a,true);v.setUint16(4,1,true);b.set([kit.tracks,kit.bank,kit.selected],6);
 kit.pads.forEach((p,i)=>{const at=12+i*12;v.setUint16(at,p.start,true);v.setUint16(at+2,p.end,true);b.set([p.source,p.tune&255,p.gain,p.pan&255,p.mode,p.reverse,p.choke,p.quality],at+4);});return b;
}
export async function uploadSampler(link,kit){
 const bytes=encodeSampler(kit);await link.command(40,u32(crc32(bytes)));
 try{for(let at=0;at<bytes.length;at+=192)await link.command(17,[...u16(at),...bytes.slice(at,at+192)]);await link.command(41);}
 catch(error){await link.command(14).catch(()=>{});throw error;}
}
export function initSampler({local,audio,link,state,sync,setParam,perform,run,keyDown,keyUp,releaseKeys,language,status,download,draftKey}){
 const $=id=>document.getElementById(id),ru=()=>language()==='ru',text=(r,e)=>ru()?r:e;
 let kit=decodeSampler(new Uint8Array(local.memory.buffer,local.z_sampler_snapshot(),SAMPLER_BYTES).slice()),gridKey='',lastRead=0,polling=false;
 const waves=new Map();
 function apply(){const bytes=encodeSampler(kit);new Uint8Array(local.memory.buffer,local.z_sampler_snapshot(),SAMPLER_BYTES).set(bytes);if(local.z_sampler_apply())throw Error('Invalid sampler');audio.send('sampler',{bytes});try{localStorage.setItem(draftKey,JSON.stringify(kit));}catch{}}
 function available(){return !link.connected||link.sampler;}
 function track(){return state().selected;}
 function enabled(){return !!(kit.tracks>>track()&1)&&state().tracks[track()].p[0]===8;}
 async function deviceSelect(){if(link.connected){if(!link.sampler)throw Error(text('Для сэмплера обнови FM-1 до 0.7.0.','Update FM-1 to 0.7.0 for the sampler.'));await link.command(36,[track(),kit.tracks>>track()&1,kit.selected]);}}
 function select(index){kit.selected=index;kit.bank=index>>4;apply();perform(deviceSelect);render();}
 function selected(){return kit.pads[kit.selected];}
 function draw(){const canvas=$('pad-wave'),ctx=canvas.getContext('2d'),pad=selected(),w=canvas.width,h=canvas.height;ctx.clearRect(0,0,w,h);
  const source=waves.get(pad.source),count=96;ctx.fillStyle=getComputedStyle(document.body).getPropertyValue('--accent');
  for(let i=0;i<count;i++){const value=source?source.peaks[i]/128:local.z_sample_peak(pad.source,i,count)/32768;ctx.fillRect(i*w/count,h/2-value*(h/2-10),Math.max(1,w/count-3),Math.max(1,value*(h-20)));}
  ctx.fillStyle='#0009';ctx.fillRect(0,0,w*pad.start/65535,h);ctx.fillRect(w*pad.end/65535,0,w*(1-pad.end/65535),h);
  ctx.fillStyle='#ffffff';ctx.fillRect(w*pad.start/65535,0,2,h);ctx.fillRect(w*pad.end/65535-2,0,2,h);
  const frames=source?.frames??local.z_sample_frames(pad.source),rate=source?.rate??local.z_sample_rate(pad.source);
  $('pad-duration').textContent=frames?`${SAMPLES[pad.source]} · ${(frames*(pad.end-pad.start)/65535/rate).toFixed(3)} s · ${rate} Hz`:text('Слот пуст: загрузи WAV ниже.','Empty slot: upload a WAV below.');
 }
 function field(name,r,e,min,max,options){const wrap=document.createElement('label'),span=document.createElement('span'),input=document.createElement(options?'select':'input');span.textContent=text(r,e);
  if(options)options.forEach((label,i)=>input.add(new Option(label,String(i))));else{input.type='number';input.min=min;input.max=max;input.step=['start','end'].includes(name)?.01:1;}
  input.value=['start','end'].includes(name)?(selected()[name]*100/65535).toFixed(2):selected()[name];input.id='pad-'+name;input.onchange=()=>perform(async()=>{const index=kit.selected,value=['start','end'].includes(name)?Math.round(Number(input.value)*65535/100):Number(input.value),next=structuredClone(kit);next.pads[index][name]=value;validateSampler(next);if(link.connected){if(!link.sampler)throw Error('FM-1 0.7.0 required');await link.command(35,[index,PAD_FIELDS.indexOf(name),...u16(value)]);}kit=next;apply();render();});wrap.append(span,input);return wrap;
 }
 function render(){
  $('sampler-title').textContent=text('Пэды. Нарезка. Свой грув.','Pads. Chops. Your groove.');
  $('sampler-help').textContent=text('32 пэда A/B · 8 общих голосов · MIDI 36–67. На FM-1: HOME / EDIT. PRESET меняет настройки. Чёрные клавиши: A, B, reverse, режим, нарезка 4/8/16. Для звука FM-1 выбери Standalone; для прослушивания здесь включи звук ПК.','32 pads A/B · 8 shared voices · MIDI 36–67. FM-1: HOME / EDIT. PRESET changes controls. Black keys: A, B, reverse, mode, chop 4/8/16. Select Standalone for local FM-1 audio; enable PC audio to preview here.');
  $('sampler-enable').textContent=enabled()?text('Пэды включены · дорожка ','Pads enabled · track ')+(track()+1):text('Включить пэды на дорожке ','Enable pads on track ')+(track()+1);
  $('sampler-enable').setAttribute('aria-pressed',String(enabled()));$('sampler-enable').disabled=true;
  $('sampler-save').textContent=text('Сохранить пэды в FM-1','Save pads in FM-1');$('sampler-save').disabled=!link.connected||!link.sampler;
  $('sampler-export').textContent=text('Скачать раскладку','Export pad map');$('sampler-import-label').textContent=text('Открыть раскладку','Import pad map');
  $('sampler-map-note').textContent=text('Раскладка хранит настройки 32 пэдов. Аудио USR хранится отдельно: сохраняй исходные WAV. Нарезка делит выбранный фрагмент поровну и заменяет первые 4/8/16 пэдов текущего банка.','Pad maps store all 32 pad settings. USR audio is separate: retain your original WAVs. Chop divides the selected region equally and replaces the first 4/8/16 pads of the current bank.');
  for(const bank of [0,1])$('pad-bank-'+bank).setAttribute('aria-pressed',String(kit.bank===bank));
  const nextGrid=kit.bank+':'+language();if(gridKey!==nextGrid){gridKey=nextGrid;$('sampler-pads').replaceChildren(...Array.from({length:16},(_,offset)=>{const index=kit.bank*16+offset,b=document.createElement('button');b.dataset.pad=index;
    b.onpointerdown=event=>{if(!available()||!enabled())return;event.preventDefault();b.setPointerCapture(event.pointerId);select(index);keyDown('sample'+event.pointerId,36+index,b);};
    b.onpointerup=b.onpointercancel=b.onlostpointercapture=event=>keyUp('sample'+event.pointerId);
    b.onclick=()=>{if(kit.selected!==index)select(index);};
    b.onkeydown=event=>{if(event.code==='Space'&&!event.repeat&&enabled()){event.preventDefault();select(index);keyDown('sample-key'+index,36+index,b);}};
    b.onkeyup=event=>{if(event.code==='Space'){event.preventDefault();keyUp('sample-key'+index);}};return b;}));}
  for(const b of $('sampler-pads').children){const index=Number(b.dataset.pad),p=kit.pads[index];const code=document.createElement('strong'),name=document.createElement('span');code.className='pad-code';name.className='pad-source';code.textContent=`${index<16?'A':'B'}${String(index%16+1).padStart(2,'0')}`;name.textContent=SAMPLES[p.source];b.title=SAMPLES[p.source];b.replaceChildren(code,name);b.setAttribute('aria-pressed',String(index===kit.selected));b.setAttribute('aria-label',`${index<16?'A':'B'}${index%16+1}: ${SAMPLES[p.source]}`);}
  $('pad-name').textContent=`${kit.selected<16?'A':'B'}${String(kit.selected%16+1).padStart(2,'0')} / MIDI ${36+kit.selected}`;
  const modes=ru()?['ONE SHOT · целиком','GATE · пока держишь','LOOP · пока держишь']:['ONE SHOT','GATE · held','LOOP · held'];
  $('pad-fields').replaceChildren(field('source','Источник','Source',0,13,SAMPLES),field('mode','Воспроизведение','Playback',0,2,modes),field('start','Начало, %','Start, %',0,99.99),field('end','Конец, %','End, %',.01,100),field('tune','Высота · полутона','Tune · semitones',-24,24),field('gain','Уровень','Level',0,127),field('pan','Панорама','Pan',-64,63),field('reverse','Направление','Direction',0,1,['FORWARD','REVERSE']),field('choke','Группа заглушения · 0 выкл.','Choke group · 0 off',0,8),field('quality','Характер','Character',0,2,['16 BIT','12 BIT','8 BIT']));
  draw();
 }
 $('sampler-enable').disabled=true;
 for(const bank of [0,1])$('pad-bank-'+bank).onclick=()=>{releaseKeys();select(bank*16+kit.selected%16);};
 for(const slices of [4,8,16])$('pad-chop-'+slices).onclick=()=>perform(async()=>{releaseKeys();if(link.connected){if(!link.sampler)throw Error('FM-1 0.7.0 required');await link.command(37,[kit.selected,slices]);kit=decodeSampler(await link.command(34));}else{if(local.z_sampler_chop(kit.selected,slices))throw Error('Trim too short to chop');kit=decodeSampler(new Uint8Array(local.memory.buffer,local.z_sampler_snapshot(),SAMPLER_BYTES).slice());}apply();render();status(text(`Банк нарезан: ${slices} частей.`,`Bank chopped into ${slices} slices.`));});
 $('sampler-save').onclick=()=>run(async()=>{await link.command(9);status(text('Проект и пэды сохранены.','Project and pads saved.'));});
 $('sampler-export').onclick=()=>download('zombie-pad-map.json',kit);
 $('sampler-import').onchange=()=>run(async()=>{const file=$('sampler-import').files[0];if(!file)return;if(file.size>32768)throw Error('Pad map > 32 KiB');const next=validateSampler(JSON.parse(await file.text()));releaseKeys();if(link.connected){if(!link.sampler)throw Error('FM-1 0.7.0 required');await uploadSampler(link,next);}kit=structuredClone(next);apply();render();status(text('Раскладка открыта. Аудио USR не менялось.','Pad map loaded. USR audio was unchanged.'));});
 async function read(){if(!link.connected||!link.sampler)return;kit=decodeSampler(await link.command(34));apply();await wave();render();}
 async function wave(){const source=selected().source;if(link.connected&&link.sampler){const data=await link.command(39,[source]);if(data.length!==105||data[0]!==source)throw Error('Invalid sample waveform');const view=new DataView(data.buffer,data.byteOffset,data.byteLength);waves.set(source,{frames:view.getUint32(1,true),rate:view.getUint32(5,true),peaks:data.slice(9)});}}
 async function poll(){if(polling||Date.now()-lastRead<800)return;polling=true;lastRead=Date.now();try{await read();}catch(error){status(error.message,true);}finally{polling=false;}}
 function sample(slot,pcm,rate){const bytes=new DataView(local.memory.buffer,local.z_transfer(),pcm.length*2);pcm.forEach((value,i)=>bytes.setInt16(i*2,value,true));local.z_preview_sample(slot,pcm.length,rate);waves.delete(slot+6);draw();}
 try{const stored=JSON.parse(localStorage.getItem(draftKey));if(stored)kit=validateSampler(stored);}catch{}
 apply();
 async function assign(source){if(link.connected&&link.sampler)await link.command(35,[kit.selected,2,...u16(source)]);kit.pads[kit.selected].source=source;kit.pads[kit.selected].start=0;kit.pads[kit.selected].end=65535;if(link.connected&&link.sampler){await link.command(35,[kit.selected,0,0,0]);await link.command(35,[kit.selected,1,255,255]);}apply();render();}
 return {render,read,poll,sample,assign,audition:t=>audio.send('sampler',{bytes:encodeSampler({...kit,tracks:kit.tracks&~(1<<t)})}),resend:()=>audio.send('sampler',{bytes:encodeSampler(kit)})};
}

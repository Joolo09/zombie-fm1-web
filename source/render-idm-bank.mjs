/* Original patches and demonstrations, rendered by the shipped C engine. */
import assert from 'node:assert/strict';
import {readFileSync,writeFileSync,mkdirSync} from 'node:fs';
import {createHash} from 'node:crypto';
import {decodeState} from '../webapp/groovebox-170/core.js';
import {wasmDrums} from '../webapp/groovebox-170/drum-state.js';
const mod=new WebAssembly.Module(readFileSync('webapp/groovebox-170/zombie-core.wasm'));
const bank=JSON.parse(readFileSync('source/editions/idm-presets.json'));
const fresh=()=>{const w=new WebAssembly.Instance(mod,{}).exports;w.z_init();return w;};
const render=(w,frames)=>{const a=new Float32Array(frames*2);for(let at=0;at<frames;at+=256){const n=Math.min(256,frames-at);w.z_render(w.z_buffer(),n);a.set(new Float32Array(w.memory.buffer,w.z_buffer(),n*2),at*2);}return a;};
function wav(path,a){const b=Buffer.alloc(44+a.length*2);b.write('RIFF');b.writeUInt32LE(b.length-8,4);b.write('WAVEfmt ',8);b.writeUInt32LE(16,16);b.writeUInt16LE(1,20);b.writeUInt16LE(2,22);b.writeUInt32LE(44100,24);b.writeUInt32LE(176400,28);b.writeUInt16LE(4,32);b.writeUInt16LE(16,34);b.write('data',36);b.writeUInt32LE(a.length*2,40);a.forEach((x,i)=>b.writeInt16LE(Math.round(Math.max(-1,Math.min(1,x))*32767),44+i*2));writeFileSync(path,b);}
const dir='emulator/sounds';mkdirSync(dir,{recursive:true});mkdirSync('source/output/idm-patches',{recursive:true});
const report=[];
for(let i=0;i<32;i++){
 const w=fresh(),id=64+i,note=i<8?36:i<16?60:i<24?60:72;w.z_preset(0,id);w.z_set(0,15,88);
 const held=i>=16&&i<24?1.1:.32,all=new Float32Array(44100*3*2);w.z_key(0,note,100,1);const first=render(w,Math.round(held*44100));all.set(first);w.z_key(0,note,0,0);all.set(render(w,44100*3-first.length/2),first.length);
 let energy=0,peak=0,clipped=0,diff=0;for(let j=0;j<all.length;j++){const x=all[j];assert(Number.isFinite(x)&&Math.abs(x)<=.981);energy+=x*x;peak=Math.max(peak,Math.abs(x));clipped+=Math.abs(x)>.975;if(j>1)diff+=(x-all[j-2])**2;}
 const rms=Math.sqrt(energy/all.length);assert(rms>.001,'inaudible '+bank[i][0]);assert(clipped/all.length<.001,'clipped '+bank[i][0]);
 const slug=bank[i][0].toLowerCase().replaceAll(' ','-');wav(dir+'/'+slug+'.wav',all);
 const p=Array.from({length:27},(_,k)=>w.z_get(0,k));writeFileSync('source/output/idm-patches/'+slug+'.json',JSON.stringify({format:'zombie-core-patch/1',name:bank[i][0],license:'MIT',p},null,2)+'\n');
 report.push({slot:id+1,name:bank[i][0],category:['BASS','FM / MALLETS','ATMOSPHERE','GLITCH / TEXTURES'][Math.floor(i/8)],note,peak:+peak.toFixed(4),rms_db:+(20*Math.log10(rms)).toFixed(2),brightness:+Math.sqrt(diff/energy).toFixed(4),file:slug+'.wav',hash:createHash('sha256').update(new Uint8Array(all.buffer)).digest('hex')});
}
assert.equal(new Set(report.map(r=>r.hash)).size,32,'presets must render distinct audio');
const w=fresh();[64,72,80,88,93].forEach((p,t)=>{w.z_preset(t,p);w.z_set(t,15,[76,57,36,49,45][t]);w.z_set(t,23,32);});w.z_tempo(142,9);w.z_set(5,15,86);
for(const [at,n] of [[0,36],[3,36],[7,43],[10,39],[14,36],[17,46],[20,39],[23,43],[28,34],[30,36]])w.z_step_set(0,at,n,95,127,65);
for(const [at,n] of [[2,65],[6,72],[11,67],[15,60],[18,77],[22,65],[27,72],[31,67]])w.z_step_set(1,at,n,79,115,43);
w.z_step_set(2,0,53,76,127,110);w.z_step_set(2,16,56,70,127,110);const state=new Uint8Array(w.memory.buffer,w.z_snapshot(),w.z_state_size());for(const at of [0,16]){state[1908+(2*64+at)*3]=61;state[1909+(2*64+at)*3]=65;state[3060+2*64+at]=7;}
for(const [at,n] of [[5,79],[13,84],[21,77],[29,89]])w.z_step_set(3,at,n,67,105,30);
for(const [at,n] of [[8,60],[24,63]])w.z_step_set(4,at,n,68,127,75);
for(let d=0;d<16;d++){w.z_dm_param(d,0,[100,79,45,39,62,56,56,50,46,33,36,40,37,35,40,44][d]);w.z_dm_param(d,1,[48,33,19,48,27,23,42,37,31,41,38,26,29,18,24,22][d]);}
w.z_dm_param(0,2,-4);w.z_dm_param(1,2,5);w.z_dm_param(5,2,-7);w.z_dm_param(2,5,15);w.z_dm_param(13,5,7);
for(let at=0;at<32;at++){if([0,6,10,16,19,26,30].includes(at))w.z_dm_step(0,at,105,127,1);if(at%8===4)w.z_dm_step(1,at,99,127,1);if(at%2===0)w.z_dm_step(2,at,65,115,at%8===6?3:1);if(at%8===7)w.z_dm_step(5,at,71,104,2);if(at%8===3)w.z_dm_step(13,at%7,52,108,1);}
const project=decodeState(state.slice());project.drums=wasmDrums(w);writeFileSync('source/output/idm-patches/idm-demo-project.json',JSON.stringify(project,null,2)+'\n');
w.z_transport(1);const demo=render(w,44100*14);w.z_transport(0);wav(dir+'/idm-demo.wav',demo);
writeFileSync(dir+'/analysis.json',JSON.stringify({version:'0.17.0',engine_sha256:createHash('sha256').update(readFileSync('webapp/groovebox-170/zombie-core.wasm')).digest('hex'),presets:report},null,2)+'\n');
const cards=report.map(r=>`<article><small>${r.category} · ${r.slot}</small><h2>${r.name}</h2><audio controls preload="none" src="${r.file}"></audio></article>`).join('');
writeFileSync(dir+'/index.html',`<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Zombie IDM · 32 пресета</title><style>body{margin:30px auto;max-width:1100px;padding:0 20px;background:#101a13;color:#e2edcf;font:16px system-ui}a{color:#b8df78}h1{font:700 44px monospace}section{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:16px}article{border:1px solid #485f3b;padding:18px;background:#1b291d}h2{font:18px monospace}small{color:#afc58e}audio{width:100%}.demo{max-width:600px;margin:24px 0}</style><a href="../?v=0137">← Эмулятор</a><h1>ZOMBIE / IDM</h1><p>32 оригинальных звука, слоты 65–96. Старые 64 сохранены. Примеры получены из C-движка прошивки, без внешних эффектов и нормализации.</p><p><a href="../../downloads/zombie-idm-bank-0.12.0.zip">Скачать банк JSON + демо-проект</a> · <a href="../../?release=FM-1_63001307">Установить 0.17.0 beta</a></p><div class="demo"><h2>Собранный IDM-грув · 142 BPM</h2><audio controls preload="none" src="idm-demo.wav"></audio></div><section>${cards}</section></html>`);
console.log('PASS 32 original, distinct, audible IDM patches with finite audio and no sustained clipping; rendered preview bank and demo project');

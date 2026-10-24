"""Patch-release packaging. Keep every 0.8.0 firmware/download and Sampler editor."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,shutil
F=Path(__file__).resolve().parent;R=F.parent;E=F/'editions';O=F/'output/editions-0.8.1';S=F/'github-pages'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def build():
 old=S/'webapp/groovebox-080';app=S/'webapp/groovebox'
 if not old.exists():shutil.copytree(app,old)
 protected=O/'protected-release-files.json'
 if not protected.exists():
  paths=[*list((S/'downloads').rglob('*')),*list((S/'webapp/sampler').rglob('*')),*list(old.rglob('*'))]
  protected.write_text(json.dumps({str(p.relative_to(S)):sha(p) for p in paths if p.is_file()},indent=2))
 shutil.copytree(old,app,dirs_exist_ok=True)
 for name in ['piano.js','drums.js']:shutil.copy2(E/name,app/name)
 shutil.copy2(O/'groovebox.wasm',app/'zombie-core.wasm')
 js=(app/'app.js').read_text(encoding='utf-8').replace('0.8.0','0.8.1')
 js=js.replace("'./core.js'","'./core.js?v=081'").replace("'./piano.js'","'./piano.js?v=081'").replace("fetch('./zombie-core.wasm')","fetch('./zombie-core.wasm?v=081')")
 js=js.replace("'zcore-groovebox-scratch'","'zcore-groovebox-081-scratch'").replace("'zcore-groovebox-v3'","'zcore-groovebox-081-v3'")
 js="import {DRUMS,drumName} from './drums.js';\n"+js
 js=js.replace("small.textContent=MODELS[t.p[0]]", "small.textContent=(i===5?'DRUM KIT':MODELS[t.p[0]])")
 js=js.replace("+' / '+MODELS[t.p[0]]", "+' / '+(i===5?'DRUM KIT':MODELS[t.p[0]])")
 js=js.replace("$('instrument-name').textContent=MODELS[state.tracks[state.selected].p[0]]", "$('instrument-name').textContent=state.selected===5?'DRUM KIT / 16':MODELS[state.tracks[state.selected].p[0]]")
 js=js.replace('function render(){','function render(){renderDrumKit();')
 js=js.replace('async function setParam(t,id,value){','async function setParam(t,id,value){if(t===5&&id===0)value=7;')
 js=js.replace("const p=Number($('preset').value);", "if(state.selected===5)return;const p=Number($('preset').value);")
 js=js.replace('function keyDown(id,n,button){','function keyDown(id,n,button){if(state.selected===5&&n>=octave&&n<octave+27)n=DRUMS[(n-octave)%16][0];')
 # Dedicated pad audition bypasses the musical keyboard's offset map.
 js+='''
function renderDrumKit(){
 const kit=state.selected===5;document.body.dataset.drumKit=String(kit);
 for(const id of ['model','preset','preset-search','load-preset'])$(id).disabled=kit;
 $('drum-kit').hidden=!kit;if(!kit)return;
 $('drum-kit-title').textContent=lang==='ru'?'Дорожка 6 · Драм-кит':'Track 6 · Drum kit';
 $('drum-kit-help').textContent=lang==='ru'?'Пэды — прослушать. В сетке: клик ставит удар, Shift добавляет к шагу, Alt удаляет один удар. До 4 звуков на шаге. GM MIDI, канал 6 по умолчанию.':'Pads audition. Grid: click places a hit, Shift adds to a step, Alt removes one hit. Up to 4 sounds per step. GM MIDI, channel 6 by default.';
 $('drum-kit-pads').replaceChildren(...DRUMS.map(([note,en,ru],i)=>{const b=document.createElement('button');b.textContent=(lang==='ru'?ru:en)+' · '+note;b.setAttribute('aria-label',(lang==='ru'?ru:en)+' MIDI '+note);b.onpointerdown=e=>{if(busy)return;e.preventDefault();b.setPointerCapture(e.pointerId);const id='drum'+e.pointerId;held.set(id,{n:note,t:5,button:b});b.classList.add('held');audio.send('note',{track:5,note,velocity:110,on:1});if(link.connected)perform(()=>link.command(4,[5,note,110,1]));};b.onpointerup=b.onpointercancel=e=>keyUp('drum'+e.pointerId);b.onlostpointercapture=e=>keyUp('drum'+e.pointerId);return b;}));
}
'''
 # Correct current edition copy in both languages, leaving old release editor untouched.
 js=js.replace('Шесть дорожек, 64 синтезаторных пресета и пианоролл.', 'Пять синтезаторных дорожек, 64 пресета и драм-кит на шестой. Пианоролл и барабанная сетка.')
 js=js.replace('Six tracks, 64 synthesis presets and a piano roll.', 'Five synth tracks, 64 presets and a drum kit on track six. Piano roll and drum grid.')
 (app/'app.js').write_text(js,encoding='utf-8')
 html=(app/'index.html').read_text(encoding='utf-8').replace('0.8.0','0.8.1')
 html=html.replace('<h3>Piano Roll</h3>', '<section id="drum-kit" class="card" hidden><h3 id="drum-kit-title"></h3><p id="drum-kit-help" class="muted"></p><div id="drum-kit-pads"></div></section><h3>Piano Roll / Drum Grid</h3>')
 html=html.replace('src="app.js"','src="app.js?v=081"')
 html=html.replace('</head>','<style>#drum-kit-pads{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px}#drum-kit-pads button{min-height:48px;border-color:#8263a9}#drum-kit-pads .held{background:#be90f9;color:#080e18;box-shadow:0 0 20px #be90f970}body[data-drum-kit=true] #osc-controls,body[data-drum-kit=true] #env-controls,body[data-drum-kit=true] .keyboard{display:none}@media(max-width:600px){#drum-kit-pads{grid-template-columns:repeat(2,minmax(0,1fr))}}</style></head>')
 (app/'index.html').write_text(html,encoding='utf-8')
 core=(app/'core.js').read_text(encoding='utf-8')
 core=core.replace("this.edition=version[3]||'core';", "this.edition=version[3]||'core';if(minor===8&&patch<1)throw Error('Groovebox 0.8.0: use webapp/groovebox-080/ editor.');")
 (app/'core.js').write_text(core,encoding='utf-8')
 downloads=S/'downloads';fw=O/'zombie-groovebox-0.8.1.fwsc';shutil.copy2(fw,downloads/fw.name)
 releases=json.loads((S/'releases.json').read_text(encoding='utf-8'))
 releases['releases']=[r for r in releases['releases'] if r['product']!='FM-1_63000801']
 for r in releases['releases']:
  if r['product']=='FM-1_63000800':r['editor']='webapp/groovebox-080/'
 releases['releases'].insert(0,dict(product='FM-1_63000801',version='0.8.1',label={'ru':'Groovebox 0.8.1 · 5 синтов + драм-кит','en':'Groovebox 0.8.1 · 5 synths + drum kit'},notes={'ru':'Дорожка 6: 16 синтезированных ударных, барабанная сетка и GM MIDI. Полный сброс чипа после установки; результат нужно проверить на FM-1. Сохрани старый проект: шестая дорожка теперь играет драм-китом.','en':'Track 6: 16 synthesized drums, drum grid and GM MIDI. Full chip reset after install; physical FM-1 confirmation pending. Back up your old project: track six now plays the drum kit.'},bytes=fw.stat().st_size,sha256=sha(fw),file='downloads/'+fw.name,editor='webapp/groovebox/'))
 releases['current']='FM-1_63000801';(S/'releases.json').write_text(json.dumps(releases,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 for name in ['installer.js']:
  p=S/name;s=p.read_text(encoding='utf-8').replace('Готово. FM-1 запущен с выбранной прошивкой.','Прошивка установлена, версия подтверждена. Проверь звук и клавиши.').replace('Done. FM-1 is running the selected firmware.','Firmware installed and version confirmed. Check sound and keys.');p.write_text(s,encoding='utf-8')
 p=S/'index.html';s=p.read_text(encoding='utf-8').replace('installer.js?v=1.1','installer.js?v=1.2').replace('GROOVEBOX / SAMPLER 0.8.0','GROOVEBOX 0.8.1 / SAMPLER 0.8.0').replace('6 tracks · 64 synth presets · 4-note piano roll','5 synths + drum kit · 64 presets · piano roll / drum grid').replace('downloads/editions-source-0.8.0.zip','downloads/groovebox-source-0.8.1.zip');p.write_text(s,encoding='utf-8')
 shutil.copy2(O/'groovebox/drum-grid-ru.png',S/'previews/groovebox-piano.png')
 shutil.copy2(O/'groovebox-native.gif',S/'previews/groovebox-motion.gif')
 shutil.copy2(O/'groovebox/groovebox-demo.wav',S/'previews/groovebox-demo.wav')
 source=S/'source/editions/shared'
 for p in (E/'shared').iterdir():
  if p.is_file():shutil.copy2(p,source/p.name)
 for name in ['piano.js','drums.js']:shutil.copy2(E/name,S/'source/editions'/name)
 for name in ['build-editions.py','build-groovebox-081.py','prepare-groovebox081-web.py']:shutil.copy2(F/name,S/'source'/name)
 for name in ['edition.h','fm1.toml']:shutil.copy2(E/'groovebox'/name,S/'source/config/groovebox'/name)
 shutil.copy2(F/'EDITIONS-081.md',downloads/'Groovebox-0.8.1-RU.md')
 (S/'source/platform-081/bsp/loader').mkdir(parents=True,exist_ok=True)
 shutil.copy2(F/'platform-081/bsp/loader/loader.c',S/'source/platform-081/bsp/loader/loader.c')
 with ZipFile(downloads/'groovebox-source-0.8.1.zip','w',ZIP_DEFLATED) as z:
  for directory,prefix in [(E/'shared','editions/shared'),(E/'assets','editions/assets'),(E/'profiles','editions/profiles'),(F/'platform-081','platform-081')]:
   for p in directory.rglob('*'):
    if p.is_file() and not any(x in p.parts for x in ['.git','build','__pycache__']):z.write(p,'source/'+prefix+'/'+p.relative_to(directory).as_posix())
  for name in ['piano.js','drums.js','presets.json']:z.write(E/name,'source/editions/'+name)
  for name in ['build-editions.py','build-groovebox-081.py','prepare-groovebox081-web.py','EDITIONS-081.md']:z.write(F/name,'source/'+name)
  for name in ['test_groovebox081.mjs','test_native_drum081.py','test_loader_restart.py','test_drum_grid081.mjs','test_editions.mjs']:z.write(F/'tests'/name,'source/tests/'+name)
  z.write(F/'zombie-core/LICENSE','LICENSE')
 with ZipFile(O/'Zombie-Groovebox-0.8.1.zip','w',ZIP_DEFLATED) as z:
  for p in [fw,downloads/'groovebox-source-0.8.1.zip',downloads/'Zombie-FM1-DAW-Profiles-0.8.0.zip',F/'EDITIONS-081.md']:z.write(p,p.name)
 expected=json.loads(protected.read_text());assert all(sha(S/p)==h for p,h in expected.items()),'Old release or sampler changed'
 (S/'README.md').write_text((S/'README.md').read_text(encoding='utf-8')+'\n\nGroovebox 0.8.1: five synth tracks + synthesized GM drum kit on track six.\nNative/web drum grid, 16 sounds, hat choke. Successful OTA now uses a full\nP33 chip reset. Hardware confirmation pending. Details: downloads/Groovebox-0.8.1-RU.md.\nSource: downloads/groovebox-source-0.8.1.zip. Sampler remains 0.8.0.\n',encoding='utf-8') if 'Groovebox 0.8.1:' not in (S/'README.md').read_text(encoding='utf-8') else None
 for dest in [R/'output/zombie-core-site',F/'output/site']:shutil.copytree(S,dest,dirs_exist_ok=True,ignore=shutil.ignore_patterns('source','tests','package.json','README.md','__pycache__'))
 print(f'Groovebox 0.8.1 packaged; {len(expected)} old download/Sampler/editor files unchanged. No hardware flashed.')
if __name__=='__main__':build()

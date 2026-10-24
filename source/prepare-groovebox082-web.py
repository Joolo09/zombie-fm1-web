"""Package 0.8.2 controls, restored splash and matching DAW profiles; no flashing."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,shutil
F=Path(__file__).resolve().parent;R=F.parent;E=F/'editions';O=F/'output/editions-0.8.2';S=F/'github-pages'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def build():
 app=S/'webapp/groovebox';old=S/'webapp/groovebox-081'
 if not old.exists():shutil.copytree(app,old)
 protected=O/'protected-release-files.json'
 if not protected.exists():
  paths=[*list((S/'downloads').rglob('*')),*list((S/'webapp/sampler').rglob('*')),*list(old.rglob('*')),*list((S/'webapp/groovebox-080').rglob('*'))]
  protected.write_text(json.dumps({str(p.relative_to(S)):sha(p) for p in paths if p.is_file()},indent=2))
 shutil.copytree(old,app,dirs_exist_ok=True);shutil.copy2(O/'groovebox.wasm',app/'zombie-core.wasm')
 p=app/'app.js';js=p.read_text(encoding='utf-8').replace('0.8.1','0.8.2').replace('v=081','v=082').replace('groovebox-081-','groovebox-082-')
 # Dedicated physical-control guide; current browser controls retain their behavior.
 extra_ru="hardwareControls:'На FM-1: SELECT всегда BPM · ALGO: дорожка · кнопка повторно: страница. Удержание + HOME: LOCK. SEQ + OCT−/+: банк. EDIT + OCT−/+: Undo/Redo. На дорожке 6 K2 выбирает ударный; SEQ + клавиша меняет только его.'"
 extra_en="hardwareControls:'FM-1: SELECT always BPM · ALGO: track · tap the same button: page. Hold + HOME: LOCK. SEQ + OCT−/+: bank. EDIT + OCT−/+: Undo/Redo. Track 6 K2 selects a drum; SEQ + key toggles only that drum.'"
 js=js.replace('const dictionaries={ru:RU,en:EN};',"Object.assign(RU,{"+extra_ru+"});Object.assign(EN,{"+extra_en+"});\nconst dictionaries={ru:RU,en:EN};")
 js=js.replace("holdHelp:'Удерживай HOME:","holdHelp:'SELECT всегда BPM. Удержание + HOME фиксирует слой; HOME выходит. Удерживай HOME:")
 js=js.replace("holdHelp:'Hold HOME:","holdHelp:'SELECT always BPM. Hold + HOME locks a layer; HOME exits. Hold HOME:")
 p.write_text(js,encoding='utf-8')
 p=app/'index.html';html=p.read_text(encoding='utf-8').replace('v=081','v=082').replace('0.8.1','0.8.2').replace('<h3>Piano Roll / Drum Grid</h3>','<p class="muted" data-i="hardwareControls"></p><h3>Piano Roll / Drum Grid</h3>');p.write_text(html,encoding='utf-8')
 p=app/'core.js';core=p.read_text(encoding='utf-8').replace("if(minor===8&&patch<1)throw Error('Groovebox 0.8.0: use webapp/groovebox-080/ editor.');","if(minor===8&&patch<2)throw Error('Use the matching older editor from the installer, or install Groovebox 0.8.2.');");p.write_text(core,encoding='utf-8')
 d=S/'downloads';fw=O/'zombie-groovebox-0.8.2.fwsc';shutil.copy2(fw,d/fw.name)
 meta=json.loads((S/'releases.json').read_text(encoding='utf-8'));meta['releases']=[x for x in meta['releases'] if x['product']!='FM-1_63000802']
 for x in meta['releases']:
  if x['product']=='FM-1_63000801':x['editor']='webapp/groovebox-081/'
 meta['releases'].insert(0,dict(product='FM-1_63000802',version='0.8.2',label={'ru':'Groovebox 0.8.2 · SELECT=BPM / Зомби','en':'Groovebox 0.8.2 · SELECT=BPM / Zombie'},notes={'ru':'SELECT всегда BPM, память страниц, быстрые удержания и LOCK через HOME. Драм-шаги сохраняют остальные удары. Возвращена заставка «Зомби ФМ-1». Для BPM в FL/Ableton обнови скрипты до 0.8.2. Проверено программно, аппаратный тест ожидается.','en':'SELECT always BPM, page memory, faster holds and HOME layer lock. Drum step edits preserve other hits. Zombie FM-1 splash restored. Update DAW scripts to 0.8.2 for BPM in FL/Ableton. Software-tested; hardware confirmation pending.'},bytes=fw.stat().st_size,sha256=sha(fw),file='downloads/'+fw.name,editor='webapp/groovebox/'))
 meta['current']='FM-1_63000802';(S/'releases.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 shutil.copy2(F/'EDITIONS-082.md',d/'Groovebox-0.8.2-RU.md')
 with ZipFile(d/'Zombie-FM1-DAW-Profiles-0.8.2.zip','w',ZIP_DEFLATED) as z:
  for p in (E/'profiles').rglob('*'):
   if p.is_file() and '__pycache__' not in p.parts:z.write(p,p.relative_to(E/'profiles').as_posix())
  z.write(F/'EDITIONS-082.md','Groovebox-0.8.2-controls.md')
 for name,dest in [('zombie-splash.png','zombie-splash.png'),('fx-locked-ru.png','groovebox-lock.png')]:shutil.copy2(O/'groovebox'/name,S/'previews'/dest)
 p=S/'index.html';html=p.read_text(encoding='utf-8').replace('installer.js?v=1.2','installer.js?v=1.3').replace('GROOVEBOX 0.8.1','GROOVEBOX 0.8.2').replace('groovebox-source-0.8.1.zip','groovebox-source-0.8.2.zip').replace('Zombie-FM1-DAW-Profiles-0.8.0.zip','Zombie-FM1-DAW-Profiles-0.8.2.zip')
 if 'id="controls-082"' not in html:html=html.replace('<section class="bottom-notes">','<section class="bottom-notes" id="controls-082"><article><h2>Зомби ФМ-1 · 0.8.2</h2><img src="previews/zombie-splash.png" width="240" height="240" alt="Восстановленная заставка Зомби ФМ-1"><p class="muted">240 × 240 · 256 colors · host render</p></article><article><h2>SELECT = BPM · HOLD + HOME = LOCK</h2><img src="previews/groovebox-lock.png" width="240" height="240" alt="Зафиксированный слой FX"><p class="muted">ALGO: track · SEQ + OCT: bank · EDIT + OCT: Undo / Redo</p><a href="downloads/Groovebox-0.8.2-RU.md">Управление / Controls ↗</a></article></section><section class="bottom-notes">',1)
 p.write_text(html,encoding='utf-8')
 for p in (E/'shared').iterdir():
  if p.is_file():shutil.copy2(p,S/'source/editions/shared'/p.name)
 for name in ['build-editions.py','build-groovebox-082.py','prepare-groovebox082-web.py']:shutil.copy2(F/name,S/'source'/name)
 for name in ['edition.h','fm1.toml']:shutil.copy2(E/'groovebox'/name,S/'source/config/groovebox'/name)
 shutil.copytree(E/'profiles',S/'source/profiles',dirs_exist_ok=True,ignore=shutil.ignore_patterns('__pycache__'))
 with ZipFile(d/'groovebox-source-0.8.2.zip','w',ZIP_DEFLATED) as z:
  for directory,prefix in [(E/'shared','editions/shared'),(E/'assets','editions/assets'),(E/'profiles','editions/profiles'),(F/'platform-081','platform-081')]:
   for p in directory.rglob('*'):
    if p.is_file() and not any(x in p.parts for x in ['.git','build','__pycache__']):z.write(p,'source/'+prefix+'/'+p.relative_to(directory).as_posix())
  for name in ['piano.js','drums.js','presets.json']:z.write(E/name,'source/editions/'+name)
  for name in ['build-editions.py','build-groovebox-082.py','prepare-groovebox082-web.py','EDITIONS-082.md']:z.write(F/name,'source/'+name)
  for name in ['test_controls082.py','test_profiles082.py']:z.write(F/'tests'/name,'source/tests/'+name)
  z.write(F/'zombie-core/LICENSE','LICENSE')
 with ZipFile(O/'Zombie-Groovebox-0.8.2.zip','w',ZIP_DEFLATED) as z:
  for p in [fw,d/'groovebox-source-0.8.2.zip',d/'Zombie-FM1-DAW-Profiles-0.8.2.zip',F/'EDITIONS-082.md']:z.write(p,p.name)
 expected=json.loads(protected.read_text());assert all(sha(S/p)==h for p,h in expected.items()),'Protected previous release changed'
 p=S/'README.md';s=p.read_text(encoding='utf-8');s=s.split('\n\nGroovebox 0.8.2:')[0]+'\n\nGroovebox 0.8.2: SELECT always BPM, remembered pages, 180ms holds, HOME layer\nlock, SEQ bank shortcuts and additive kit step toggles. Original Zombie FM-1\n240x240 splash restored. New matching DAW profiles map relative tempo CC116.\nDetails: downloads/Groovebox-0.8.2-RU.md. Physical hardware unverified.\nSource: downloads/groovebox-source-0.8.2.zip. Sampler stays 0.8.0.\n';p.write_text(s,encoding='utf-8')
 for dest in [R/'output/zombie-core-site',F/'output/site']:shutil.copytree(S,dest,dirs_exist_ok=True,ignore=shutil.ignore_patterns('source','tests','package.json','README.md','__pycache__'))
 print(f'0.8.2 packaged; {len(expected)} previous download/editor files unchanged. No auto flash.')
if __name__=='__main__':build()

"""Package the optional experimental release; preserve the default release."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,shutil,subprocess
r=Path(__file__).resolve().parents[1]
fw=r/'downloads/zombie-groovebox-0.9.1.fwsc'
shutil.copy2(r/'source/output/editions-0.9.1/zombie-groovebox-0.9.1.fwsc',fw)
shutil.copy2(r/'source/output/editions-0.9.1/groovebox.wasm',r/'webapp/groovebox-090/zombie-core.wasm')
mfile=r/'releases.json';m=json.loads(mfile.read_text(encoding='utf-8'))
release=dict(product='FM-1_63000901',version='0.9.1',
 label=dict(ru='Groovebox 0.9.1 BETA · исправления драмки и сохранения',en='Groovebox 0.9.1 BETA · drum and persistence fixes'),
 notes=dict(ru='Исправления после аудита: SAVE/слоты, темп импорта, motion, зависшие MIDI-ноты/клавиши, SEL + PRESET, сохранение при перезапуске звука ПК и гонки редактора. Пять синтезаторов сохранены. На физическом FM-1 не проверено.',en='Audit fixes: SAVE/slots, imported tempo, motion, stuck MIDI notes/keys, SEL + PRESET, persistence across PC audio restart and editor races. Five synths preserved. Not tested on physical FM-1.'),
 bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),
 file='downloads/'+fw.name,editor='webapp/groovebox-090/',source='downloads/groovebox-source-0.9.1.zip')
m['releases']=[x for x in m['releases'] if x['product']!=release['product']]+[release]
mfile.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
# Base distribution contains the matching platform, existing generators/assets and
# build dependencies. Override its files with this revision, add all integration sources.
names=set(subprocess.check_output(['git','ls-files'],cwd=r,text=True).splitlines())
names.update(['source/AUDIT-091.md','tests/test_drum_store.mjs','tests/test_audio_engine.mjs','source/editions/shared/drum_suite.c','source/editions/shared/drum_suite.h','source/build-groovebox-091.py','source/platform-081/tools/build.py','tests/package_091.py','tests/test_hortator_suite.mjs','tests/test_hortator_worklet.mjs','tests/test_hortator_host.py','downloads/Groovebox-0.9.1-RU.md','licenses/GPL-3.0-Hortator.txt'])
for folder in ['source/drum-suite','source/third_party/hortator','webapp/groovebox-090']:
 names.update(str(p.relative_to(r)).replace('\\','/') for p in (r/folder).rglob('*') if p.is_file())
with ZipFile(r/'downloads/groovebox-source-0.9.0.zip') as base:
 names.update(base.namelist())
 with ZipFile(r/release['source'],'w',ZIP_DEFLATED) as out:
  for name in sorted(names):
   if not name.startswith(('source/','tests/','licenses/','webapp/groovebox-090/')) and name not in ['README.md','LICENSE','downloads/Groovebox-0.9.1-RU.md']:continue
   p=r/name
   if p.is_file():out.writestr(name,p.read_bytes())
   elif name in base.namelist():out.writestr(name,base.read(name))
print('Packaged beta:',release['sha256'],'default:',m['current'])

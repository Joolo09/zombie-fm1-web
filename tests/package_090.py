"""Package the optional experimental release; preserve the default release."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,shutil,subprocess
r=Path(__file__).resolve().parents[1]
fw=r/'downloads/zombie-groovebox-0.9.0.fwsc'
shutil.copy2(r/'source/output/editions-0.9.0/zombie-groovebox-0.9.0.fwsc',fw)
shutil.copy2(r/'source/output/editions-0.9.0/groovebox.wasm',r/'webapp/groovebox-090/zombie-core.wasm')
mfile=r/'releases.json';m=json.loads(mfile.read_text(encoding='utf-8'))
release=dict(product='FM-1_63000900',version='0.9.0',
 label=dict(ru='Groovebox 0.9.0 BETA · Hortator в дорожке 6',en='Groovebox 0.9.0 BETA · Hortator on track 6'),
 notes=dict(ru='Тестовая сборка: 8 драм-партий Hortator внутри дорожки 6; пять синтезаторов сохранены. Проверено в симуляторе и браузере, на физическом FM-1 НЕ проверено. Драм-дилей использует 8-битное компандирование; SAMPLE-kit ZOMBIE. Сохрани прежний проект перед проверкой.',en='Experimental: 8 Hortator drum lanes inside track 6; five synths preserved. Simulator/browser tested; physical FM-1 NOT tested. Drum delay uses 8-bit companding; original ZOMBIE sample kit. Back up the previous project before testing.'),
 bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),
 file='downloads/'+fw.name,editor='webapp/groovebox-090/',source='downloads/groovebox-source-0.9.0.zip')
m['releases']=[x for x in m['releases'] if x['product']!=release['product']]+[release]
mfile.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
# Base distribution contains the matching platform, existing generators/assets and
# build dependencies. Override its files with this revision, add all integration sources.
names=set(subprocess.check_output(['git','ls-files'],cwd=r,text=True).splitlines())
names.update(['source/editions/shared/drum_suite.c','source/editions/shared/drum_suite.h','source/build-groovebox-090.py','source/platform-081/tools/build.py','tests/package_090.py','tests/test_hortator_suite.mjs','tests/test_hortator_worklet.mjs','tests/test_hortator_host.py','downloads/Groovebox-0.9.0-RU.md','licenses/GPL-3.0-Hortator.txt'])
for folder in ['source/drum-suite','source/third_party/hortator','webapp/groovebox-090']:
 names.update(str(p.relative_to(r)).replace('\\','/') for p in (r/folder).rglob('*') if p.is_file())
with ZipFile(r/'downloads/groovebox-source-0.8.3.zip') as base:
 names.update(base.namelist())
 with ZipFile(r/release['source'],'w',ZIP_DEFLATED) as out:
  for name in sorted(names):
   if not name.startswith(('source/','tests/','licenses/','webapp/groovebox-090/')) and name not in ['README.md','LICENSE','downloads/Groovebox-0.9.0-RU.md']:continue
   p=r/name
   if p.is_file():out.writestr(name,p.read_bytes())
   elif name in base.namelist():out.writestr(name,base.read(name))
print('Packaged beta:',release['sha256'],'default:',m['current'])

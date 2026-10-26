"""Package matching 0.11.1 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.11.1.fwsc'
shutil.copy2(root/'source/output/editions-0.11.1'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.11.1/groovebox.wasm',root/'webapp/groovebox-111/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001101',version='0.11.1',label=dict(ru='Groovebox 0.11.1 beta · запись с клавиш + пианоролл',en='Groovebox 0.11.1 beta · keyboard recording + piano roll'),notes=dict(ru='Правка выбранного шага через EDIT + клавиши, 16 барабанных пэдов, чёрные клавиши для шагов и пауз, заметный курсор. Исправлен порядок быстрых нажатий эмулятора. Проверено программно, физический FM-1 ожидается.',en='EDIT + keys writes the focused step, sixteen drum pads, black-key step/rest shortcuts, visible cursor and ordered emulator input. Software tested; physical FM-1 pending.'),bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-111/',source='downloads/groovebox-source-0.11.1.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-111']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['source/build-groovebox-111.py','source/build-emulator.py','source/package-groovebox-111.py','source/GROOVEBOX-111-RU.md','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.11.1.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

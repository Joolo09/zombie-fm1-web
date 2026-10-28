"""Package matching 0.13.1 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.13.1.fwsc'
shutil.copy2(root/'source/output/editions-0.13.1'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.13.1/groovebox.wasm',root/'webapp/groovebox-131/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001301',version='0.13.1',label=dict(ru='Groovebox 0.13.1 beta · MIDI + запись + отмена',en='Groovebox 0.13.1 beta · MIDI + recording + undo'),notes=dict(ru='Повторные MIDI-удары снова срабатывают. REPLACE убирает старую дробь; пустые повороты ручек не стирают историю отмены; одинаковые высоты аккорда не дублируются. Проверено программно; физический FM-1 ожидается.',en='Repeated MIDI attacks retrigger, REPLACE clears stale rolls, no-op knob turns preserve undo, duplicate chord pitches are rejected. Software tested; physical FM-1 pending.'),bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-131/',source='downloads/groovebox-source-0.13.1.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-131']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['source/build-groovebox-131.py','source/build-emulator.py','source/package-groovebox-131.py','source/GROOVEBOX-131-RU.md','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.13.1.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

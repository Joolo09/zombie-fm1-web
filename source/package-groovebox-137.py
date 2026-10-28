"""Package matching 0.13.7 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.13.7.fwsc'
shutil.copy2(root/'source/output/editions-0.13.7'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.13.7/groovebox.wasm',root/'webapp/groovebox-137/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001307',version='0.13.7',label=dict(ru='Groovebox 0.13.7 beta · драмка с клавиш',en='Groovebox 0.13.7 beta · drum key workflow'),notes=dict(ru='Драмка: OP1–OP6 выбирают звук, PITCH/GLOBAL листают группы. Без REC белые включают/выключают шаги, EDIT + белая стирает. Исправлены подсветка и переход при смене OP на зажатой записи. REC сохраняет 16 пэдов. Мелодии не изменены. Физический FM-1 не проверен.',en='Drums: OP1–OP6 select sound, PITCH/GLOBAL change group. REC off: white keys toggle steps, EDIT+white erases. Fixes LEDs and held-take cursor after OP changes. REC keeps 16 pads; melody controls unchanged. Physical FM-1 unverified.'),bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-137/',source='downloads/groovebox-source-0.13.7.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-137']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['webapp/groovebox-100/drum-state.js','webapp/groovebox-100/drums.js','webapp/groovebox/zombie-core.wasm','source/build-groovebox-137.py','source/build-emulator.py','source/package-groovebox-137.py','source/GROOVEBOX-137-RU.md','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.13.7.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

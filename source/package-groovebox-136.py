"""Package matching 0.13.6 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.13.6.fwsc'
shutil.copy2(root/'source/output/editions-0.13.6'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.13.6/groovebox.wasm',root/'webapp/groovebox-136/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001306',version='0.13.6',label=dict(ru='Groovebox 0.13.6 beta · согласованное сохранение',en='Groovebox 0.13.6 beta · paired project saves'),notes=dict(ru='Мелодии и драмка сохраняются согласованной парой: сбой не смешивает разные поколения проекта. Старые проекты загружаются; новая схема хранения требует экспорта JSON перед откатом на 0.13.5. Физический FM-1 не проверен.',en='Commits melodies and drums as a verified pair, preventing mixed generations after a failed save. Legacy projects load; export JSON before downgrading to 0.13.5. Physical FM-1 unverified.'),bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-136/',source='downloads/groovebox-source-0.13.6.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-136']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['source/build-groovebox-136.py','source/build-emulator.py','source/package-groovebox-136.py','source/GROOVEBOX-136-RU.md','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.13.6.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

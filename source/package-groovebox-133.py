"""Package matching 0.13.3 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.13.3.fwsc'
shutil.copy2(root/'source/output/editions-0.13.3'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.13.3/groovebox.wasm',root/'webapp/groovebox-133/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001303',version='0.13.3',label=dict(ru='Groovebox 0.13.3 beta · отмена и длина драмки',en='Groovebox 0.13.3 beta · undo and drum length'),notes=dict(ru='Исправлены потеря UNDO после повторной очистки, выключение REC недоступным REDO и длина драм-партии в меню записи. Проверено программно; физический FM-1 ожидается.',en='Fixes undo loss after repeated clearing, unavailable redo stopping a drum take, and recording-menu drum length controls. Software tested; physical FM-1 pending.'),bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-133/',source='downloads/groovebox-source-0.13.3.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-133']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['source/build-groovebox-133.py','source/build-emulator.py','source/package-groovebox-133.py','source/GROOVEBOX-133-RU.md','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.13.3.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

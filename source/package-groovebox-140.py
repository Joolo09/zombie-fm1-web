"""Package matching 0.14.0 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.14.0.fwsc'
shutil.copy2(root/'source/output/editions-0.14.0'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.14.0/groovebox.wasm',root/'webapp/groovebox-140/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001400',version='0.14.0',label=dict(ru='Groovebox 0.14.0 beta · чистый звук',en='Groovebox 0.14.0 beta · clean sound'),notes=dict(ru='Без щелчков при STOP, смене дорожки и перехвате голосов. Лимитер на шине вместо жёсткого клиппинга. Повтор одной ноты не занимает все 8 голосов. ATTACK 0 без щелчка. Ноты и аккорды на экране названиями (C4, E4). Управление и формат проектов 0.13.7 сохранены. Физический FM-1 не проверен.',en='No clicks on STOP, track cuts and voice stealing. Bus limiter instead of hard clipping. A repeated note no longer fills all 8 voices. Click-free ATTACK 0. Notes and chords shown by name (C4, E4). Controls and project format of 0.13.7 unchanged. Physical FM-1 unverified.'),channel='recommended',bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-140/',source='downloads/groovebox-source-0.14.0.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r];s['current']=r['product']
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-140']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['webapp/groovebox-100/drum-state.js','webapp/groovebox-100/drums.js','webapp/groovebox/zombie-core.wasm','source/build-groovebox-140.py','source/build-emulator.py','source/package-groovebox-140.py','source/GROOVEBOX-140-RU.md','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-137-RU.md','tests/test_sound_quality.mjs','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.14.0.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

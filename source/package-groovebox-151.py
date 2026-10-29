"""Package matching 0.15.1 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.15.1.fwsc'
shutil.copy2(root/'source/output/editions-0.15.1'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.15.1/groovebox.wasm',root/'webapp/groovebox-151/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001501',version='0.15.1',label=dict(ru='Groovebox 0.15.1 beta · исправления',en='Groovebox 0.15.1 beta · fixes'),notes=dict(ru='Исправлены: переполнение буфера заголовка драмки (0.15.0), выход за таблицу синуса (щелчки в FM), «удвоить» на драмке теперь удваивает весь кит, импорт проекта с банком не по курсору. Найдено фаззингом под ASan/UBSan и проверкой секвенсора. Управление и проекты 0.15.0 без изменений. Физический FM-1 не проверен.',en='Fixes: drum title buffer overflow (0.15.0), sine table overrun (FM clicks), drum DOUBLE doubles the whole kit, imported projects with a bank away from the cursor. Found by ASan/UBSan fuzzing and a sequencer differential check. 0.15.0 controls and projects unchanged. Physical FM-1 unverified.'),channel='recommended',bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-151/',source='downloads/groovebox-source-0.15.1.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r];s['current']=r['product']
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-151']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['webapp/groovebox-100/drum-state.js','webapp/groovebox-100/drums.js','webapp/groovebox/zombie-core.wasm','source/build-groovebox-151.py','source/build-emulator.py','source/package-groovebox-151.py','source/GROOVEBOX-151-RU.md','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-137-RU.md','source/GROOVEBOX-140-RU.md','source/GROOVEBOX-150-RU.md','tests/test_sound_quality.mjs','tests/test_step_length.mjs','emulator/checks/steps-150.png','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.15.1.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

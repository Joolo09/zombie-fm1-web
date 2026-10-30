"""Package matching 0.16.0 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.16.0.fwsc'
shutil.copy2(root/'source/output/editions-0.16.0'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.16.0/groovebox.wasm',root/'webapp/groovebox-160/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001600',version='0.16.0',label=dict(ru='Groovebox 0.16.0 beta · запись и драмка как в Felucca',en='Groovebox 0.16.0 beta · Felucca-style recording and drums'),notes=dict(ru='REC с главной: отсчёт 1 такт, затем запись по кругу; на SEQ — по шагам. Драмка: шаг переключается при отпускании, зажатый шаг + K2/K3/K4 = шанс/сила/дробь, подсветка долей. SAVE во время игры сохраняет после STOP. Новый экран: огибающая, волна, микшер, эффекты. Сначала попробуйте в эмуляторе. Физический FM-1 не проверен.',en='REC from home: one-bar count-in, then a live loop; on SEQ: step entry. Drums: steps toggle on release, a held step + K2/K3/K4 = chance/velocity/roll, beat LEDs. SAVE while playing saves on STOP. New screens: envelope, waveform, mixer, effects. Try it in the emulator first. Physical FM-1 unverified.'),channel='beta',bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-160/',source='downloads/groovebox-source-0.16.0.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-160']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['webapp/groovebox-100/drum-state.js','webapp/groovebox-100/drums.js','webapp/groovebox/zombie-core.wasm','source/build-groovebox-160.py','source/build-emulator.py','source/package-groovebox-160.py','source/GROOVEBOX-160-RU.md','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-137-RU.md','source/GROOVEBOX-140-RU.md','source/GROOVEBOX-150-RU.md','source/GROOVEBOX-151-RU.md','tests/test_recording_160.mjs','tests/test_drums_160.mjs','emulator/checks/ui-160.png','tests/fuzz_panel.py','tests/asan-cc.sh','tests/test_sound_quality.mjs','tests/test_step_length.mjs','emulator/checks/steps-150.png','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.16.0.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

"""Package matching 0.17.0 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.17.0.fwsc'
shutil.copy2(root/'source/output/editions-0.17.0'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.17.0/groovebox.wasm',root/'webapp/groovebox-170/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63001700',version='0.17.0',label=dict(ru='Groovebox 0.17.0 beta · лад, правка шагов, SEQ-инструменты',en='Groovebox 0.17.0 beta · scale, step editing, SEQ tools'),notes=dict(ru='Лад и тоника: клавиши попадают в тональность, ноты лада подсвечены. Зажми шаг мелодии и крути K1–K4. SEQ-инструменты: Эвклид, бит, половина, COOK. MIDI clock наружу (выкл по умолчанию). Отмена на 8 шагов. Физический FM-1 не проверен.',en='Scale and root: keys snap to the key and its notes glow. Hold a melodic step and turn K1-K4. SEQ tools: Euclid, beat, half, COOK. MIDI clock out (off by default). Eight undo steps. Physical FM-1 unverified.'),channel='beta',bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-170/',source='downloads/groovebox-source-0.17.0.zip')
s['releases']=[v for v in s['releases'] if v['product']!=r['product']]+[r]
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-170']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['webapp/groovebox-100/drum-state.js','webapp/groovebox-100/drums.js','webapp/groovebox/zombie-core.wasm','source/build-groovebox-170.py','source/build-emulator.py','source/package-groovebox-170.py','source/GROOVEBOX-170-RU.md','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-137-RU.md','source/GROOVEBOX-140-RU.md','source/GROOVEBOX-150-RU.md','source/GROOVEBOX-151-RU.md','source/GROOVEBOX-160-RU.md','tests/test_scale_170.mjs','tests/test_melody_hold_170.mjs','tests/test_clock_170.mjs','tests/test_tools_170.mjs','tests/test_history_170.mjs','tests/test_editor_system_170.mjs','emulator/checks/ui-170.png','tests/test_recording_160.mjs','tests/test_drums_160.mjs','emulator/checks/ui-160.png','tests/fuzz_panel.py','tests/asan-cc.sh','tests/test_sound_quality.mjs','tests/test_step_length.mjs','emulator/checks/steps-150.png','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.17.0.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

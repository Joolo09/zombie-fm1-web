"""Package matching 0.21.1 firmware, editor, simulator and complete source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parents[1]
fw=root/'downloads/zombie-groovebox-0.21.1.fwsc'
shutil.copy2(root/'source/output/editions-0.21.1'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.21.1/groovebox.wasm',root/'webapp/groovebox-180/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=dict(product='FM-1_63002101',version='0.21.1',label=dict(ru='Groovebox 0.21.1 · аккорды не пропадают, REC очищает паттерн, генератор, 249 пресетов',en='Groovebox 0.21.1 · chords keep sounding, hold REC to clear, generator, 249 presets'),notes=dict(ru='Исправлено: зажатый аккорд глох, когда играл секвенсор. Долгий REC на странице SEQ очищает паттерн дорожки. Генератор мелодий и ритмов (8 алгоритмов, 7 стилей, мутация, развитие, джем), глайд, вибрато, формы и цели LFO, FM-огибающая, дрейф, биткраш, дробь. 11 новых банков: 249 пресетов.',en='Fixed: a held chord went silent while the sequence played. Hold REC on the SEQ page to clear the track pattern. A melody and rhythm generator (8 algorithms, 7 styles, mutate, evolve, jam), glide, vibrato, LFO shapes and targets, FM envelope, drift, crush, roll. 11 new banks: 249 presets.'),channel='recommended',bytes=fw.stat().st_size,sha256=hashlib.sha256(fw.read_bytes()).hexdigest(),file='downloads/'+fw.name,editor='webapp/groovebox-180/',source='downloads/groovebox-source-0.21.1.zip')
s['releases']=[r]
s['current']=r['product']
for v in s['releases']:
 if v['product'] in('FM-1_63001700','FM-1_63001800'):v['channel']='archive'
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for folder in ['source/editions/shared','tests','emulator','webapp/groovebox-180']:
 for f in (root/folder).rglob('*'):
  if f.is_file() and '__pycache__' not in f.parts:entries[f.relative_to(root).as_posix()]=f.read_bytes()
for n in ['webapp/groovebox-100/drum-state.js','webapp/groovebox-100/drums.js','webapp/groovebox/zombie-core.wasm','source/build-groovebox-180.py','source/build-emulator.py','source/package-groovebox-180.py','source/package-groovebox-181.py','source/package-groovebox-182.py','source/package-groovebox-183.py','source/package-groovebox-190.py','source/package-groovebox-191.py','source/package-groovebox-200.py','source/package-groovebox-201.py','source/package-groovebox-202.py','source/package-groovebox-210.py','source/package-groovebox-211.py','tests/test_fix_211.mjs','source/GROOVEBOX-210-RU.md','tests/test_gen_210.mjs','source/editions/bank-210.json','tests/test_hold_undo_201.mjs','tests/test_samples_200.mjs','source/GROOVEBOX-200-RU.md','tests/test_step_191.mjs','tests/test_system_190.mjs','source/GROOVEBOX-190-RU.md','source/build-groovebox-170.py','source/GROOVEBOX-180-RU.md','source/GROOVEBOX-170-RU.md','source/editions/zombie-presets.json','source/render-zmb-bank.mjs','tests/test_scene_180.mjs','previews/fm1-home.webp','previews/fm1-saver-night.webp','source/build-editions.py','source/GROOVEBOX-AUDIT-136-RU.md','source/GROOVEBOX-137-RU.md','source/GROOVEBOX-140-RU.md','source/GROOVEBOX-150-RU.md','source/GROOVEBOX-151-RU.md','source/GROOVEBOX-160-RU.md','tests/test_scale_170.mjs','tests/test_melody_hold_170.mjs','tests/test_clock_170.mjs','tests/test_tools_170.mjs','tests/test_history_170.mjs','tests/test_editor_system_170.mjs','emulator/checks/ui-170.png','tests/test_recording_160.mjs','tests/test_drums_160.mjs','emulator/checks/ui-160.png','tests/fuzz_panel.py','tests/asan-cc.sh','tests/test_sound_quality.mjs','tests/test_step_length.mjs','emulator/checks/steps-150.png','source/GROOVEBOX-130-RU.md','source/editions/idm-presets.json','source/render-idm-bank.mjs','source/editions/assets/wasm-exports.json','source/platform-081/web/emu/emu_web.c','source/platform-081/host/plat_host.c']:
 entries[n]=(root/n).read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.21.1.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

"""Prepare the reviewed 0.8.3 artifacts without changing earlier downloads."""
from pathlib import Path
import hashlib
import json
import shutil
from zipfile import ZipFile, ZIP_DEFLATED

root = Path(__file__).resolve().parents[1]
fw = root / 'source/output/editions-0.8.3/zombie-groovebox-0.8.3.fwsc'
target = root / 'downloads' / fw.name
shutil.copy2(fw, target)
manifest_path = root / 'releases.json'
manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
release = dict(product='FM-1_63000803', version='0.8.3',
    label={'ru': 'Groovebox 0.8.3 · исправление SAVE', 'en': 'Groovebox 0.8.3 · SAVE LED fix'},
    notes={'ru': 'SAVE больше не горит постоянно после поворота K2. Несохранённые изменения: * на экране. Управление и DAW-профили 0.8.2 сохранены. Проверено в симуляторе; аппаратный тест ожидается.',
           'en': 'SAVE no longer stays brightly lit after turning K2. Unsaved edits use an on-screen *. Controls and DAW profiles remain compatible with 0.8.2. Simulator tested; hardware confirmation pending.'},
    bytes=target.stat().st_size, sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
    file='downloads/' + target.name, editor='webapp/groovebox/')
manifest['releases'] = [release] + [r for r in manifest['releases'] if r['product'] != release['product']]
manifest['current'] = release['product']
manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

# Retain the exact platform and assets from the previous source distribution.
# Replace only edited files and add the build wrapper, test and release notes.
updates = ['source/editions/shared/panel.c', 'source/editions/shared/edition_ui.inc',
           'source/editions/shared/edition_host.inc', 'source/build-groovebox-083.py',
           'tests/test_save_led.py', 'tests/package_083.py', 'downloads/Groovebox-0.8.3-RU.md']
with ZipFile(root / 'downloads/groovebox-source-0.8.2.zip') as old:
    with ZipFile(root / 'downloads/groovebox-source-0.8.3.zip', 'w', ZIP_DEFLATED) as new:
        for item in old.infolist():
            new.writestr(item.filename, (root / item.filename).read_bytes() if item.filename in updates else old.read(item.filename))
        for name in updates:
            if name not in old.namelist():
                new.write(root / name, name)
print('Packaged', target.name, release['sha256'])

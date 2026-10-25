"""Package the checked 0.10.0 build with matching source, without a drum-machine vendor tree."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
root=Path(__file__).resolve().parent.parent
fw=root/'downloads/zombie-groovebox-0.10.0.fwsc'
shutil.copy2(root/'source/output/editions-0.10.0'/fw.name,fw)
shutil.copy2(root/'source/output/editions-0.10.0/groovebox.wasm',root/'webapp/groovebox-100/zombie-core.wasm')
p=root/'releases.json';s=json.loads(p.read_text(encoding='utf-8'))
r=next(r for r in s['releases'] if r['product']=='FM-1_63001000')
r['bytes']=fw.stat().st_size;r['sha256']=hashlib.sha256(fw.read_bytes()).hexdigest()
p.write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\r\n')
with ZipFile(root/'downloads/groovebox-source-0.8.3.zip') as old:
 entries={n:((root/n).read_bytes() if (root/n).is_file() else old.read(n)) for n in old.namelist() if not n.endswith('/')}
for n in ['source/build-groovebox-100.py','source/package-groovebox-100.py','source/editions/shared/drum_machine.h','source/editions/shared/drum_machine.inc','source/ZOMBIE-DRUMS-100.md','tests/test_zombie_drums.mjs','tests/zombie-drum-controls.txt','webapp/groovebox/zombie-core.wasm']:
 entries[n]=(root/n).read_bytes()
for f in (root/'webapp/groovebox-100').iterdir():
 if f.is_file():entries[f.relative_to(root).as_posix()]=f.read_bytes()
with ZipFile(root/'downloads/groovebox-source-0.10.0.zip','w',ZIP_DEFLATED) as z:
 for n,b in sorted(entries.items()):z.writestr(n,b)
print('Packaged',r['product'],r['sha256'])

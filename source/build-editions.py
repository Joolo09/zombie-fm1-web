"""Isolated Groovebox/Sampler 0.8.0 builds; never flash USB or replace old releases."""
from pathlib import Path
import argparse, ast, hashlib, json, os, re, shutil, subprocess, sys

FIX=Path(__file__).resolve().parent
ROOT=FIX.parent
ED=FIX/'editions'
OUT=FIX/'output/editions-0.8.2'
VERSION='0.8.2'
PLATFORM=FIX/'platform-081'
PY=sys.executable
ENV=dict(os.environ,PYTHONUTF8='1',JIELI_TOOLCHAIN=os.environ.get('JIELI_TOOLCHAIN',str(ROOT/'.tooling/jieli-win/C$/JL/pi32')),AC79_SDK=os.environ.get('AC79_SDK',str(ROOT/'.tooling/ac79-sdk')),X0X_FONTSET='terminus',CC=os.environ.get('CC',str(ROOT/'tools/host-cc.cmd') if (ROOT/'tools/host-cc.cmd').exists() else 'cc'),ZIG_GLOBAL_CACHE_DIR=str(ROOT/'.tooling/zig-global'),ZIG_LOCAL_CACHE_DIR=str(ROOT/'.tooling/zig-local'))

def run(args,log):
    result=subprocess.run([str(x) for x in args],cwd=ROOT,env=ENV,capture_output=True)
    (OUT/log).write_bytes(result.stdout+result.stderr)
    if result.returncode:
        print((result.stdout+result.stderr).decode('utf-8',errors='replace')[-6500:])
        raise SystemExit(result.returncode)
    print(log+': PASS',flush=True)

def presets(kind="groovebox"):
    rows=re.findall(r'\{"(\w+)",(-?\d+),(-?\d+),(-?\d+)\}',(ED/'shared/dsp.c').read_text())
    bank=json.loads((ED/'presets.json').read_text(encoding='utf-8'))
    assert len(rows)==27 and len(bank)==64
    if kind=='groovebox':
        bank+=json.loads((ED/'idm-presets.json').read_text(encoding='utf-8'))
        if (ED/'zombie-presets.json').exists():bank+=json.loads((ED/'zombie-presets.json').read_text(encoding='utf-8'))
    assert len(bank)<=128
    keys=[r[0] for r in rows];values=[]
    for name,changes in bank:
        assert len(name)<24
        p=[int(r[3]) for r in rows]
        for key,value in changes.items():
            i=keys.index(key);assert int(rows[i][1])<=value<=int(rows[i][2]),(name,key,value);p[i]=value
        assert p[0]<8;values.append(p)
    return bank,values

def prepare(kind):
    app=ED/kind;app.mkdir(exist_ok=True);sample=kind=='sampler';bank,values=presets(kind)
    for p in (ED/'shared').iterdir():
        if p.is_file():shutil.copy2(p,app/p.name)
    title='ZOMBIE SAMPLER' if sample else 'ZOMBIE GROOVEBOX'
    (app/'edition.h').write_text(f'#pragma once\n#define Z_SAMPLER_EDITION {int(sample)}\n#define Z_EDITION_PRESETS {len(bank)}\n#define Z_EDITION_VERSION "{VERSION}"\n#define Z_EDITION_TITLE "{title}"\n#define Z_PROTOCOL_ID "ZCORE/1/{VERSION}/{kind}"\n',encoding='utf-8')
    (app/'fm1.toml').write_text(f'''name = "zombie-{kind}"
id_prefix = "FM-1_{64 if sample else 63}"
usb_manufacturer = "Zombie Core"
usb_product = "Zombie {kind.title()} FM-1"
ble = false
usb_audio_in = true
usb_audio_out = false
second_core = true
o2 = ["dsp.c", "output.c"]
words = "words.tsv"
tagline = "Independent {kind}, animated 240x240 display and DAW MIDI"
licence = "MIT / MPL-2.0 platform"
''',encoding='utf-8')
    gen=app/'build/gen';gen.mkdir(parents=True,exist_ok=True)
    for p in (ED/'assets').glob('z_*.h'):
        if p.name!='z_presets.h':shutil.copy2(p,gen/p.name)
    names=[b[0] for b in bank]
    if sample:
        names=['Pad Tone %02d'%(i+1) for i in range(64)]
        for p in values:p[0]=8;p[6]=0;p[8]=127;p[9]=8
    (gen/'z_presets.h').write_text(f'/* Original MIT factory bank. */\nstatic const char *const Z_PRESET_NAMES[{len(bank)}]='+json.dumps(names).replace('[','{').replace(']','}')+f';\nstatic const int16_t Z_PRESET_VALUES[{len(bank)}][27]='+json.dumps(values).replace('[','{').replace(']','}')+';\n',encoding='utf-8')
    return app,names

def script(kind):
    note='24' if kind=='sampler' else '35'
    s=f'''wait 100
editioncheck
defaults
wait 200
shot {kind}-home-ru.png
synthtest
master 3000
peakreset
wav {kind}-keyboard.wav
key w0 down 110
wait 180
expect peak_db_min -45
key w0 up
wait 250
wavstop
midiready
out
key w0 down 100
expect_out USB 90 {note} 64
expect voices 0
turn ALGO 1
key w0 up
expect_out USB 80 {note} 00
track 0
turn K1 1
expect_out USB B0 4A 41
turn K2 1
expect_out USB B0 47 41
turn K3 1
expect_out USB B0 49 41
turn K4 1
expect_out USB B0 48 41
tap PLAY
expect_out USB BF 72 7F
tap PLAY
expect_out USB BF 72 00
tap REC
expect_out USB BF 73 7F
tap REC
expect_out USB BF 73 00
dawshow
wait 100
shot {kind}-daw.png
defaults
'''
    if kind=='groovebox':s+='''rolltest
wait 100
shot groovebox-piano-ru.png
turn K4 1
expect duration 5
turn K2 2
expect note 62
tap SEQ
expect subpage 1
turn K1 1
expect chord1 66
press EDIT
tap OCT-
release EDIT
expect chord1 65
press EDIT
tap OCT+
release EDIT
expect chord1 66
en
wait 100
shot groovebox-piano-en.png
defaults
wait 100
wav groovebox-demo.wav
tap PLAY
wait 4000
shot groovebox-playing.png
wait 4000
tap PLAY
wavstop
expect playing 0
'''
    else:s+='''gatepad
peakreset
midi USB 90 24 6E
wait 80
expect peak_db_min -45
midi USB 80 24 00
wait 400
expect voices 0
key w0 down 100
wait 20
shot sampler-hit.png
key w0 up
turn K2 1
expect sampler_bank 1
turn PRESET 1
wait 100
shot sampler-trim.png
en
wait 100
shot sampler-en.png
'''
    s+='''defaults
press HOME
tap SEL
release HOME
expect midi_mode 1
press HOME
tap SEL
release HOME
expect midi_mode 2
press HOME
tap SEL
release HOME
expect midi_mode 0
reboot
expect midi_mode 0
'''
    path=OUT/f'{kind}-test.txt';path.write_text(s,encoding='utf-8');return path

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--host-only',action='store_true');parser.add_argument('--firmware-only',action='store_true');parser.add_argument('--wasm-only',action='store_true');parser.add_argument('--kind',choices=['groovebox','sampler']);args=parser.parse_args()
    OUT.mkdir(parents=True,exist_ok=True)
    for kind in ([args.kind] if args.kind else ['groovebox','sampler']):
        app,names=prepare(kind)
        if not args.firmware_only and not args.wasm_only:
            run([PY,'-X','utf8',PLATFORM/'tools/build.py',app,'--host','--release',VERSION],kind+'-host-build.log')
            shots=OUT/kind;shots.mkdir(exist_ok=True)
            run([app/f'build/host/zombie-{kind}_host',script(kind),shots],kind+'-host-tests.log')
        if args.host_only:continue
        fw=app/f'build/zombie-{kind}-{VERSION}.fwsc'
        if not args.wasm_only:
            run([PY,'-X','utf8',PLATFORM/'tools/build.py',app,'--release',VERSION],kind+'-device-build.log')
            shutil.copy2(fw,OUT/fw.name)
        exports=json.loads((ED/'assets/wasm-exports.json').read_text())
        wasm=OUT/f'{kind}.wasm'
        run([os.environ.get('ZIG',str(ROOT/'.tooling/zig-windows-x86_64-0.13.0/zig.exe')),'cc','-target','wasm32-freestanding','-O2','-ffp-contract=off','-ffreestanding','-nostdlib','-DZ_WASM','-I'+str(app/'build/gen'),app/'dsp.c',app/'output.c','-Wl,--no-entry','-Wl,--export-memory',*['-Wl,--export=z_'+n for n in exports],*(['-Wl,--export=z_dm_'+n for n in ['data','checkpoint','history_state','pattern','size','snapshot','apply','param','step','select','position','init','migrate','length','common_length']] if kind=='groovebox' else []),'-o',wasm],kind+'-wasm-build.log')
        (OUT/f'{kind}-presets.json').write_text(json.dumps(names),encoding='utf-8')
        if fw.exists():(OUT/(fw.name+'.sha256')).write_text(hashlib.sha256(fw.read_bytes()).hexdigest()+'  '+fw.name+'\n',encoding='utf-8')
    print('Edition builds ready; no physical FM-1 flashed.',flush=True)

if __name__=='__main__':main()

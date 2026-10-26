"""Build Zombie's complete native panel + audio simulator as browser WASM (Zig/WASI)."""
from pathlib import Path
import re,subprocess,os,json,importlib.util,shutil,hashlib
ROOT=Path(__file__).resolve().parent.parent
PLAT=ROOT/'source/platform-081';APP=ROOT/'source/editions/groovebox'
OUT=ROOT/'source/output/zombie-emulator'
def main():
 spec=importlib.util.spec_from_file_location('edbuild',ROOT/'source/build-editions.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder);builder.VERSION='0.13.0';builder.prepare('groovebox')
 if not (APP/'build/gen/felucca_font.h').exists():raise SystemExit('First run: python source/build-groovebox-130.py --host-only')
 OUT.mkdir(parents=True,exist_ok=True)
 src=PLAT/'web/emu/emu_web.c'
 names=sorted(set(re.findall(r'\b(web_\w+)\s*\(',src.read_text()))-{'web_audio','web_said'})
 names+=['z_snapshot','z_state_size','z_track_hits','z_revision','z_playing','z_position','z_get','z_dm_data','z_dm_size','z_solo_state','z_live_state','z_voice_count','z_preset']
 zig=os.environ.get('ZIG',str(ROOT.parent/'.tooling/zig-windows-x86_64-0.13.0/zig.exe'))
 cmd=[zig,'cc','-target','wasm32-wasi','-mexec-model=reactor','-O2','-ffp-contract=off','-DPLAT_HOST','-DPLAT_WEB','-DAPP_VERSION="0.13.0"','-Wno-unused-function','-Wno-unused-variable']
 cmd+=['-I'+str(p) for p in [PLAT/'host',PLAT/'device',PLAT/'include',PLAT/'lib',APP,APP/'build/gen']]
 cmd+=[str(src),str(APP/'dsp.c'),str(APP/'output.c'),'-Wl,--export-memory','-Wl,-z,stack-size=1048576']+['-Wl,--export='+n for n in names]+['-o',str(OUT/'app.wasm')]
 env=dict(os.environ,ZIG_GLOBAL_CACHE_DIR=str(OUT/'cache-global'),ZIG_LOCAL_CACHE_DIR=str(OUT/'cache-local'))
 r=subprocess.run(cmd,env=env,capture_output=True);(OUT/'build.log').write_bytes(r.stdout+r.stderr)
 print((r.stdout+r.stderr).decode(errors='replace')[-3200:]);print('emulator build:',r.returncode)
 if not r.returncode:
  target=ROOT/'emulator';target.mkdir(exist_ok=True);shutil.copy2(OUT/'app.wasm',target/'app.wasm')
  sources=sorted((ROOT/'source/editions/shared').glob('*'));source_hash=hashlib.sha256(b''.join(p.read_bytes() for p in sources if p.is_file())+src.read_bytes()).hexdigest()
  (target/'build.json').write_text(json.dumps(dict(version='0.13.0',product='FM-1_63001300',wasm_sha256=hashlib.sha256((OUT/'app.wasm').read_bytes()).hexdigest(),source_sha256=source_hash),indent=2)+'\n',encoding='utf-8')
 raise SystemExit(r.returncode)
if __name__=='__main__':main()

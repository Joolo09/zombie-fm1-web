"""Repeatable Groovebox-only audit. No physical MIDI/USB flashing is performed."""
from pathlib import Path
import argparse,subprocess,sys,os,json,hashlib,time,shutil
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'source/output/zombie-emulator';OUT.mkdir(parents=True,exist_ok=True)
parser=argparse.ArgumentParser();parser.add_argument('--build',action='store_true');parser.add_argument('--fuzz',type=int,default=0,help='N random sessions on an ASan/UBSan host (POSIX)');args=parser.parse_args()
results=[]
def run(name,cmd):
 begin=time.monotonic();r=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True)
 (OUT/(name+'.log')).write_bytes(r.stdout+r.stderr)
 results.append(dict(name=name,passed=r.returncode==0,seconds=round(time.monotonic()-begin,3)))
 print(('PASS ' if r.returncode==0 else 'FAIL ')+name,flush=True)
 if r.returncode:
  print((r.stdout+r.stderr).decode('utf-8',errors='replace')[-4200:]);raise SystemExit(r.returncode)
py=[sys.executable,'-X','utf8'];node=os.environ.get('NODE','node')
if args.build:
 run('groovebox-build',py+['source/build-groovebox-151.py'])
 run('native-emulator-build',py+['source/build-emulator.py'])
shutil.copy2(ROOT/'source/output/editions-0.15.1/groovebox.wasm',ROOT/'webapp/groovebox-151/zombie-core.wasm')
for name,test,extra in [('drum-operator-labels','test_drum_operator_labels.mjs',[]),('drum-key-workflow','test_drum_key_workflow.mjs',[]),('paired-project-application','test_project_pair_app.mjs',[]),('full-panel','test_full_emulator.mjs',[]),('keyboard-roll','test_keyboard_roll.mjs',[]),('dsp-boundaries','test_dsp_boundaries.mjs',[]),('live-edit-feedback','test_live_edit_feedback.mjs',[]),('editor-link-sessions','test_editor_link_sessions.mjs',[]),('editor-key-queue','test_editor_key_queue.mjs',[]),('protocol-clock','test_protocol_clock_audit.mjs',[]),('protocol-integrity','test_protocol_integrity.mjs',[]),('history-and-length','test_history_length_audit.mjs',[]),('transition-regressions','test_transition_audit.mjs',[]),('followup-regressions','test_followup_audit.mjs',[]),('playback-lights-and-grid','test_lights_grid.mjs',[]),('idm-regressions','test_idm_regressions.mjs',[]),('native-performance','test_native_performance.mjs',[]),('native-worklet','test_native_worklet.mjs',[]),('pattern-recording','test_pattern_recording.mjs',['groovebox-151']),('synth-and-drums','test_zombie_drums.mjs',['groovebox-151']),('audio-lifecycle','test_audio_engine.mjs',['groovebox-151']),('sound-quality','test_sound_quality.mjs',['groovebox-151']),('step-length','test_step_length.mjs',[]),('bugfix-151','test_bugfix_151.mjs',[]),('web-installer','web-installer.mjs',[])]:
 run(name,[node,ROOT/'tests'/test,*extra])
host=ROOT/'source/editions/groovebox/build/host/zombie-groovebox_host'
run('save-led',py+['tests/test_save_led.py',host])
run('physical-pattern-controls',[host,'tests/native-pattern-editing.txt',OUT])
run('drum-save-reboot',[host,'tests/zombie-drum-controls.txt',OUT])
project=OUT/'project-audit.txt';project.write_text('wait 100\nprojectaudit\nwait 100\n',encoding='utf-8')
run('project-migration-and-orphans',[host,project,OUT])
zig=os.environ.get('ZIG',str(ROOT.parent/'.tooling/zig-windows-x86_64-0.13.0/zig.exe'))
run('journal-build',[zig,'cc','-O2','-I'+str(ROOT/'source/editions/shared'),'-I'+str(ROOT/'source/editions/groovebox'),'-I'+str(ROOT/'source/platform-081/include'),ROOT/'tests/test_zombie_journal.c','-o',OUT/'journal-test.exe'])
run('actual-nor-journal',[OUT/'journal-test.exe'])
run('paired-journal-build',[zig,'cc','-O2','-I'+str(ROOT/'source/editions/shared'),'-I'+str(ROOT/'source/editions/groovebox'),'-I'+str(ROOT/'source/platform-081/include'),ROOT/'tests/test_project_pair.c','-o',OUT/'pair-test.exe'])
run('paired-project-journal',[OUT/'pair-test.exe'])
run('idm-sound-bank',[node,'source/render-idm-bank.mjs'])
if args.fuzz:
 env=dict(os.environ,CC=str(ROOT/'tests/asan-cc.sh'))
 r=subprocess.run(py+['source/build-groovebox-151.py','--host-only'],cwd=ROOT,env=env,capture_output=True);(OUT/'asan-host-build.log').write_bytes(r.stdout+r.stderr)
 if r.returncode:print((r.stdout+r.stderr).decode(errors='replace')[-3000:]);raise SystemExit('ASan host build failed')
 asan=OUT/'zombie-groovebox_asan';shutil.copy2(host,asan)
 run('fuzz-panel-midi-editor',py+['tests/fuzz_panel.py',asan,'--runs',args.fuzz,'--steps',400,'--out',OUT/'fuzz'])
 subprocess.run(py+['source/build-groovebox-151.py','--host-only'],cwd=ROOT,capture_output=True)
report=dict(version='0.15.1',hardware_verified=False,scope='C panel, display, audio and platform doubles; simulated NOR and MIDI',checks=results)
for key,p in [('emulator','emulator/app.wasm'),('firmware','source/output/editions-0.15.1/zombie-groovebox-0.15.1.fwsc')]:
 f=ROOT/p
 if f.exists():report[key+'_sha256']=hashlib.sha256(f.read_bytes()).hexdigest()
(ROOT/'emulator/audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Audit complete. Hardware keyboard, real-time DSP load and physical USB/power behavior still require an FM-1.',flush=True)

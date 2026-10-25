"""Actual app protocol, drum state machine and panel; simulated flash only."""
from pathlib import Path
import subprocess,sys,tempfile
host=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory() as tmp:
 root=Path(tmp);script=root/'drum.txt'
 script.write_text('wait 100\nhortatorcheck\nauditcheck\ndefaults\ntrack 5\nwait 100\nexpect drum_keys 0\npress SEL\nturn PRESET 1\nrelease SEL\nexpect drum_keys 0\ntap SEL\nexpect drum_keys 1\nshot hortator-native.png\nturn ALGO -1\nwait 100\nshot synth-return.png\n',encoding='utf-8')
 subprocess.run([str(host),str(script),str(root)],check=True,timeout=30)
print('PASS drum engine features, protocol and return to synth panel')

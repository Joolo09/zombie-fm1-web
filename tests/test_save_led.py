"""Exercise actual firmware input, LED output and simulated flash; no USB."""
from pathlib import Path
import re
import subprocess
import sys
import tempfile

host = Path(sys.argv[1]).resolve()
commands = '''wait 250
tap SAVE
wait 250
expect dirty 0
leds
turn K2 1
expect dirty 1
leds
wait 1000
expect dirty 1
leds
shot unsaved-k2.png
press SAVE
wait 40
expect dirty 0
leds
release SAVE
wait 250
expect dirty 0
leds
shot saved-k2.png
reboot
wait 250
expect dirty 0
leds
'''
with tempfile.TemporaryDirectory() as tmp:
    folder = Path(tmp)
    script = folder / 'save-led.txt'
    script.write_text(commands, encoding='utf-8')
    result = subprocess.run([str(host), str(script), str(folder)], capture_output=True, text=True)
    print(result.stdout)
    if result.returncode:
        print(result.stderr)
        raise SystemExit(result.returncode)
    leds = re.findall(r'leds: buttons(.*?) \|', result.stdout)
    assert len(leds) == 6, leds
    assert ['SAVE' in line.split() for line in leds] == [False, False, False, True, False, False], leds
print('PASS: K2 changes remain unsaved; SAVE pulses only on press, saves, and stays off after reboot.')

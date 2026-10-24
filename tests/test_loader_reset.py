"""Run reset branches from shipped source archives with stub hardware.

CC must name a host C compiler (or wrapper). This verifies the reset selection,
not physical P33/USB behavior on an FM-1.
"""
from pathlib import Path
from zipfile import ZipFile
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
cc = os.environ.get('CC', 'cc')
for archive, expected in [('editions-source-0.8.0.zip', 1), ('groovebox-source-0.8.3.zip', 2)]:
    with ZipFile(root / 'downloads' / archive) as z:
        name = next(n for n in z.namelist() if n.endswith('/bsp/loader/loader.c'))
        source = z.read(name).decode().replace('\r', '')
    branch = source.split('if (ldr_session() == 0) {', 1)[1].split('\n            }', 1)[0]
    harness = '''
static int stage=0,kind=0;
static void fm1_delay_ms(unsigned ms){if((stage==0&&ms==200)||(stage==2&&ms==30))stage++;else stage=-100;}
static void usb_detach(void){if(stage==1)stage++;else stage=-100;}
static void fm1_core_reset(void){kind=1;}
static void fm1_reboot(void){kind=2;}
int main(void){
''' + branch + f'\nreturn stage==3&&kind=={expected}?0:1;}}\n'
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / 'reset.c'
        exe = Path(tmp) / ('reset.exe' if os.name == 'nt' else 'reset')
        src.write_text(harness, encoding='utf-8')
        subprocess.run([cc, str(src), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
    print(f'PASS {archive}: USB detach and settle, reset kind {expected} (1 CPU, 2 full chip)')

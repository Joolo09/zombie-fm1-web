"""Original Zombie track-six drum mechanics; build only, never flash automatically."""
import importlib.util
from pathlib import Path
p=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('edbuild',p/'build-editions.py')
b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
b.OUT=p/'output/editions-0.13.7';b.VERSION='0.13.7'
old_script=b.script
def keyboard_script(kind):
 path=old_script(kind)
 if kind=='groovebox':path.write_text(path.read_text(encoding='utf-8').replace('turn K1 1\nexpect chord1','turn K2 1\nexpect chord1'),encoding='utf-8')
 return path
b.script=keyboard_script
if __name__=='__main__':
 import sys
 if '--kind' not in sys.argv:sys.argv+=['--kind','groovebox']
 b.main()

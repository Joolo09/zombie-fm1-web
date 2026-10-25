"""Original Zombie track-six drum mechanics; build only, never flash automatically."""
import importlib.util
from pathlib import Path
p=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('edbuild',p/'build-editions.py')
b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
b.OUT=p/'output/editions-0.10.0';b.VERSION='0.10.0'
if __name__=='__main__':
 import sys
 if '--kind' not in sys.argv:sys.argv+=['--kind','groovebox']
 b.main()

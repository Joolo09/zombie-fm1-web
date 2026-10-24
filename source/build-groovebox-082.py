"""Groovebox 0.8.2 only; no old firmware overwrite or automatic flashing."""
import importlib.util
from pathlib import Path
p=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('edbuild',p/'build-editions.py')
b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
b.OUT=p/'output/editions-0.8.2';b.VERSION='0.8.2';b.PLATFORM=p/'platform-081'
if __name__=='__main__':
 import sys
 if '--kind' not in sys.argv:sys.argv+=['--kind','groovebox']
 b.main()

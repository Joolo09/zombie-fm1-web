"""Build Groovebox 0.8.1 only. Preserve Sampler 0.8.0 and all old downloads."""
import importlib.util
from pathlib import Path
p=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('edbuild',p/'build-editions.py')
b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
b.OUT=p/'output/editions-0.8.1';b.VERSION='0.8.1';b.PLATFORM=p/'platform-081'
if __name__=='__main__':
 import sys
 if '--kind' not in sys.argv:sys.argv+=['--kind','groovebox']
 b.main()

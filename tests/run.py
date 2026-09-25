#!/usr/bin/env python3
"""Run SDK host interface tests without modifying the SDK checkout."""
import argparse,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True);p.add_argument('tests',nargs='*');a=p.parse_args()
root=Path(__file__).resolve().parent;a.build_dir.mkdir(parents=True,exist_ok=True)
for src in sorted(root.glob('*.cpp')):
 if a.tests and src.stem not in a.tests:continue
 target=a.build_dir/src.stem
 cmd=[os.environ.get('CXX','clang++'),'-std=c++17','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-pthread','-I'+str(root/'stubs'),'-I'+str(root.parent/'include'),str(src),str(root.parent/'src/eeprom_runtime.cpp'),'-o',str(target)]
 subprocess.run(cmd,check=True);subprocess.run([str(target)],check=True);print('PASS',src.stem,flush=True)

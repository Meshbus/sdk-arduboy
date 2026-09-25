#!/usr/bin/env python3
from pathlib import Path
import subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
class DiagnosticTests(unittest.TestCase):
    def compile(self,source,compat=False):
        with tempfile.TemporaryDirectory() as temporary:
            p=Path(temporary);(p/'probe.cpp').write_text(source)
            cmd=['clang++','-std=c++17','-DMESHBUS_ARDUBOY_RUNTIME=1','-I'+str(ROOT/'tests/stubs'),'-I'+str(ROOT/'include'),'-c',str(p/'probe.cpp'),'-o',str(p/'probe.o')]
            if compat:cmd+=['-DMESHBUS_ARDUBOY_COMPATIBILITY=1']
            return subprocess.run(cmd,capture_output=True,text=True)
    def test_gpio_requires_explicit_compatibility(self):
        source='#include <Arduino.h>\nvoid setup(){pinMode(1,1);}\n'
        run=self.compile(source);self.assertNotEqual(run.returncode,0);self.assertIn('GPIO',run.stderr)
        run=self.compile(source,True);self.assertEqual(run.returncode,0,run.stderr);self.assertIn('compatibility',run.stderr)
    def test_spi_is_not_loopback_hardware(self):
        run=self.compile('#include <SPI.h>\nint read(){return SPI.transfer(42);}')
        self.assertNotEqual(run.returncode,0);self.assertIn('SPI',run.stderr)
    def test_unused_unsupported_interface_does_not_poison_include(self):
        run=self.compile('#include <SPI.h>\n#include <Arduboy2.h>\nvoid setup(){Arduboy2 b;b.clear();b.display();}')
        self.assertEqual(run.returncode,0,run.stderr)
if __name__=='__main__':unittest.main()

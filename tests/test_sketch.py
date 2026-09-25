#!/usr/bin/env python3
"""Behavior tests for the deliberately bounded Sketch preprocessor."""
from pathlib import Path
import subprocess, sys, tempfile, unittest
TOOL = Path(__file__).resolve().parents[1] / 'tools/sketch.py'
class SketchTests(unittest.TestCase):
    def generate(self, files):
        with tempfile.TemporaryDirectory(prefix='arduboy sketch ') as temporary:
            root = Path(temporary); sketch = root/'Game'; sketch.mkdir()
            for name, text in files.items(): (sketch/name).write_text(text)
            output = root/'build/Game.cpp'
            run = subprocess.run([sys.executable, str(TOOL), '--main', str(sketch/'Game.ino'), '--output', str(output)], text=True, capture_output=True)
            return run, output.read_text() if output.exists() else ''
    def test_order_prototypes_lines_and_literals(self):
        run, text = self.generate({'Game.ino':'const char *s = R"x(void fake() { })x";\nvoid setup(){ later(); }\nvoid loop(){}\n',
                                  'z.ino':'void last() {}\n', 'a.ino':'void later() {}\n'})
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertIn('#include <Arduino.h>', text)
        self.assertIn('void later();', text)
        self.assertLess(text.index('void later();'), text.index('void setup(){'))
        self.assertLess(text.index('a.ino"'), text.index('z.ino"'))
        self.assertIn('#line 2 ', text)
        self.assertNotIn('void fake();', text)
    def test_existing_prototype_not_duplicated(self):
        run, text = self.generate({'Game.ino':'void later();\nvoid setup(){later();}\nvoid later(){}\nvoid loop(){}'})
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertEqual(text.count('void later();'), 1)
    def test_complex_declarator_rejected(self):
        run, _ = self.generate({'Game.ino':'int (*factory())(int) { return nullptr; }\nvoid setup(){}\nvoid loop(){}'})
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('unsupported', run.stderr)
    def test_template_rejected(self):
        run, _ = self.generate({'Game.ino':'template<class T> T f(T x){return x;}\nvoid setup(){}\nvoid loop(){}'})
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('unsupported', run.stderr)
    def test_conditional_definition_rejected(self):
        run, _ = self.generate({'Game.ino':'#if FEATURE\nvoid f(){}\n#endif\nvoid setup(){}\nvoid loop(){}'})
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('conditional', run.stderr)
    def test_unterminated_comment_rejected(self):
        run, _ = self.generate({'Game.ino':'/* bad'})
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('unterminated', run.stderr)
if __name__ == '__main__': unittest.main()

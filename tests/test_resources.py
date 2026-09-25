#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import hashlib
import zlib

ROOT = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'tools' / (name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


class ResourceTests(unittest.TestCase):
    def test_generation_and_install_integrity(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); source = root / 'source'; source.write_bytes(bytes(range(256)) * 3)
            resources = module('resources'); installer = module('install')
            result = resources.generate(source, root, 'fixture', 7, '/extra/apps/arduboy_test')
            self.assertEqual(result['payload_crc32'], zlib.crc32(source.read_bytes()))
            (root / 'fixture.mba').write_bytes(b'mba-test')
            files = [dict(name='fixture.abr', destination=result['destination'],
                          length=result['length'], sha256=result['sha256']),
                     dict(name='fixture.mba', destination='/extra/apps/arduboy_test/fixture.mba',
                          length=8, sha256=hashlib.sha256(b'mba-test').hexdigest())]
            manifest = dict(schema=1, id='fixture', files=files)
            (root / 'install.json').write_text(json.dumps(manifest))
            records = installer.verify(root)
            self.assertEqual(installer.required_capacity(records), 16384)
            (root / 'fixture.abr').write_bytes(b'bad')
            with self.assertRaisesRegex(ValueError, 'content mismatch'): installer.verify(root)
            for directory in ['/extra/apps/../saves', '/extra/apps//test', '/extra/saves', '/extra/apps/test;cmd']:
                with self.assertRaises(ValueError): resources.generate(source, root, 'fixture', 7, directory)


if __name__ == '__main__': unittest.main()

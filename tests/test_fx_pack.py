#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import importlib.util
from pathlib import Path
import random
import struct
import unittest
spec=importlib.util.spec_from_file_location('fx_pack',Path(__file__).resolve().parents[1]/'tools/fx_pack.py')
fx=importlib.util.module_from_spec(spec);spec.loader.exec_module(fx)

class PackingTests(unittest.TestCase):
    def test_independent_blocks_round_trip(self):
        rng=random.Random(18)
        for size in (1,255,256,257,4097):
            for raw in (bytes([42])*size, bytes(rng.randrange(256) for _ in range(size))):
                packed=fx.pack(raw,0xfebe);count=struct.unpack_from('<I',packed,16)[0]
                offsets=struct.unpack_from('<'+'I'*(count+1),packed,20)
                restored=b''.join(fx.decode(packed[offsets[i]:offsets[i+1]]) for i in range(count))
                self.assertEqual(restored,raw)
    def test_malformed_runs(self):
        for raw in (bytes([128]),bytes([128,0]),bytes([127,1])):
            with self.assertRaises(ValueError):fx.decode(raw)
if __name__=='__main__':unittest.main()

#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path
import sys
first = bytes(0x55 if (i % 128) % 16 < 8 else 0xaa for i in range(1024))
data = first + bytes(v ^ 0xff for v in first) + bytes([12, 32, 56, 80, 104, 7, 11, 19])
path = Path(sys.argv[1])
if not path.exists() or path.read_bytes() != data: path.write_bytes(data)

#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Losslessly pack FX bytes in independently readable 256-byte blocks."""
import argparse
import hashlib
import json
from pathlib import Path
import struct


def encode(data):
    out = bytearray(); literals = bytearray(); offset = 0
    def flush():
        if literals:
            out.append(len(literals) - 1); out.extend(literals); literals.clear()
    while offset < len(data):
        best = 0; distance = 0
        for previous in range(max(0, offset - 256), offset):
            if data[previous] != data[offset]: continue
            length = 0
            while length < 130 and offset + length < len(data) and data[previous + length] == data[offset + length]:
                length += 1
            if length > best: best = length; distance = offset - previous
        if best >= 3:
            flush(); out.extend((128 | (best - 3), distance & 255)); offset += best
        else:
            literals.append(data[offset]); offset += 1
            if len(literals) == 128: flush()
    flush()
    return out


def decode(data):
    out = bytearray(); offset = 0
    while offset < len(data):
        control = data[offset]; offset += 1
        if control < 128:
            length = control + 1
            if offset + length > len(data): raise ValueError('short literal')
            out.extend(data[offset:offset + length]); offset += length
        else:
            if offset >= len(data): raise ValueError('short match')
            distance = data[offset] or 256; offset += 1
            if distance > len(out): raise ValueError('invalid match distance')
            for _ in range((control & 127) + 3): out.append(out[-distance])
        if len(out) > 256: raise ValueError('block overflow')
    return out


def pack(data, page):
    if not data or len(data) > 0x1000000 or not 0 <= page <= 65535:
        raise ValueError('invalid FX data length or page')
    blocks = [encode(data[i:i + 256]) for i in range(0, len(data), 256)]
    restored = b''.join(decode(b) for b in blocks)
    if restored != data: raise ValueError('FX round-trip mismatch')
    first = 20 + 4 * (len(blocks) + 1)
    offsets = [first]
    for block in blocks: offsets.append(offsets[-1] + len(block))
    return (struct.pack('<4sHHIHHI', b'FXPK', 1, 256, len(data), page, 0, len(blocks)) +
            b''.join(struct.pack('<I', offset) for offset in offsets) + b''.join(blocks))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input', required=True, type=Path); p.add_argument('--output', required=True, type=Path)
    p.add_argument('--page', required=True, type=lambda value: int(value, 0)); a = p.parse_args()
    try:
        data = a.input.read_bytes(); output = pack(data, a.page)
        a.output.parent.mkdir(parents=True, exist_ok=True)
        if not a.output.exists() or a.output.read_bytes() != output: a.output.write_bytes(output)
        print(json.dumps({'format': 'FXPK1', 'logical_bytes': len(data), 'packed_bytes': len(output),
            'logical_sha256': hashlib.sha256(data).hexdigest(), 'packed_sha256': hashlib.sha256(output).hexdigest(),
            'round_trip_matches': True, 'page': a.page}))
    except (OSError, ValueError) as error: p.exit(2, f'FX packing failed: {error}\n')


if __name__ == '__main__': main()

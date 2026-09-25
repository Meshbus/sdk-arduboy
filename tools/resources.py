#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Generate a versioned read-only sidecar and its compiled runtime contract."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import struct
import zlib


def generate(source, output, identity, version, directory):
    if not re.fullmatch(r'[a-z][a-z0-9_-]{0,30}', identity):
        raise ValueError('resource identity must be an app identifier (31 characters maximum)')
    path = PurePosixPath(directory)
    if (not directory.startswith('/extra/apps/') or '..' in path.parts or
            str(path) != directory or not re.fullmatch(r'(?:/[A-Za-z0-9_-]+)+', directory)):
        raise ValueError('resource directory must be a canonical directory below /extra/apps/')
    data = source.read_bytes()
    if not data or len(data) > 0x7fffffff - 64 or not 0 < version <= 0xffffffff:
        raise ValueError('invalid resource size or version')
    checksum = zlib.crc32(data)
    header = bytearray(64)
    struct.pack_into('<4sHHIIII', header, 0, b'MARB', 1, 64, version, len(data), checksum, 0)
    header[24:24 + len(identity)] = identity.encode('ascii')
    struct.pack_into('<I', header, 60, zlib.crc32(header[:60]))
    sidecar = bytes(header) + data
    name = identity + '.abr'
    spec = {'schema': 1, 'identity': identity, 'version': version, 'directory': directory,
            'file': name, 'destination': directory + '/' + name, 'length': len(sidecar),
            'sha256': hashlib.sha256(sidecar).hexdigest(), 'payload_length': len(data),
            'payload_crc32': checksum}
    output.mkdir(parents=True, exist_ok=True)
    contents = {
        name: sidecar,
        'arduboy-resource.hpp': ('/* Generated; do not edit. */\n'
            '#include <meshbus_arduboy/resources.hpp>\n'
            'inline constexpr meshbus::arduboy::ResourceSpec arduboy_resource = {'
            f'"{spec["destination"]}","{identity}",{version}U,{len(data)}U,{checksum}U' + '};\n').encode(),
        'arduboy-resources.json': (json.dumps(spec, indent=2) + '\n').encode(),
    }
    for name, content in contents.items():
        target = output / name
        if not target.exists() or target.read_bytes() != content:
            target.write_bytes(content)
    return spec


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    p.add_argument('--identity', required=True)
    p.add_argument('--version', required=True, type=int)
    p.add_argument('--directory', required=True)
    a = p.parse_args()
    try:
        print(json.dumps(generate(a.source, a.output, a.identity, a.version, a.directory)))
    except (ValueError, OSError) as error:
        p.exit(2, f'resource bundle: {error}\n')


if __name__ == '__main__':
    main()

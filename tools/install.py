#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Verify and install a new resource collection; never replace existing files."""
import argparse
import asyncio
import hashlib
import json
from pathlib import Path, PurePosixPath
import subprocess
import re


def verify(root):
    manifest = json.loads((root / 'install.json').read_text())
    if manifest.get('schema') != 1 or len(manifest.get('files', [])) != 2:
        raise ValueError('unsupported install manifest')
    seen = set()
    records = []
    for item in manifest['files']:
        name = item['name']; destination = item['destination']
        path = PurePosixPath(destination)
        if (not re.fullmatch(r'/[A-Za-z0-9_./-]+', destination) or
                Path(name).name != name or name in seen or name in ('.', '..') or
                not destination.startswith('/extra/apps/') or str(path) != destination or
                any(p in ('.', '..') for p in path.parts) or path.name != name):
            raise ValueError('invalid or duplicate install path')
        seen.add(name)
        data = (root / name).read_bytes()
        if len(data) != item['length'] or hashlib.sha256(data).hexdigest() != item['sha256']:
            raise ValueError('collection content mismatch: ' + name)
        records.append((item, data))
    identity = manifest['id']
    if seen != {identity + '.mba', identity + '.abr'}:
        raise ValueError('collection needs one MBA and its sidecar')
    if len({str(PurePosixPath(row[0]['destination']).parent) for row in records}) != 1:
        raise ValueError('collection paths must share a directory')
    records.sort(key=lambda row: row[0]['name'].endswith('.mba'))  # Publish entry last.
    return records


def required_capacity(records):
    # Fixed legacy estimate: 4 KiB blocks plus two metadata blocks.
    # Validate for the target filesystem; this tool does not discover its geometry.
    return sum(((len(data) + 4095) // 4096) * 4096 for _, data in records) + 8192


async def install(records, port):
    from smpclient import SMPClient
    from smpclient.transport.serial import SMPSerialTransport, BufferSize
    async with SMPClient(SMPSerialTransport(BufferSize(buf_size=256)), port, timeout_s=5) as client:
        for item, data in records:
            async for _ in client.upload_file(data, item['destination']):
                pass
            actual = await client.download_file(item['destination'])
            if actual != data:
                raise RuntimeError('device readback mismatch: ' + item['destination'])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('collection', type=Path)
    p.add_argument('--meshbus', type=Path, required=True)
    p.add_argument('--port', required=True)
    p.add_argument('--dry-run', action='store_true')
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()

    def command(text, allowed=()):
        run = subprocess.run([str(a.meshbus.resolve()), 'connect', '-p', a.port,
                              '--json', '--yes', '-c', text], capture_output=True, text=True)
        reply = json.loads(run.stdout)
        if not reply.get('ok') and reply.get('error', {}).get('rc') not in allowed:
            raise RuntimeError(reply)
        return reply

    try:
        records = verify(a.collection)
        status = command('fs status extra')['result']['status']
        required = required_capacity(records)
        if not status['mounted'] or not status['has_capacity'] or status['free_bytes'] < required:
            raise ValueError(f'insufficient verified capacity: need {required}, status={status}')
        for item, _ in records:
            reply = command('fs stat ' + item['destination'], allowed=(-2,))
            if reply.get('ok'):
                raise ValueError('refusing to replace existing file: ' + item['destination'])
        report = {'schema': 1, 'capacity': status, 'required_bytes': required,
                  'files': [item for item, _ in records], 'dry_run': a.dry_run}
        if not a.dry_run:
            directory = PurePosixPath(records[0][0]['destination']).parent
            for parent in reversed([directory, *directory.parents]):
                if str(parent).startswith('/extra/apps/'):
                    command('fs mkdir ' + str(parent), allowed=(-17,))
            asyncio.run(install(records, a.port))
            report['readback_matches'] = True
        a.report.parent.mkdir(parents=True, exist_ok=True)
        a.report.write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report))
    except (ValueError, OSError, RuntimeError) as error:
        p.exit(2, f'installation failed: {error}\n')


if __name__ == '__main__':
    main()

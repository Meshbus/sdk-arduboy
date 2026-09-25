#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Check local publication inputs without network, EDK, device, or dependencies."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]


def source_files():
    try:
        result = subprocess.run(
            ['git', '-C', str(ROOT), 'ls-files', '--cached', '--others',
             '--exclude-standard', '-z'], capture_output=True, check=False)
    except FileNotFoundError:
        result = subprocess.CompletedProcess([], 1, stdout=b'')
    if result.returncode == 0:
        return {ROOT / p.decode() for p in result.stdout.split(b'\0')
                if p and (ROOT / p.decode()).is_file()}
    # GitHub source archives contain only source files, without .git metadata.
    return {p for p in ROOT.rglob('*') if p.is_file() and not any(
        part in ('.git', 'build', '__pycache__') for part in p.relative_to(ROOT).parts)}


def font_bytes(path, name):
    body = path.read_text().split(name, 1)[1].split('{', 1)[1].split('}', 1)[0]
    body = re.sub(r'//[^\n]*|/\*.*?\*/', '', body, flags=re.S)
    return bytes(int(value, 16) for value in re.findall(r'0x([0-9a-fA-F]+)', body))


def main():
    files = source_files()
    errors = []

    def require(condition, message):
        if not condition:
            errors.append(message)

    for path in sorted(files):
        if path.suffix != '.md' or 'upstream' in path.relative_to(ROOT).parts:
            continue  # Keep vendored README links unchanged; UPSTREAM.md explains them.
        if not path.exists():
            require(False, f'missing source file: {path.relative_to(ROOT)}')
            continue
        for number, line in enumerate(path.read_text().splitlines(), 1):
            for link in re.findall(r'\]\(([^)]+)\)', line):
                if re.match(r'[a-zA-Z][a-zA-Z0-9+.-]*:', link) or link.startswith('#'):
                    continue
                target = unquote(link.split('#', 1)[0])
                require((path.parent / target).exists(),
                        f'{path.relative_to(ROOT)}:{number}: missing link {link}')

    for name in ('bounce', 'isojourn'):
        directory = ROOT / 'examples' / name / 'upstream'
        lock = json.loads((directory.parent / 'upstream.lock.json').read_text())['game']
        declared = {directory / row['path'] for row in lock['files']}
        actual = {p for p in files if directory in p.parents}
        require(declared == actual, f'{name}: lock inventory differs from source inventory')
        for row in lock['files']:
            path = directory / row['path']
            if not path.is_file():
                require(False, f'{name}: missing {row["path"]}')
                continue
            data = path.read_bytes()
            require(len(data) == row['size'] and hashlib.sha256(data).hexdigest() == row['sha256'],
                    f'{name}: size/hash mismatch: {row["path"]}')

    lock = json.loads((ROOT / 'upstream.lock.json').read_text())
    notices = lock['local_notices']
    require({row['identifier'] for row in notices['files']} ==
            {'BSD-3-Clause', 'BSD-2-Clause', 'Apache-2.0'},
            'license file inventory mismatch')
    require(len(notices['files']) == 3, 'duplicate license file record')
    for entry in notices['files']:
        identifier = entry['identifier']
        expected = f'LICENSES/{identifier}.txt'
        require(entry['path'] == expected, f'unexpected license path: {identifier}')
        path = ROOT / expected
        require(path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest() == entry['sha256'],
                f'missing or changed license file: {identifier}')
    font = lock['portions'][0]
    data = font_bytes(ROOT / font['local'], 'meshbus_arduboy_font5x7[]')
    require(hashlib.sha256(data).hexdigest() == font['data_sha256'], 'Arduboy2 font hash mismatch')
    catalog = json.loads((ROOT / 'capabilities.json').read_text())
    header = (ROOT / 'include/meshbus_arduboy/capabilities.hpp').read_text()
    require(f'"{catalog["sdk_version"]}"' in header, 'SDK version differs from header')
    require(re.search(r'#define MESHBUS_ARDUBOY_SEMANTIC_REVISION\s+'
                      + str(catalog['semantic_revision']) + r'\b', header),
            'semantic revision differs from header')
    require((ROOT / catalog['evidence_document']).is_file(), 'public evidence document missing')

    for name in ('LICENSE', 'LICENSING.md'):
        path = ROOT / name
        require(path.is_file() and path.stat().st_size > 100, f'missing/empty notice: {name}')

    require((ROOT / 'LICENSE').is_file() and
            'Copyright (c) 2026 FoBE Studio' in (ROOT / 'LICENSE').read_text().splitlines(),
            'original SDK copyright must name FoBE Studio')

    root_license = (ROOT / 'LICENSE').read_text()
    apache = (ROOT / 'LICENSES/Apache-2.0.txt').read_text()
    require(root_license == 'Copyright (c) 2026 FoBE Studio\n\n' + apache,
            'original SDK root license must contain the full Apache-2.0 text')
    for path in sorted(files):
        relative = path.relative_to(ROOT)
        if 'upstream' in relative.parts or relative.parts[0] == 'LICENSES':
            continue
        try:
            text = path.read_text()
        except UnicodeDecodeError:
            continue
        require(not re.search(r'SPDX-License-Identifier: [^\n]*\bMIT\b', text),
                f'{relative}: original SDK retains an MIT SPDX declaration')

    examples = sorted((ROOT / 'examples').glob('*/CMakeLists.txt'))
    for cmake in examples:
        for name in ('README.md', 'llext.yaml', 'toolchain.cmake'):
            require((cmake.parent / name).is_file(), f'{cmake.parent.name}: missing {name}')
        # Catch stale SDK source dependencies such as the removed runtime_audio.inc.
        for target in re.findall(r'\$\{MESHBUS_ARDUBOY_SDK_DIR\}/([^\s)";]+)', cmake.read_text()):
            require((ROOT / target).exists(), f'{cmake.parent.name}: missing SDK input {target}')

    if errors:
        for error in errors:
            print('FAIL:', error, file=sys.stderr)
        return 1
    print(f'PASS publication inputs: {len(examples)} examples, game locks, font, links, notices, versions')
    return 0


if __name__ == '__main__':
    sys.exit(main())

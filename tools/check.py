#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Run host contract tests and build one SDK example (Bounce by default)."""
import argparse
import json
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('edk', 'toolchain', 'meshbus', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    root = Path(__file__).resolve().parents[1]
    examples = sorted(path.name for path in (root / 'examples').iterdir()
                      if (path / 'CMakeLists.txt').is_file())
    parser.add_argument('--example', choices=examples, default='bounce',
                        help='example to package (default: bounce)')
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    commands = [
        [sys.executable, str(root / 'tests/run.py'), '--build-dir', str(output / 'native')],
        [sys.executable, '-m', 'unittest', 'discover', '-s', str(root / 'tests'), '-p', 'test_*.py'],
        [str(args.meshbus.resolve()), 'llext', '--llext-sdk', str(args.edk.resolve()),
         '--zephyr-sdk', str(args.toolchain.resolve()), '-o', str(output / 'packages'),
         str(root / 'examples' / args.example), '--', '-DMESHBUS_ARDUBOY_SDK_DIR=' + str(root)],
    ]
    results = []
    for index, command in enumerate(commands):
        log = output / f'check-{index}.log'
        with log.open('w') as stream:
            code = subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT).returncode
        results.append({'command': command, 'exit_code': code, 'log': str(log)})
        print(f'{index}: {"PASS" if code == 0 else "FAIL"} {log}', flush=True)
    (output / 'results.json').write_text(json.dumps({
        'checks': results, 'device_validation': 'not performed by this tool',
        'build_identity': f'packages/{args.example}.build.json',
    }, indent=2) + '\n')
    return int(any(row['exit_code'] for row in results))


if __name__ == '__main__':
    sys.exit(main())

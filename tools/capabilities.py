#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Validate declared SDK capabilities and record build-input identities."""
import argparse, hashlib, json, subprocess
from pathlib import Path

def revision(root):
    p = subprocess.run(['git','-C',str(root),'rev-parse','HEAD'],capture_output=True,text=True)
    return p.stdout.strip() if p.returncode == 0 else None

def digest(root, paths=None):
    result = hashlib.sha256()
    files = sorted(paths if paths is not None else (p for p in root.rglob('*') if p.is_file()))
    for path in files:
        relative = path.relative_to(root)
        if any(part in ('.git','build','__pycache__') for part in relative.parts): continue
        result.update(str(relative).encode()); result.update(b'\0'); result.update(path.read_bytes())
    return result.hexdigest()

def main():
    p = argparse.ArgumentParser(description=__doc__)
    for key in ('sdk','source','edk','output'): p.add_argument('--'+key,type=Path,required=True)
    p.add_argument('--runtime',action='store_true');p.add_argument('--compatibility',action='store_true')
    p.add_argument('--resources',action='store_true')
    p.add_argument('--fx',action='store_true')
    p.add_argument('--requires',nargs='*',default=[])
    a = p.parse_args()
    catalog = json.loads((a.sdk/'capabilities.json').read_text())
    edk = json.loads((a.edk/'edk-release.json').read_text())
    actual = {name: dict(value) for name,value in catalog['capabilities'].items()}
    for value in actual.values():
        if any(symbol not in (edk.get('exported-symbols') or []) for symbol in value.get('requires_host_symbols', [])): value['status'] = 'unavailable'
        if value.get('requires_runtime') and not a.runtime: value['status'] = 'degraded'
        if value.get('requires_resources') and not a.resources: value['status'] = 'unavailable'
        if value.get('requires_fx') and not a.fx: value['status'] = 'unavailable'
    requested = a.requires or (['clock','input','display','graphics','save','audio'] if a.runtime else [])
    if a.resources and 'resources' not in requested: requested.append('resources')
    if a.fx and 'fx' not in requested: requested.append('fx')
    missing = [name for name in requested if name not in actual or actual[name]['status'] not in ('implemented','bounded')]
    if missing: p.exit(2,'sdk-arduboy missing requested capabilities: '+', '.join(missing)+'\n')
    if not a.compatibility and not isinstance(edk.get('exported-symbols'),list):
        p.exit(2,'sdk-arduboy strict mode requires an EDK with exported-symbols inventory; regenerate the EDK\n')
    paths = [a.sdk/'capabilities.json']
    for folder in ('include','src','cmake','tools'):
        paths.extend(f for f in (a.sdk/folder).rglob('*') if f.is_file())
    report = {'schema':1,'sdk_version':catalog['sdk_version'],'semantic_revision':catalog['semantic_revision'],
              'mode':'compatibility' if a.compatibility else 'strict','requested':requested,'actual':actual,
              'missing_capabilities':missing,'sdk_identity':{'revision':revision(a.sdk),'sha256':digest(a.sdk,paths)},
              'source_identity':{'revision':revision(a.source),'sha256':digest(a.source)},
              'edk_identity':edk.get('edk'),'host':edk.get('host'),'target':edk.get('target')}
    if a.compatibility: report['degradations'] = catalog['legacy_degradations']
    text = json.dumps(report,indent=2)+'\n';a.output.parent.mkdir(parents=True,exist_ok=True)
    if not a.output.exists() or a.output.read_text()!=text:a.output.write_text(text)
    print('sdk-arduboy '+catalog['sdk_version']+' '+report['mode']+' requested='+','.join(requested)+
          ' available='+','.join(name for name,value in actual.items() if value['status'] in ('implemented','bounded')))
if __name__=='__main__':main()

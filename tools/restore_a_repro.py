#!/usr/bin/env python3
"""Verify and restore A's snapshot from the original archive plus its delta."""
import argparse
import hashlib
from pathlib import Path, PurePosixPath
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / 'original/eagleeye_z035.zip'
DELTA = ROOT / 'original/a_repro_delta_20261027.zip'

def safe_entries(archive):
    result = {}
    for item in archive.infolist():
        if item.is_dir():
            continue
        name = item.filename
        p = PurePosixPath(name)
        if p.is_absolute() or '..' in p.parts or '\\' in name or not name.startswith('eagleeye_z035/'):
            raise ValueError('unsafe archive path: ' + name)
        if name in result or (item.external_attr >> 16) & 0o170000 == 0o120000:
            raise ValueError('duplicate or symlink entry: ' + name)
        result[name] = archive.read(item)
    return result

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, help='new empty directory for restored snapshot')
    args = parser.parse_args()
    with zipfile.ZipFile(BASE) as a, zipfile.ZipFile(DELTA) as b:
        files = safe_entries(a)
        files.update(safe_entries(b))
    manifest = files['eagleeye_z035/SHA256SUMS.txt'].decode('utf-8-sig')
    checked = set()
    for line in manifest.splitlines():
        if not line.strip():
            continue
        m = re.fullmatch(r'([0-9a-fA-F]{64})\s+\*?(.+)', line)
        if not m:
            raise ValueError('invalid manifest line')
        name = 'eagleeye_z035/' + m.group(2)
        if name not in files or hashlib.sha256(files[name]).hexdigest() != m.group(1).lower():
            raise ValueError('missing or bad hash: ' + name)
        checked.add(name)
    if checked != set(files) - {'eagleeye_z035/SHA256SUMS.txt'}:
        raise ValueError('manifest does not match snapshot')
    print('PASS verified files:', len(checked), 'plus manifest')
    if args.output:
        dest = args.output.resolve()
        if dest.exists():
            raise ValueError('output directory already exists: ' + str(dest))
        dest.mkdir(parents=True)
        for name, data in files.items():
            p = dest.joinpath(*PurePosixPath(name).parts)
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(data)
        print('RESTORED', dest / 'eagleeye_z035')

if __name__ == '__main__':
    main()

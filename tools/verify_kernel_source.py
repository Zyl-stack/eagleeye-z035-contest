#!/usr/bin/env python3
"""Check the pinned LFS kernel archive, not the identity of a board image."""
import hashlib
from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE = ROOT / 'reference/linux_z035/source/linux_xlnx_19984dd.tar.gz'
EXPECTED = '139cbe4ad3b6e21f243f21e0fd272640b295d50737d666c055a54784f0e954a9'
REQUIRED = {'Makefile', 'COPYING', 'drivers/base/Makefile',
            'drivers/fpga/fpga-mgr.c', 'arch/arm/Kconfig'}

def main():
    if not ARCHIVE.is_file():
        raise SystemExit('MISSING: use git lfs pull to obtain the kernel archive')
    h = hashlib.sha256()
    with ARCHIVE.open('rb') as src:
        for chunk in iter(lambda: src.read(4 * 1024 * 1024), b''):
            h.update(chunk)
    if h.hexdigest() != EXPECTED:
        raise SystemExit('HASH_MISMATCH: check Git LFS hydration and the original checksum')
    seen = set()
    count = 0
    makefile = ''
    with tarfile.open(ARCHIVE, 'r|gz') as archive:
        for entry in archive:
            count += 1
            if entry.name in REQUIRED:
                seen.add(entry.name)
            if entry.name == 'Makefile':
                makefile = archive.extractfile(entry).read(1024).decode('utf-8')
    if seen != REQUIRED or count != 78975 or 'SUBLEVEL = 36' not in makefile:
        raise SystemExit('ARCHIVE_CONTENT_MISMATCH')
    print('KERNEL_ARCHIVE_PASS commit=19984dd147fa7fbb7cb14b17400263ad0925c189 '
          f'sha256={h.hexdigest()} entries={count} version=5.15.36')
    print('Scope: archive integrity only; no claim of image identity or board validation.')

if __name__ == '__main__':
    main()

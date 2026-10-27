#!/usr/bin/env python3
"""Check the generated XSA contains the identical BIT and a system HWH."""
import argparse
import hashlib
from pathlib import Path
import zipfile


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build_dir', type=Path)
    args = parser.parse_args()
    bit = args.build_dir / 'vivado_work/eagleeye_z035.runs/impl_1/z100_top.bit'
    xsa = args.build_dir / 'eagleeye_z035.xsa'
    try:
        bit_data = bit.read_bytes()
        with zipfile.ZipFile(xsa) as archive:
            bits = [n for n in archive.namelist() if n.lower().endswith('.bit')]
            hwhs = [n for n in archive.namelist() if n.lower().split('/')[-1] == 'system.hwh']
            if len(bits) != 1 or len(hwhs) != 1:
                raise ValueError('XSA must contain exactly one BIT and one system.hwh')
            if archive.read(bits[0]) != bit_data:
                raise ValueError('XSA BIT does not match standalone BIT')
            hwh_data = archive.read(hwhs[0])
            if not hwh_data:
                raise ValueError('system.hwh is empty')
        print('PASS XSA embedded BIT equals standalone BIT; system.hwh present')
        print('BIT_SHA256', digest(bit_data))
        print('XSA_SHA256', digest(xsa.read_bytes()))
        print('HWH_SHA256', digest(hwh_data))
        print('Scope: file identity only; timing, CDC, hardware and board behavior require separate checks.')
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        parser.exit(1, 'FAIL: ' + str(exc) + '\n')

if __name__ == '__main__':
    main()
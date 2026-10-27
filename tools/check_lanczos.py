#!/usr/bin/env python3
"""Compare current Eyes Lanczos RTL with its archived historical hash list."""
import hashlib
from pathlib import Path
import zipfile

root = Path(__file__).resolve().parents[1]
eyes = root / 'src/vivado/eagleeye_7035_eyes.srcs/sources_1/imports/rtl'
hashes = root / 'sim/lanczos_legacy_rtl_sha256.txt'
archive = root / 'original/eagleeye_z035.zip'
count = 0
with zipfile.ZipFile(archive) as original:
    for line in hashes.read_text(encoding='ascii').splitlines():
        expected, rel = line.split(None, 1)
        rel = rel.strip()
        source = eyes / Path(rel).name.lower()
        archived = original.read('eagleeye_z035/skill/lanczos-2x-stream/' + rel)
        if not source.is_file() or any(hashlib.sha256(data).hexdigest() != expected.lower()
                                      for data in (source.read_bytes(), archived)):
            raise SystemExit(f'HASH_MISMATCH {source}')
        count += 1
print(f'LANCZOS_SOURCE_MATCH files={count}; pixel correctness NOT verified')
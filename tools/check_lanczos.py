#!/usr/bin/env python3
"""Check the seven original Lanczos RTL hashes in the lowercase mirror."""
import hashlib
from pathlib import Path

root = Path(__file__).resolve().parents[1]
skill = root / "skill" / "lanczos-2x-stream"
eyes = root / "src" / "vivado" / "eagleeye_7035_eyes.srcs" / "sources_1" / "imports" / "rtl"
count = 0
for line in (skill / "sha256sums.txt").read_text(encoding="ascii").splitlines():
    expected, rel = line.split(None, 1)
    for path in (skill / rel.strip().lower(), eyes / Path(rel.strip()).name.lower()):
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != expected.lower():
            raise SystemExit(f"HASH_MISMATCH {path}")
    count += 1
print(f"LANCZOS_SOURCE_MATCH files={count}; pixel correctness NOT verified")

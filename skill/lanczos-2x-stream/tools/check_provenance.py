#!/usr/bin/env python3
"""Check Lanczos RTL hashes; this is provenance, not a pixel-level test."""

import argparse
import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, help="optional verified Eyes RTL directory")
    args = parser.parse_args()
    failures = []
    checked = 0
    for line in (ROOT / "SHA256SUMS.txt").read_text(encoding="ascii").splitlines():
        expected, relative = line.split(None, 1)
        relative = relative.strip()
        paths = [ROOT / relative]
        if args.source_dir:
            paths.append(args.source_dir / Path(relative).name)
        for path in paths:
            if not path.is_file():
                failures.append(f"missing {path}")
                continue
            actual = hashlib.sha256(path.read_bytes()).hexdigest()
            if actual.lower() != expected.lower():
                failures.append(f"hash mismatch {path}")
            checked += 1
    for problem in failures:
        print(problem)
    print(f"LANCZOS_PROVENANCE checked={checked} failures={len(failures)}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())

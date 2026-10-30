#!/usr/bin/env python3
"""Hash an embedded XSA bitstream and compare with an external .bit file."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile


def sha256_stream(stream):
    h = hashlib.sha256()
    size = 0
    for chunk in iter(lambda: stream.read(1024*1024), b""):
        h.update(chunk)
        size += len(chunk)
    return h.hexdigest(), size


def verify(xsa, bit, member=None):
    with zipfile.ZipFile(xsa) as archive:
        bits = [e for e in archive.infolist() if not e.is_dir() and e.filename.lower().endswith(".bit")]
        selected = bits if member is None else [e for e in bits if e.filename == member]
        if len(selected) != 1:
            raise ValueError("select exactly one .bit using --member; candidates=" +
                             repr([e.filename for e in bits]))
        entry = selected[0]
        with archive.open(entry) as stream:
            embedded_hash, embedded_size = sha256_stream(stream)
    with Path(bit).open("rb") as stream:
        external_hash, external_size = sha256_stream(stream)
    if embedded_size == 0 or external_size == 0:
        raise ValueError("empty bitstream")
    return {"pass": embedded_hash == external_hash and embedded_size == external_size,
            "member": entry.filename, "embedded_sha256": embedded_hash,
            "external_sha256": external_hash, "embedded_bytes": embedded_size,
            "external_bytes": external_size,
            "scope": "file identity only; verify hardware/PS config and tool versions separately"}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("xsa", type=Path)
    p.add_argument("bitstream", type=Path)
    p.add_argument("--member")
    a = p.parse_args()
    try:
        result = verify(a.xsa, a.bitstream, a.member)
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print(json.dumps({"error": str(exc)}, ensure_ascii=False))
        return 2
    print(json.dumps(result, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

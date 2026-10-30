#!/usr/bin/env python3
"""Inspect ELF header only: ARM32 little endian EABI5 hard-float candidate."""
import argparse
import json
from pathlib import Path
import struct


def audit(path):
    with Path(path).open("rb") as f:
        data = f.read(64)
    if len(data) < 16 or data[:4] != b"\x7fELF":
        raise ValueError("not an ELF file")
    elf_class, endian, ident_version = data[4:7]
    if endian not in (1, 2) or elf_class not in (1, 2):
        raise ValueError("invalid ELF class or endian")
    fmt = "<" if endian == 1 else ">"
    required = 52 if elf_class == 1 else 64
    if len(data) < required:
        raise ValueError("truncated ELF header")
    machine = struct.unpack_from(fmt+"H", data, 18)[0]
    flags = struct.unpack_from(fmt+"I", data, 36 if elf_class == 1 else 48)[0]
    header_version = struct.unpack_from(fmt+"I", data, 20)[0]
    header_size = struct.unpack_from(fmt+"H", data, 40 if elf_class == 1 else 52)[0]
    checks = {"elf32": elf_class == 1, "little_endian": endian == 1,
              "em_arm": machine == 40, "elf_version": ident_version == header_version == 1,
              "header_size": header_size == required,
              "eabi5": (flags >> 24) == 5,
              "hard_float_flag": bool(flags & 0x400), "no_soft_float_flag": not bool(flags & 0x200)}
    return {"pass": all(checks.values()), "checks": checks, "machine": machine,
            "flags": hex(flags), "scope": "ELF header only; not CPU/NEON/library/runtime proof"}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("binary", type=Path)
    a = p.parse_args()
    try:
        result = audit(a.binary)
    except (OSError, ValueError, struct.error) as exc:
        print(json.dumps({"error": str(exc)}))
        return 2
    print(json.dumps(result, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

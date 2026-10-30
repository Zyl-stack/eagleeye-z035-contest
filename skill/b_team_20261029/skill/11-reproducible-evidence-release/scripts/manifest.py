#!/usr/bin/env python3
"""Create/check a portable file manifest. Keep the manifest outside its root."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath


def digest(path):
    h = hashlib.sha256()
    size = 0
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024*1024), b""):
            h.update(block)
            size += len(block)
    return {"bytes": size, "sha256": h.hexdigest()}


def scan(root):
    root = Path(root).resolve(strict=True)
    if not root.is_dir():
        raise ValueError("root must be a directory")
    result = {}
    for path in sorted(root.rglob("*")):
        if path.is_symlink() or path.resolve().is_relative_to(root) is False:
            raise ValueError("symlink or escaping path: " + str(path))
        if path.is_file():
            result[path.relative_to(root).as_posix()] = digest(path)
    if not result:
        raise ValueError("release root contains no files")
    return result


def safe_name(name):
    if not isinstance(name, str) or "\\" in name or ":" in name:
        return False
    p = PurePosixPath(name)
    return bool(name) and not p.is_absolute() and ".." not in p.parts and p.as_posix() == name and name != "."


def create(root, output):
    root, output = Path(root).resolve(strict=True), Path(output).resolve()
    if output.is_relative_to(root):
        raise ValueError("manifest must be outside release root to avoid self-reference")
    if output.exists():
        raise ValueError("manifest already exists; choose a new output path")
    result = {"schema_version": 1, "algorithm": "sha256", "files": scan(root)}
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("x", encoding="utf-8", newline="\n") as f:
        json.dump(result, f, ensure_ascii=False, indent=2)
        f.write("\n")
    return {"pass": True, "files": len(result["files"]), "manifest": str(output)}


def check(root, manifest):
    data = json.loads(Path(manifest).read_text(encoding="utf-8"))
    if data.get("schema_version") != 1 or data.get("algorithm") != "sha256":
        raise ValueError("unsupported manifest schema or algorithm")
    files = data.get("files")
    if not isinstance(files, dict) or not files:
        raise ValueError("manifest has no file mapping")
    for name, record in files.items():
        if not safe_name(name) or not isinstance(record, dict):
            raise ValueError("invalid manifest path/record: " + repr(name))
        hash_value = record.get("sha256")
        size = record.get("bytes")
        if (not isinstance(hash_value, str) or len(hash_value) != 64 or
                any(c not in "0123456789abcdef" for c in hash_value) or
                not isinstance(size, int) or isinstance(size, bool) or size < 0):
            raise ValueError("invalid size/hash: " + name)
    actual = scan(root)
    missing = sorted(set(files)-set(actual))
    extra = sorted(set(actual)-set(files))
    changed = sorted(n for n in set(files) & set(actual) if files[n] != actual[n])
    return {"pass": not (missing or extra or changed), "expected_files": len(files),
            "actual_files": len(actual), "missing": missing, "extra": extra, "changed": changed}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("mode", choices=("create", "check"))
    p.add_argument("root", type=Path)
    p.add_argument("manifest", type=Path)
    a = p.parse_args()
    try:
        result = create(a.root, a.manifest) if a.mode == "create" else check(a.root, a.manifest)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(json.dumps({"error": str(exc)}, ensure_ascii=False))
        return 2
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Strict standalone validator for EE_B_TARGET_STREAM_V1; no default source size."""
import argparse
import json
from pathlib import Path


class ContractError(ValueError):
    pass


def validate(text, *, legacy_size=None, max_targets=16):
    if max_targets <= 0:
        raise ValueError("max_targets must be positive")
    lines = [(i, s.strip()) for i, s in enumerate(text.splitlines(), 1)
             if s.strip() and not s.strip().startswith("#")]
    if not lines or lines[0][1] != "EE_B_TARGET_STREAM_V1":
        raise ContractError("missing or unsupported stream header")
    idx = 1
    size = legacy_size
    explicit = False
    def error(line, message):
        raise ContractError(f"line {line}: {message}")
    if idx < len(lines) and lines[idx][1].split(",")[0] == "STREAM_INFO":
        line, content = lines[idx]
        fields = content.split(",")
        if len(fields) != 3:
            error(line, "STREAM_INFO must have 3 fields")
        try:
            size = tuple(map(int, fields[1:]))
        except ValueError:
            error(line, "invalid source dimensions")
        explicit = True
        idx += 1
    if size is None:
        raise ContractError("source dimensions unknown: provide STREAM_INFO or verified --legacy-size W H")
    if len(size) != 2 or any(not isinstance(v, int) or not 0 < v <= 65535 for v in size):
        raise ContractError("source dimensions must be integers in [1,65535]")
    width, height = size
    frames = targets = held = recoveries = 0
    previous_frame = -1
    identity_type = {}
    previous_valid = {}
    while idx < len(lines):
        line, content = lines[idx]
        fields = content.split(",")
        if len(fields) != 3 or fields[0] != "FRAME":
            error(line, "expected FRAME,id,count")
        try:
            frame_id, count = map(int, fields[1:])
        except ValueError:
            error(line, "frame ID and count must be integers")
        if frame_id <= previous_frame:
            error(line, "frame IDs must be nonnegative and strictly increasing")
        if not 0 <= count <= max_targets:
            error(line, "target count outside configured budget")
        previous_frame = frame_id
        idx += 1
        seen = set()
        for _ in range(count):
            if idx >= len(lines):
                error(line, "truncated frame")
            target_line, content = lines[idx]
            v = content.split(",")
            if len(v) != 15 or v[0] != "TARGET":
                error(target_line, "TARGET must have 15 fields including record")
            kind = v[3]
            if kind not in {"PLAYER", "BALL", "REFEREE"}:
                error(target_line, "unsupported target type")
            try:
                fid, tid = int(v[1]), int(v[2])
                x, y, w, h, confidence, valid, vx, vy, velocity_valid, age, missed = map(int, v[4:])
            except ValueError:
                error(target_line, "all numeric TARGET fields must be integers")
            if fid != frame_id:
                error(target_line, "TARGET frame ID differs from FRAME")
            if not 1 <= tid < 65535 or tid in seen:
                error(target_line, "reserved/out-of-range/duplicate target ID")
            seen.add(tid)
            if tid in identity_type and identity_type[tid] != kind:
                error(target_line, "observed ID changed target type")
            identity_type[tid] = kind
            if not (w > 0 and h > 0 and x >= 0 and y >= 0 and x+w <= width and y+h <= height):
                error(target_line, "bbox outside declared source or nonpositive size")
            if not 0 <= confidence <= 100:
                error(target_line, "confidence must be 0..100")
            if valid not in (0, 1) or velocity_valid not in (0, 1):
                error(target_line, "valid flags must be 0 or 1")
            if age < 0 or missed < 0:
                error(target_line, "negative lifecycle counter")
            if not valid and velocity_valid:
                error(target_line, "held target cannot claim measured consecutive velocity")
            held += not valid
            recoveries += previous_valid.get(tid) == 0 and valid == 1
            previous_valid[tid] = valid
            targets += 1
            idx += 1
        if idx >= len(lines) or lines[idx][1] != "END_FRAME":
            error(line, "missing END_FRAME or target count mismatch")
        idx += 1
        frames += 1
    if not frames:
        raise ContractError("no complete frames")
    return {"pass": True, "source_size": list(size), "explicit_stream_info": explicit,
            "frames": frames, "targets": targets, "held_records": held,
            "observed_invalid_to_valid": recoveries,
            "scope": "syntax/range/observed ID type only; identity accuracy and timebase need source evidence"}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("stream", type=Path)
    p.add_argument("--legacy-size", nargs=2, type=int, metavar=("W", "H"))
    p.add_argument("--max-targets", type=int, default=16)
    a = p.parse_args()
    try:
        result = validate(a.stream.read_text(encoding="utf-8-sig"),
                          legacy_size=a.legacy_size, max_targets=a.max_targets)
    except ContractError as exc:
        print(json.dumps({"pass": False, "error": str(exc)}, ensure_ascii=False))
        return 1
    except (OSError, UnicodeError, ValueError) as exc:
        print(json.dumps({"error": str(exc)}, ensure_ascii=False))
        return 2
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

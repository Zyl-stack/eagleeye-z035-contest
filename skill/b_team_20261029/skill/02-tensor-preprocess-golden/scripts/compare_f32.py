#!/usr/bin/env python3
"""Compare complete little-endian raw FP32 tensors; standard library only."""
import argparse
import json
import math
from pathlib import Path
import struct


def compare(reference, actual, *, count=None, atol=1e-6, rtol=0.0, exact=False):
    if not math.isfinite(atol) or not math.isfinite(rtol) or atol < 0 or rtol < 0:
        raise ValueError("tolerances must be finite and nonnegative")
    if count is not None and count <= 0:
        raise ValueError("count must be positive")
    ref_bytes, got_bytes = Path(reference).read_bytes(), Path(actual).read_bytes()
    if not ref_bytes or len(ref_bytes) % 4 or len(got_bytes) % 4:
        raise ValueError("FP32 data must be nonempty and divisible by 4 bytes")
    if len(ref_bytes) != len(got_bytes):
        return {"pass": False, "reason": "byte_length_mismatch",
                "reference_bytes": len(ref_bytes), "actual_bytes": len(got_bytes)}
    n = len(ref_bytes) // 4
    if count is not None and n != count:
        return {"pass": False, "reason": "count_mismatch", "expected": count, "actual": n}
    refs = struct.iter_unpack("<f", ref_bytes)
    gots = struct.iter_unpack("<f", got_bytes)
    violations = nonfinite = bit_equal = 0
    first = None
    max_abs = square_sum = 0.0
    for i, ((ref,), (got,)) in enumerate(zip(refs, gots)):
        same_bits = ref_bytes[4*i:4*i+4] == got_bytes[4*i:4*i+4]
        bit_equal += same_bits
        if not math.isfinite(ref) or not math.isfinite(got):
            nonfinite += 1
            if first is None:
                first = {"index": i, "reason": "nonfinite",
                         "reference": repr(ref), "actual": repr(got)}
            continue
        delta = abs(ref-got)
        max_abs = max(max_abs, delta)
        square_sum += delta*delta
        failed = (not same_bits) if exact else (delta > atol + rtol*abs(ref))
        if failed:
            violations += 1
            if first is None:
                first = {"index": i, "reason": "difference", "reference": ref,
                         "actual": got, "absolute_error": delta}
    finite_count = n-nonfinite
    return {"pass": violations == 0 and nonfinite == 0, "count": n,
            "mode": "bit_exact" if exact else "tolerance", "atol": atol, "rtol": rtol,
            "bit_equal": bit_equal, "violations": violations, "nonfinite_pairs": nonfinite,
            "max_absolute_error": max_abs if finite_count else None,
            "rmse_finite_pairs": math.sqrt(square_sum/finite_count) if finite_count else None,
            "first_failure": first}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("reference", type=Path)
    p.add_argument("actual", type=Path)
    p.add_argument("--count", type=int)
    p.add_argument("--atol", type=float, default=1e-6)
    p.add_argument("--rtol", type=float, default=0.0)
    p.add_argument("--exact", action="store_true")
    a = p.parse_args()
    try:
        result = compare(a.reference, a.actual, count=a.count, atol=a.atol,
                         rtol=a.rtol, exact=a.exact)
    except (OSError, ValueError) as exc:
        print(json.dumps({"error": str(exc)}, ensure_ascii=False))
        return 2
    print(json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

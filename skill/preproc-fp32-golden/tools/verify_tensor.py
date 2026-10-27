#!/usr/bin/env python3
"""Compare little-endian FP32 NCHW tensors without external packages."""

import argparse
import json
import math
import struct
from pathlib import Path


def parse_shape(value):
    dims = [int(part) for part in value.lower().replace(",", "x").split("x")]
    if not dims or any(dim <= 0 for dim in dims):
        raise argparse.ArgumentTypeError("shape must contain positive dimensions")
    return dims


def compare(actual_path, golden_path, shape, atol, rtol, require_exact):
    count = math.prod(shape)
    expected_bytes = count * 4
    actual = Path(actual_path).read_bytes()
    golden = Path(golden_path).read_bytes()
    if len(actual) != expected_bytes or len(golden) != expected_bytes:
        raise ValueError(
            f"expected {expected_bytes} bytes; actual={len(actual)} golden={len(golden)}"
        )

    exact = 0
    over_tolerance = 0
    max_abs = 0.0
    sum_abs = 0.0
    for offset in range(0, expected_bytes, 4):
        a = struct.unpack_from("<f", actual, offset)[0]
        g = struct.unpack_from("<f", golden, offset)[0]
        if not math.isfinite(a) or not math.isfinite(g):
            raise ValueError(f"non-finite FP32 value at index {offset // 4}")
        exact += actual[offset : offset + 4] == golden[offset : offset + 4]
        delta = abs(a - g)
        max_abs = max(max_abs, delta)
        sum_abs += delta
        over_tolerance += delta > atol + rtol * abs(g)

    passed = over_tolerance == 0 and (not require_exact or exact == count)
    return {
        "shape": shape,
        "values": count,
        "exact": exact,
        "max_abs": max_abs,
        "mean_abs": sum_abs / count,
        "over_tolerance": over_tolerance,
        "passed": passed,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--actual", type=Path, required=True)
    parser.add_argument("--golden", type=Path, required=True)
    parser.add_argument("--shape", type=parse_shape, default=[3, 320, 320])
    parser.add_argument("--atol", type=float, default=1e-6)
    parser.add_argument("--rtol", type=float, default=0.0)
    parser.add_argument("--require-exact", action="store_true")
    args = parser.parse_args()
    if not math.isfinite(args.atol) or not math.isfinite(args.rtol) or min(args.atol, args.rtol) < 0:
        parser.error("tolerances must be finite and nonnegative")
    try:
        result = compare(
            args.actual, args.golden, args.shape, args.atol, args.rtol, args.require_exact
        )
    except (OSError, ValueError) as error:
        parser.exit(2, f"INVALID: {error}\n")
    print(json.dumps(result, sort_keys=True))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Summarize TIME records; no source or release identity is inferred."""
import argparse
import json
import re
from pathlib import Path
from statistics import mean

FIELDS = ("capture_ms", "preprocess_ms", "inference_ms", "postprocess_ms", "total_ms", "FPS")
PAIR = re.compile(r"\b(capture_ms|preprocess_ms|inference_ms|postprocess_ms|total_ms|FPS)=([0-9]+(?:\.[0-9]+)?)\b")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    args = parser.parse_args()
    rows = []
    for number, line in enumerate(args.log.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
        if not line.startswith("TIME "):
            continue
        pairs = PAIR.findall(line)
        values = {key: float(value) for key, value in pairs}
        if len(pairs) != len(FIELDS) or set(values) != set(FIELDS) or values["total_ms"] <= 0:
            raise ValueError(f"Malformed TIME record at line {number}")
        rows.append(values)
    if not rows:
        raise ValueError("No TIME records found")
    averages = {key: mean(row[key] for row in rows) for key in FIELDS}
    print(json.dumps({"records": len(rows), "mean": averages,
                      "fps_from_mean_total": 1000 / averages["total_ms"],
                      "scope": "log statistics only; board identity and comparability require separate evidence"},
                     indent=2))

if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as exc:
        raise SystemExit(f"SUMMARY_FAILED: {exc}")

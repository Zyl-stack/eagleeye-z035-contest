"""Extract final validation metrics from an Ultralytics training run."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write a compact detector metrics JSON file.")
    parser.add_argument("--run-dir", type=Path, required=True, help="Ultralytics run directory")
    parser.add_argument("--output", type=Path, default=None, help="defaults to <run-dir>/metrics_summary.json")
    return parser.parse_args()


def value(row: dict[str, str], suffix: str) -> float:
    for key, raw in row.items():
        if key.strip().endswith(suffix):
            return float(raw)
    raise KeyError(f"could not find metric ending with {suffix}")


def main() -> int:
    args = parsed_args()
    results = args.run_dir / "results.csv"
    if not results.is_file():
        raise FileNotFoundError(results)
    with results.open(encoding="utf-8", newline="") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise RuntimeError(f"no result rows in {results}")
    final = rows[-1]
    summary = {
        "epoch": int(float(final["epoch"])),
        "precision": value(final, "metrics/precision(B)"),
        "recall": value(final, "metrics/recall(B)"),
        "map50": value(final, "metrics/mAP50(B)"),
        "map50_95": value(final, "metrics/mAP50-95(B)"),
        "best_weights": str(args.run_dir / "weights" / "best.pt"),
    }
    output = args.output or args.run_dir / "metrics_summary.json"
    output.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

"""Validate the prepared PLAYER/BALL YOLO dataset before model training."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
IMAGE_SUFFIXES = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}
SPLITS = ("train", "val", "test")


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Audit a two-class PLAYER/BALL YOLO dataset.")
    parser.add_argument("--dataset", type=Path, default=ROOT / "datasets" / "football_yolo")
    parser.add_argument("--output", type=Path, default=None)
    return parser.parse_args()


def audit_split(root: Path, split: str) -> dict[str, object]:
    images = root / "images" / split
    labels = root / "labels" / split
    if not images.is_dir() or not labels.is_dir():
        return {"images": 0, "labels": 0, "empty_labels": 0, "boxes": {"PLAYER": 0, "BALL": 0}}

    image_files = sorted(item for item in images.iterdir() if item.suffix.lower() in IMAGE_SUFFIXES)
    label_files = sorted(labels.glob("*.txt"))
    image_stems = {item.stem for item in image_files}
    label_stems = {item.stem for item in label_files}
    missing_labels = sorted(image_stems - label_stems)
    orphan_labels = sorted(label_stems - image_stems)
    if missing_labels or orphan_labels:
        raise ValueError(
            f"{split}: {len(missing_labels)} images have no label and {len(orphan_labels)} labels have no image"
        )

    counts: Counter[int] = Counter()
    empty_labels = 0
    for label in label_files:
        lines = [line.split() for line in label.read_text(encoding="utf-8").splitlines() if line.strip()]
        if not lines:
            empty_labels += 1
        for line_number, fields in enumerate(lines, start=1):
            if len(fields) != 5:
                raise ValueError(f"{label}:{line_number}: expected 5 YOLO fields")
            class_id = int(fields[0])
            if class_id not in (0, 1):
                raise ValueError(f"{label}:{line_number}: expected class 0 or 1, got {class_id}")
            values = [float(value) for value in fields[1:]]
            if any(value < 0.0 or value > 1.0 for value in values):
                raise ValueError(f"{label}:{line_number}: normalized values must be in [0, 1]")
            if values[2] == 0.0 or values[3] == 0.0:
                raise ValueError(f"{label}:{line_number}: box width and height must be non-zero")
            counts[class_id] += 1
    return {
        "images": len(image_files),
        "labels": len(label_files),
        "empty_labels": empty_labels,
        "boxes": {"PLAYER": counts[0], "BALL": counts[1]},
    }


def main() -> int:
    args = parsed_args()
    result = {"dataset": str(args.dataset), "splits": {split: audit_split(args.dataset, split) for split in SPLITS}}
    train_boxes = result["splits"]["train"]["boxes"]
    if train_boxes["PLAYER"] == 0 or train_boxes["BALL"] == 0:
        raise RuntimeError("training set must contain at least one PLAYER box and one BALL box")
    output = args.output or args.dataset / "audit_report.json"
    output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

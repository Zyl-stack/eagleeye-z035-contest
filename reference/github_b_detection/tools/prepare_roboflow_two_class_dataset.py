"""Merge Roboflow YOLO exports into the fixed PLAYER/BALL training layout.

Download the two datasets through the Roboflow web UI first. This script does
not need an API key and deliberately keeps raw downloaded data outside Git.
"""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE_SPLITS = (("train", "train"), ("valid", "val"), ("val", "val"), ("test", "test"))
IMAGE_SUFFIXES = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Combine Roboflow player and ball YOLO exports into classes PLAYER=0, BALL=1."
    )
    parser.add_argument("--players-root", type=Path, required=True, help="extracted player dataset")
    parser.add_argument("--balls-root", type=Path, required=True, help="extracted ball dataset")
    parser.add_argument("--output", type=Path, default=ROOT / "datasets" / "football_yolo")
    parser.add_argument("--overwrite", action="store_true", help="replace an existing output directory")
    return parser.parse_args()


def _find_split(root: Path, source_name: str) -> Path | None:
    candidate = root / source_name
    return candidate if (candidate / "images").is_dir() and (candidate / "labels").is_dir() else None


def _rewrite_labels(
    source: Path, destination: Path, *, source_class_id: int, target_class_id: int
) -> int:
    count = 0
    with source.open(encoding="utf-8") as labels, destination.open("w", encoding="utf-8", newline="\n") as output:
        for line_number, line in enumerate(labels, start=1):
            fields = line.split()
            if not fields:
                continue
            if len(fields) != 5:
                raise ValueError(f"{source}:{line_number}: expected 5 YOLO fields, got {len(fields)}")
            input_class = int(fields[0])
            values = [float(field) for field in fields[1:]]
            if any(value < 0.0 or value > 1.0 for value in values):
                raise ValueError(f"{source}:{line_number}: normalized coordinates must be in [0, 1]")
            if input_class == source_class_id:
                output.write(f"{target_class_id} {' '.join(fields[1:])}\n")
                count += 1
    return count


def _copy_source(
    root: Path,
    output: Path,
    prefix: str,
    *,
    source_class_id: int,
    target_class_id: int,
) -> dict[str, int]:
    counts = {"images": 0, "boxes": 0}
    found_splits: set[str] = set()
    for source_name, destination_name in SOURCE_SPLITS:
        if destination_name in found_splits and source_name == "val":
            continue
        split = _find_split(root, source_name)
        if split is None:
            continue
        found_splits.add(destination_name)
        image_out = output / "images" / destination_name
        label_out = output / "labels" / destination_name
        image_out.mkdir(parents=True, exist_ok=True)
        label_out.mkdir(parents=True, exist_ok=True)
        for image in sorted((split / "images").iterdir()):
            if not image.is_file() or image.suffix.lower() not in IMAGE_SUFFIXES:
                continue
            label = split / "labels" / f"{image.stem}.txt"
            if not label.is_file():
                raise FileNotFoundError(f"missing label for image: {image}")
            destination_image = image_out / f"{prefix}_{image.name}"
            destination_label = label_out / f"{prefix}_{image.stem}.txt"
            if destination_image.exists() or destination_label.exists():
                raise FileExistsError(f"name collision at {destination_image}")
            shutil.copy2(image, destination_image)
            counts["images"] += 1
            counts["boxes"] += _rewrite_labels(
                label,
                destination_label,
                source_class_id=source_class_id,
                target_class_id=target_class_id,
            )
    if not found_splits:
        raise FileNotFoundError(f"no YOLO split directories found below {root}")
    return counts


def main() -> int:
    args = parsed_args()
    if args.output.exists():
        if not args.overwrite:
            raise FileExistsError(f"{args.output} already exists; use --overwrite to replace it")
        shutil.rmtree(args.output)
    args.output.mkdir(parents=True)
    # football-players-detection v1 class IDs: ball=0, goalkeeper=1,
    # player=2, referee=3. Keep only actual field players for this two-class
    # stage; football-ball-detection v4 has ball=0 as its only class.
    player_counts = _copy_source(
        args.players_root,
        args.output,
        "player",
        source_class_id=2,
        target_class_id=0,
    )
    ball_counts = _copy_source(
        args.balls_root,
        args.output,
        "ball",
        source_class_id=0,
        target_class_id=1,
    )
    manifest = {
        "classes": {"0": "PLAYER", "1": "BALL"},
        "players_source": str(args.players_root),
        "balls_source": str(args.balls_root),
        "player_images": player_counts["images"],
        "ball_images": ball_counts["images"],
        "player_boxes": player_counts["boxes"],
        "ball_boxes": ball_counts["boxes"],
    }
    (args.output / "dataset_manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

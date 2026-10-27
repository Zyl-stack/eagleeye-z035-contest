"""Create a short repeated-frame video for deterministic B->C regression checks.

This is intentionally a pipeline test, not a motion or broadcast-quality test.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import cv2


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Make a deterministic static-frame MP4 clip.")
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--frames", type=int, default=24)
    parser.add_argument("--fps", type=float, default=24.0)
    return parser.parse_args()


def main() -> int:
    args = parsed_args()
    if args.frames <= 0 or args.fps <= 0:
        raise ValueError("--frames and --fps must be positive")
    frame = cv2.imread(str(args.source))
    if frame is None:
        raise FileNotFoundError(args.source)
    height, width = frame.shape[:2]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    writer = cv2.VideoWriter(
        str(args.output), cv2.VideoWriter_fourcc(*"mp4v"), args.fps, (width, height)
    )
    if not writer.isOpened():
        raise RuntimeError(f"cannot create {args.output}")
    try:
        for _ in range(args.frames):
            writer.write(frame)
    finally:
        writer.release()
    print(f"wrote {args.frames} frames to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

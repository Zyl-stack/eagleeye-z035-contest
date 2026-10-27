"""Generate a repeatable PLAYER/BALL stream for the first B/C integration test."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from b_detection import BBox, CandidateDetection, MultiObjectTracker, TargetType
from b_detection.stream import write_stream


def detections_for_frame(frame_id: int) -> list[CandidateDetection]:
    player_7_x = 180 + frame_id * 3
    player_10_x = 390 + frame_id
    if frame_id < 4:
        ball_x = player_7_x + 26
    elif frame_id < 8:
        # A continuous high-speed pass; do not teleport the BALL between players.
        ball_x = 218 + (frame_id - 4) * 60
    else:
        ball_x = player_10_x + 8
    detections = [
        CandidateDetection(TargetType.PLAYER, BBox(player_7_x, 140, 42, 88), 92),
        CandidateDetection(TargetType.PLAYER, BBox(player_10_x, 145, 44, 90), 90),
    ]
    # Reproducible one-frame BALL occlusion after the pass.
    if frame_id != 8:
        detections.append(CandidateDetection(TargetType.BALL, BBox(ball_x, 205, 12, 12), 88))
    return detections


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--frames", type=int, default=12)
    parser.add_argument("--output", type=Path, default=ROOT / "build" / "b_target_stream_v1.txt")
    args = parser.parse_args()
    if args.frames <= 0:
        parser.error("--frames must be positive")

    tracker = MultiObjectTracker(max_center_distance=100.0, max_missed=3)
    frames = [
        tracker.update(frame_id, detections_for_frame(frame_id), frame_size=(640, 360))
        for frame_id in range(args.frames)
    ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="ascii", newline="\n") as output:
        write_stream(frames, output, source_size=(640, 360))
    print(f"wrote {len(frames)} frames to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

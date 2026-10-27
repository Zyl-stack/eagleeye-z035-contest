"""Run the B-side detector on one image and save a visual plus JSON report."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import cv2

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from b_detection import CandidateDetection, TargetType
from vision_detection import YoloFootballDetector


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write a qualitative PLAYER/BALL detection case.")
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--model", required=True)
    parser.add_argument("--output-image", type=Path, required=True)
    parser.add_argument("--output-json", type=Path, required=True)
    parser.add_argument("--imgsz", type=int, default=1280)
    parser.add_argument("--confidence", type=float, default=0.15)
    return parser.parse_args()


def annotate(frame: object, detections: list[CandidateDetection]) -> object:
    preview = frame.copy()
    colors = {TargetType.PLAYER: (0, 220, 0), TargetType.BALL: (0, 165, 255)}
    for detection in detections:
        bbox = detection.bbox
        color = colors[detection.target_type]
        cv2.rectangle(preview, (bbox.x, bbox.y), (bbox.x + bbox.w, bbox.y + bbox.h), color, 2)
        cv2.putText(
            preview,
            f"{detection.target_type.name} {detection.confidence}%",
            (bbox.x, max(16, bbox.y - 5)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.45,
            color,
            1,
            cv2.LINE_AA,
        )
    return preview


def main() -> int:
    args = parsed_args()
    frame = cv2.imread(str(args.source))
    if frame is None:
        raise FileNotFoundError(args.source)
    detections = YoloFootballDetector(args.model).detect(
        frame, confidence=args.confidence, image_size=args.imgsz
    )
    args.output_image.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    if not cv2.imwrite(str(args.output_image), annotate(frame, detections)):
        raise RuntimeError(f"cannot write {args.output_image}")
    report = {
        "source": str(args.source),
        "model": args.model,
        "image_size": args.imgsz,
        "confidence_threshold": args.confidence,
        "target_count": len(detections),
        "targets": [
            {
                "target_type": item.target_type.name,
                "confidence": item.confidence,
                "bbox": [item.bbox.x, item.bbox.y, item.bbox.w, item.bbox.h],
            }
            for item in detections
        ],
    }
    args.output_json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

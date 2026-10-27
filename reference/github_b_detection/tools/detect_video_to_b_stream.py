"""Run a YOLO model on a video and write B -> C target-stream V1 output."""

from __future__ import annotations

import argparse
import json
import sys
import time
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

import cv2

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from b_detection import CandidateDetection, FrameTargetList, MultiObjectTracker, TargetType
from b_detection.stream import write_stream
from vision_detection import YoloFootballDetector


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Detect PLAYER/BALL in a video and write EE_B_TARGET_STREAM_V1."
    )
    parser.add_argument("--source", type=Path, required=True, help="input video path")
    parser.add_argument("--output", type=Path, default=ROOT / "build" / "video_b_target_stream.txt")
    parser.add_argument("--model", default="yolo11n.pt", help="fine-tuned .pt or COCO pretrained model")
    parser.add_argument("--imgsz", type=int, default=1280, help="larger value helps small balls")
    parser.add_argument(
        "--tile-size",
        type=int,
        default=0,
        help="optional source-pixel square tiles for tiny balls; 0 uses full frames",
    )
    parser.add_argument("--confidence", type=float, default=0.20)
    parser.add_argument("--frame-stride", type=int, default=1)
    parser.add_argument("--max-frames", type=int, default=0, help="0 means process the full video")
    parser.add_argument("--preview-output", type=Path, default=None, help="optional annotated MP4")
    parser.add_argument("--metrics-output", type=Path, default=None, help="optional JSON metrics report")
    return parser.parse_args()


@dataclass
class VideoDetectionStats:
    source_frames: int = 0
    processed_frames: int = 0
    inference_seconds: float = 0.0

    def to_dict(
        self,
        *,
        source: Path,
        model: str,
        image_size: int,
        frame_stride: int,
        tile_size: int,
        source_fps: float,
    ) -> dict[str, object]:
        fps = self.processed_frames / self.inference_seconds if self.inference_seconds else 0.0
        return {
            "source": str(source),
            "model": model,
            "image_size": image_size,
            "frame_stride": frame_stride,
            "tile_size": tile_size,
            "source_fps": round(source_fps, 6),
            "source_frames_read": self.source_frames,
            "frames_processed": self.processed_frames,
            "detector_seconds": round(self.inference_seconds, 6),
            "detector_fps": round(fps, 3),
        }


def draw_detections(frame: object, detections: list[CandidateDetection]) -> object:
    """Return an annotated preview frame; this does not alter B/C stream values."""
    preview = frame.copy()
    colors = {TargetType.PLAYER: (0, 220, 0), TargetType.BALL: (0, 165, 255)}
    for detection in detections:
        bbox = detection.bbox
        color = colors.get(detection.target_type, (255, 255, 255))
        cv2.rectangle(preview, (bbox.x, bbox.y), (bbox.x + bbox.w, bbox.y + bbox.h), color, 2)
        cv2.putText(
            preview,
            f"{detection.target_type.name} {detection.confidence}%",
            (bbox.x, max(15, bbox.y - 5)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.45,
            color,
            1,
            cv2.LINE_AA,
        )
    return preview


def stream_video(
    capture: cv2.VideoCapture,
    detector: YoloFootballDetector,
    tracker: MultiObjectTracker,
    *,
    image_size: int,
    confidence: float,
    frame_stride: int,
    max_frames: int,
    stats: VideoDetectionStats,
    preview_output: Path | None,
    source_fps: float,
    tile_size: int,
) -> Iterator[FrameTargetList]:
    source_index = 0
    emitted = 0
    writer: cv2.VideoWriter | None = None
    while True:
        ok, frame = capture.read()
        if not ok:
            break
        stats.source_frames += 1
        if source_index % frame_stride == 0:
            height, width = frame.shape[:2]
            started = time.perf_counter()
            detections = detector.detect(
                frame,
                confidence=confidence,
                image_size=image_size,
                tile_size=tile_size,
            )
            stats.inference_seconds += time.perf_counter() - started
            stats.processed_frames += 1
            if preview_output is not None:
                if writer is None:
                    preview_output.parent.mkdir(parents=True, exist_ok=True)
                    writer = cv2.VideoWriter(
                        str(preview_output),
                        cv2.VideoWriter_fourcc(*"mp4v"),
                        source_fps / frame_stride,
                        (width, height),
                    )
                    if not writer.isOpened():
                        raise RuntimeError(f"cannot create preview video: {preview_output}")
                writer.write(draw_detections(frame, detections))
            yield tracker.update(source_index, detections, frame_size=(width, height))
            emitted += 1
            if max_frames and emitted >= max_frames:
                break
        source_index += 1
    if writer is not None:
        writer.release()


def main() -> int:
    args = parsed_args()
    if not args.source.is_file():
        raise FileNotFoundError(args.source)
    if args.frame_stride <= 0:
        raise ValueError("--frame-stride must be positive")
    capture = cv2.VideoCapture(str(args.source))
    if not capture.isOpened():
        raise RuntimeError(f"cannot open video: {args.source}")
    try:
        # These are the source-frame coordinates B emits to C.  They are not
        # related to --imgsz, which only controls YOLO's internal inference size.
        source_size = (
            int(capture.get(cv2.CAP_PROP_FRAME_WIDTH)),
            int(capture.get(cv2.CAP_PROP_FRAME_HEIGHT)),
        )
        if source_size[0] <= 0 or source_size[1] <= 0:
            raise RuntimeError(f"cannot determine source video resolution: {args.source}")
        source_fps = max(1.0, capture.get(cv2.CAP_PROP_FPS))
        detector = YoloFootballDetector(args.model)
        tracker = MultiObjectTracker(max_center_distance=100.0, max_missed=5)
        stats = VideoDetectionStats()
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("w", encoding="ascii", newline="\n") as output:
            write_stream(
                stream_video(
                    capture,
                    detector,
                    tracker,
                    image_size=args.imgsz,
                    confidence=args.confidence,
                    frame_stride=args.frame_stride,
                    max_frames=args.max_frames,
                    stats=stats,
                    preview_output=args.preview_output,
                    source_fps=source_fps,
                    tile_size=args.tile_size,
                ),
                output,
                source_size=source_size,
            )
    finally:
        capture.release()
    metrics = stats.to_dict(
        source=args.source,
        model=args.model,
        image_size=args.imgsz,
        frame_stride=args.frame_stride,
        tile_size=args.tile_size,
        source_fps=source_fps,
    )
    metrics_output = args.metrics_output or args.output.with_suffix(".metrics.json")
    metrics_output.parent.mkdir(parents=True, exist_ok=True)
    metrics_output.write_text(json.dumps(metrics, indent=2) + "\n", encoding="utf-8")
    print(f"wrote target stream to {args.output}")
    print(f"wrote metrics to {metrics_output}")
    if args.preview_output is not None:
        print(f"wrote annotated preview to {args.preview_output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

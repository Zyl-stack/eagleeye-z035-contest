"""Convert YOLO detections from a football frame into B-side candidates.

The model import is intentionally delayed: the B/C stream and tracker remain
testable on machines that do not have Ultralytics installed.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from b_detection import BBox, CandidateDetection, TargetType


BALL_NAMES = {"ball", "football", "soccer ball", "sports ball"}
PLAYER_NAMES = {"person", "player", "goalkeeper", "goal keeper"}


@dataclass(frozen=True)
class RawDetection:
    """A model-independent detection in xyxy pixel coordinates."""

    class_name: str
    confidence: float
    x1: float
    y1: float
    x2: float
    y2: float


def _target_type_for_name(class_name: str) -> TargetType | None:
    normalized = class_name.strip().lower().replace("_", " ")
    if normalized in BALL_NAMES:
        return TargetType.BALL
    if normalized in PLAYER_NAMES:
        return TargetType.PLAYER
    return None


def select_b_candidates(
    candidates: list[CandidateDetection], *, maximum: int
) -> list[CandidateDetection]:
    """Keep the B/C target budget while reserving priority for a detected ball.

    The C-side transport limit is 16 targets. A broadcast wide shot can contain
    more than 16 players, and a small ball is usually lower confidence than a
    player. Sorting BALL candidates before PLAYER candidates prevents the ball
    from being discarded simply because the player list filled the budget.
    """

    if maximum <= 0:
        raise ValueError("maximum must be positive")
    return sorted(
        candidates,
        key=lambda item: (
            0 if item.target_type is TargetType.BALL else 1,
            -item.confidence,
        ),
    )[:maximum]


def _iou(first: BBox, second: BBox) -> float:
    left = max(first.x, second.x)
    top = max(first.y, second.y)
    right = min(first.x + first.w, second.x + second.w)
    bottom = min(first.y + first.h, second.y + second.h)
    intersection = max(0, right - left) * max(0, bottom - top)
    if not intersection:
        return 0.0
    union = first.w * first.h + second.w * second.h - intersection
    return intersection / union


def deduplicate_candidates(
    candidates: list[CandidateDetection], *, iou_threshold: float = 0.50
) -> list[CandidateDetection]:
    """Apply class-wise NMS after tiled inference maps boxes to one frame."""
    if not 0.0 < iou_threshold <= 1.0:
        raise ValueError("iou_threshold must be in (0, 1]")
    kept: list[CandidateDetection] = []
    for candidate in sorted(candidates, key=lambda item: -item.confidence):
        if any(
            candidate.target_type is selected.target_type
            and _iou(candidate.bbox, selected.bbox) >= iou_threshold
            for selected in kept
        ):
            continue
        kept.append(candidate)
    return kept


def to_candidate_detection(
    detection: RawDetection,
    frame_size: tuple[int, int],
    *,
    minimum_confidence: float = 0.20,
) -> CandidateDetection | None:
    """Map one raw detection to the fixed B/C descriptor contract.

    Unknown classes, low-confidence detections, and empty boxes are discarded.
    The result always uses a left-top ``x,y,w,h`` box contained in the frame.
    """

    target_type = _target_type_for_name(detection.class_name)
    if target_type is None or detection.confidence < minimum_confidence:
        return None

    frame_width, frame_height = frame_size
    if frame_width <= 0 or frame_height <= 0:
        raise ValueError("frame dimensions must be positive")

    x1 = max(0, min(frame_width, round(detection.x1)))
    y1 = max(0, min(frame_height, round(detection.y1)))
    x2 = max(0, min(frame_width, round(detection.x2)))
    y2 = max(0, min(frame_height, round(detection.y2)))
    if x2 <= x1 or y2 <= y1:
        return None

    return CandidateDetection(
        target_type=target_type,
        bbox=BBox(x=x1, y=y1, w=x2 - x1, h=y2 - y1),
        confidence=max(0, min(100, round(detection.confidence * 100))),
    )


class YoloFootballDetector:
    """Thin YOLO11 adapter for PLAYER/BALL detections in real frames."""

    def __init__(self, model_path: str) -> None:
        try:
            from ultralytics import YOLO
        except ModuleNotFoundError as exc:
            raise RuntimeError(
                "Ultralytics is required. Install requirements-vision.txt first."
            ) from exc
        self._model: Any = YOLO(model_path)

    def detect(
        self,
        frame: Any,
        *,
        confidence: float = 0.20,
        image_size: int = 1280,
        max_detections: int = 16,
        tile_size: int = 0,
    ) -> list[CandidateDetection]:
        """Run inference and return B candidates in original-frame coordinates.

        A positive ``tile_size`` runs square source-image tiles independently.
        This is useful for a tiny ball in a 1080p wide shot: it enlarges the
        ball before YOLO's fixed ``image_size`` resize, while output boxes are
        translated back to the original source-frame coordinate system.
        """

        frame_height, frame_width = frame.shape[:2]
        if tile_size < 0:
            raise ValueError("tile_size must be zero or positive")
        if tile_size == 0:
            tiles = [(0, 0, frame)]
        else:
            if tile_size < 32:
                raise ValueError("tile_size must be at least 32 pixels")
            x_positions = _tile_positions(frame_width, tile_size)
            y_positions = _tile_positions(frame_height, tile_size)
            tiles = [
                (x, y, frame[y : min(y + tile_size, frame_height), x : min(x + tile_size, frame_width)])
                for y in y_positions
                for x in x_positions
            ]

        candidates: list[CandidateDetection] = []
        for offset_x, offset_y, tile in tiles:
            candidates.extend(
                self._detect_one(
                    tile,
                    frame_size=(frame_width, frame_height),
                    offset=(offset_x, offset_y),
                    confidence=confidence,
                    image_size=image_size,
                    max_detections=max_detections,
                )
            )
        return select_b_candidates(deduplicate_candidates(candidates), maximum=max_detections)

    def _detect_one(
        self,
        tile: Any,
        *,
        frame_size: tuple[int, int],
        offset: tuple[int, int],
        confidence: float,
        image_size: int,
        max_detections: int,
    ) -> list[CandidateDetection]:
        result = self._model.predict(
            source=tile,
            conf=confidence,
            imgsz=image_size,
            # Ask YOLO for enough boxes before applying our smaller transport
            # budget. Otherwise its confidence ordering can remove the ball.
            max_det=max(100, max_detections),
            verbose=False,
        )[0]
        names = result.names
        candidates: list[CandidateDetection] = []
        offset_x, offset_y = offset
        for box in result.boxes:
            class_index = int(box.cls[0].item())
            raw = RawDetection(
                class_name=str(names[class_index]),
                confidence=float(box.conf[0].item()),
                x1=float(box.xyxy[0][0].item()) + offset_x,
                y1=float(box.xyxy[0][1].item()) + offset_y,
                x2=float(box.xyxy[0][2].item()) + offset_x,
                y2=float(box.xyxy[0][3].item()) + offset_y,
            )
            candidate = to_candidate_detection(
                raw, frame_size, minimum_confidence=confidence
            )
            if candidate is not None:
                candidates.append(candidate)
        return candidates


def _tile_positions(length: int, tile_size: int) -> list[int]:
    """Cover an axis while adding one edge-aligned tile when needed."""
    if length <= tile_size:
        return [0]
    positions = list(range(0, length - tile_size + 1, tile_size))
    edge = length - tile_size
    if positions[-1] != edge:
        positions.append(edge)
    return positions

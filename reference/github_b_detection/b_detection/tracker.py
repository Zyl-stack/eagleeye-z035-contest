"""Small class-aware tracker used to keep B-side target IDs stable."""

from __future__ import annotations

from dataclasses import dataclass

from .descriptor import (
    BBox,
    CandidateDetection,
    FrameTargetList,
    INVALID_TARGET_ID,
    MAX_TARGETS,
    RESERVED_TARGET_ID,
    TargetDescriptor,
    TargetType,
)


def _iou(left: BBox, right: BBox) -> float:
    x1 = max(left.x, right.x)
    y1 = max(left.y, right.y)
    x2 = min(left.x + left.w, right.x + right.w)
    y2 = min(left.y + left.h, right.y + right.h)
    intersection = max(0, x2 - x1) * max(0, y2 - y1)
    union = left.w * left.h + right.w * right.h - intersection
    return intersection / union if union else 0.0


def _distance(left: BBox, right: BBox) -> float:
    lx, ly = left.center
    rx, ry = right.center
    return ((lx - rx) ** 2 + (ly - ry) ** 2) ** 0.5


def _clamp_i16(value: float) -> int:
    return max(-0x8000, min(0x7FFF, round(value)))


@dataclass
class _Track:
    target_id: int
    target_type: TargetType
    bbox: BBox
    confidence: float
    last_frame_id: int
    age: int = 1
    miss_count: int = 0
    vx: float = 0.0
    vy: float = 0.0
    velocity_valid: bool = False

    def predicted_bbox(self, frame_delta: int) -> BBox:
        return BBox(
            max(0, round(self.bbox.x + self.vx * frame_delta)),
            max(0, round(self.bbox.y + self.vy * frame_delta)),
            self.bbox.w,
            self.bbox.h,
        )


class MultiObjectTracker:
    """Greedy tracker for the B/C integration baseline, not a learned detector."""

    def __init__(
        self,
        *,
        iou_threshold: float = 0.10,
        max_center_distance: float = 90.0,
        velocity_alpha: float = 0.60,
        max_missed: int = 5,
    ) -> None:
        self.iou_threshold = iou_threshold
        self.max_center_distance = max_center_distance
        self.velocity_alpha = velocity_alpha
        self.max_missed = max_missed
        self._tracks: dict[int, _Track] = {}
        self._next_target_id = 1

    def _allocate_id(self) -> int:
        for _ in range(INVALID_TARGET_ID - 1):
            candidate = self._next_target_id
            self._next_target_id = 1 if candidate >= INVALID_TARGET_ID - 1 else candidate + 1
            if candidate != RESERVED_TARGET_ID and candidate not in self._tracks:
                return candidate
        raise RuntimeError("no free target_id")

    def update(
        self,
        frame_id: int,
        detections: list[CandidateDetection],
        *,
        frame_size: tuple[int, int],
    ) -> FrameTargetList:
        if len(detections) > MAX_TARGETS:
            raise ValueError(f"received {len(detections)} detections; maximum is {MAX_TARGETS}")
        if frame_id < 0 or frame_id > 0xFFFFFFFF:
            raise ValueError("frame_id must fit uint32")
        frame_width, frame_height = frame_size
        if frame_width <= 0 or frame_height <= 0:
            raise ValueError("frame dimensions must be positive")
        for detection in detections:
            detection.validate()

        candidates: list[tuple[float, int, int]] = []
        for target_id, track in self._tracks.items():
            delta = max(1, frame_id - track.last_frame_id)
            predicted = track.predicted_bbox(delta)
            for index, detection in enumerate(detections):
                if detection.target_type != track.target_type:
                    continue
                distance_limit = self.max_center_distance * (
                    2.0 if track.target_type is TargetType.BALL else 1.0
                )
                iou = _iou(predicted, detection.bbox)
                distance = _distance(predicted, detection.bbox)
                if iou >= self.iou_threshold or distance <= distance_limit:
                    score = 2.0 * iou + max(0.0, 1.0 - distance / distance_limit)
                    candidates.append((score, target_id, index))

        matched_tracks: set[int] = set()
        matched_detections: set[int] = set()
        matches: list[tuple[int, int]] = []
        for _score, target_id, detection_index in sorted(candidates, reverse=True):
            if target_id in matched_tracks or detection_index in matched_detections:
                continue
            matched_tracks.add(target_id)
            matched_detections.add(detection_index)
            matches.append((target_id, detection_index))

        for target_id, detection_index in matches:
            track = self._tracks[target_id]
            detection = detections[detection_index]
            delta = max(1, frame_id - track.last_frame_id)
            old_cx, old_cy = track.bbox.center
            new_cx, new_cy = detection.bbox.center
            measured_vx = (new_cx - old_cx) / delta
            measured_vy = (new_cy - old_cy) / delta
            a = self.velocity_alpha
            track.vx = a * measured_vx + (1.0 - a) * track.vx
            track.vy = a * measured_vy + (1.0 - a) * track.vy
            track.velocity_valid = True
            track.bbox = detection.bbox
            track.confidence = detection.confidence
            track.age = min(0xFFFF, track.age + delta)
            track.miss_count = 0
            track.last_frame_id = frame_id

        for target_id, track in list(self._tracks.items()):
            if target_id in matched_tracks:
                continue
            delta = max(1, frame_id - track.last_frame_id)
            track.bbox = track.predicted_bbox(delta)
            track.confidence *= 0.75
            track.age = min(0xFFFF, track.age + delta)
            track.miss_count = min(0xFFFF, track.miss_count + delta)
            track.velocity_valid = False
            track.last_frame_id = frame_id
            if track.miss_count > self.max_missed:
                del self._tracks[target_id]

        for index, detection in enumerate(detections):
            if index in matched_detections:
                continue
            if len(self._tracks) >= MAX_TARGETS:
                break
            target_id = self._allocate_id()
            self._tracks[target_id] = _Track(
                target_id=target_id,
                target_type=detection.target_type,
                bbox=detection.bbox,
                confidence=float(detection.confidence),
                last_frame_id=frame_id,
            )

        targets = tuple(
            self._descriptor(track, frame_id, frame_width, frame_height)
            for track in sorted(self._tracks.values(), key=lambda item: item.target_id)
        )
        result = FrameTargetList(frame_id=frame_id, targets=targets)
        result.validate()
        return result

    @staticmethod
    def _descriptor(
        track: _Track, frame_id: int, frame_width: int, frame_height: int
    ) -> TargetDescriptor:
        x = min(max(0, track.bbox.x), frame_width - 1)
        y = min(max(0, track.bbox.y), frame_height - 1)
        bbox = BBox(
            x,
            y,
            max(1, min(track.bbox.w, frame_width - x)),
            max(1, min(track.bbox.h, frame_height - y)),
        )
        return TargetDescriptor(
            frame_id=frame_id,
            target_id=track.target_id,
            target_type=track.target_type,
            bbox=bbox,
            confidence=max(0, min(100, round(track.confidence))),
            valid=track.miss_count == 0,
            vx=_clamp_i16(track.vx),
            vy=_clamp_i16(track.vy),
            velocity_valid=track.velocity_valid,
            age=track.age,
            miss_count=track.miss_count,
        )

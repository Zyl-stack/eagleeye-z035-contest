"""Transport-independent B -> C soccer target descriptors."""

from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum


MAX_TARGETS = 16
RESERVED_TARGET_ID = 0
INVALID_TARGET_ID = 0xFFFF


class TargetType(IntEnum):
    UNKNOWN = 0
    BALL = 1
    PLAYER = 2
    REFEREE = 3


@dataclass(frozen=True)
class BBox:
    x: int
    y: int
    w: int
    h: int

    def validate(self) -> None:
        for name, value in (("x", self.x), ("y", self.y), ("w", self.w), ("h", self.h)):
            if not 0 <= value <= 0xFFFF:
                raise ValueError(f"bbox.{name} must fit uint16, got {value}")
        if self.w == 0 or self.h == 0:
            raise ValueError("bbox width and height must be non-zero")

    @property
    def center(self) -> tuple[float, float]:
        return self.x + self.w / 2.0, self.y + self.h / 2.0


@dataclass(frozen=True)
class CandidateDetection:
    target_type: TargetType
    bbox: BBox
    confidence: int

    def validate(self) -> None:
        self.bbox.validate()
        if not 0 <= self.confidence <= 100:
            raise ValueError(f"confidence must be in [0, 100], got {self.confidence}")


@dataclass(frozen=True)
class TargetDescriptor:
    frame_id: int
    target_id: int
    target_type: TargetType
    bbox: BBox
    confidence: int
    valid: bool
    vx: int = 0
    vy: int = 0
    velocity_valid: bool = False
    age: int = 1
    miss_count: int = 0

    def validate(self) -> None:
        self.bbox.validate()
        if not 0 <= self.frame_id <= 0xFFFFFFFF:
            raise ValueError(f"frame_id must fit uint32, got {self.frame_id}")
        if not RESERVED_TARGET_ID < self.target_id < INVALID_TARGET_ID:
            raise ValueError(
                f"target_id must be in [1, {INVALID_TARGET_ID - 1}], got {self.target_id}"
            )
        if not 0 <= self.confidence <= 100:
            raise ValueError(f"confidence must be in [0, 100], got {self.confidence}")
        if not -0x8000 <= self.vx <= 0x7FFF or not -0x8000 <= self.vy <= 0x7FFF:
            raise ValueError("vx/vy must fit int16")
        if not 0 <= self.age <= 0xFFFF or not 0 <= self.miss_count <= 0xFFFF:
            raise ValueError("age/miss_count must fit uint16")

    def to_dict(self) -> dict[str, object]:
        self.validate()
        return {
            "frame_id": self.frame_id,
            "target_id": self.target_id,
            "target_type": self.target_type.name,
            "bbox": [self.bbox.x, self.bbox.y, self.bbox.w, self.bbox.h],
            "confidence": self.confidence,
            "valid": int(self.valid),
            "vx": self.vx,
            "vy": self.vy,
            "velocity_valid": int(self.velocity_valid),
            "age": self.age,
            "miss_count": self.miss_count,
        }


@dataclass(frozen=True)
class FrameTargetList:
    frame_id: int
    targets: tuple[TargetDescriptor, ...]

    def validate(self) -> None:
        if not 0 <= self.frame_id <= 0xFFFFFFFF:
            raise ValueError(f"frame_id must fit uint32, got {self.frame_id}")
        if len(self.targets) > MAX_TARGETS:
            raise ValueError(f"at most {MAX_TARGETS} targets are allowed")
        ids: set[int] = set()
        for target in self.targets:
            target.validate()
            if target.frame_id != self.frame_id:
                raise ValueError("target frame_id must match its containing frame")
            if target.target_id in ids:
                raise ValueError(f"duplicate target_id {target.target_id} in frame {self.frame_id}")
            ids.add(target.target_id)

    @property
    def target_count(self) -> int:
        return len(self.targets)

    def to_dict(self) -> dict[str, object]:
        self.validate()
        return {
            "frame_id": self.frame_id,
            "target_count": self.target_count,
            "targets": [target.to_dict() for target in self.targets],
        }

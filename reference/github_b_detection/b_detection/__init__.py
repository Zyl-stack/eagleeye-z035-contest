"""B-side soccer detection descriptors and stable multi-target tracking."""

from .descriptor import (
    BBox,
    CandidateDetection,
    FrameTargetList,
    INVALID_TARGET_ID,
    RESERVED_TARGET_ID,
    TargetDescriptor,
    TargetType,
)
from .tracker import MultiObjectTracker

__all__ = [
    "BBox",
    "CandidateDetection",
    "FrameTargetList",
    "INVALID_TARGET_ID",
    "MultiObjectTracker",
    "RESERVED_TARGET_ID",
    "TargetDescriptor",
    "TargetType",
]

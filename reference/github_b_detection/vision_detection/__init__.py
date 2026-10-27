"""Real-image detection adapters for the B-side target stream."""

from .yolo_adapter import RawDetection, YoloFootballDetector, to_candidate_detection

__all__ = ["RawDetection", "YoloFootballDetector", "to_candidate_detection"]

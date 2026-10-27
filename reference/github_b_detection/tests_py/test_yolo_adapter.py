from __future__ import annotations

import unittest

from b_detection import BBox, CandidateDetection, TargetType
from vision_detection.yolo_adapter import (
    RawDetection,
    deduplicate_candidates,
    select_b_candidates,
    to_candidate_detection,
)


class YoloAdapterTests(unittest.TestCase):
    def test_coco_sports_ball_maps_to_ball(self) -> None:
        candidate = to_candidate_detection(
            RawDetection("sports ball", 0.83, 10.2, 20.1, 22.8, 31.7), (640, 360)
        )
        self.assertIsNotNone(candidate)
        assert candidate is not None
        self.assertEqual(candidate.target_type, TargetType.BALL)
        self.assertEqual(candidate.bbox.x, 10)
        self.assertEqual(candidate.bbox.w, 13)

    def test_person_maps_to_player_and_box_is_clamped(self) -> None:
        candidate = to_candidate_detection(RawDetection("person", 0.91, -5, 4, 20, 400), (100, 80))
        self.assertIsNotNone(candidate)
        assert candidate is not None
        self.assertEqual(candidate.target_type, TargetType.PLAYER)
        self.assertEqual((candidate.bbox.x, candidate.bbox.y, candidate.bbox.w, candidate.bbox.h), (0, 4, 20, 76))

    def test_unknown_or_low_confidence_detection_is_not_emitted(self) -> None:
        self.assertIsNone(to_candidate_detection(RawDetection("car", 0.99, 0, 0, 10, 10), (100, 100)))
        self.assertIsNone(to_candidate_detection(RawDetection("ball", 0.19, 0, 0, 10, 10), (100, 100)))

    def test_ball_is_retained_when_player_count_exceeds_transport_budget(self) -> None:
        players = [
            CandidateDetection(TargetType.PLAYER, BBox(index, 0, 5, 5), 99 - index)
            for index in range(16)
        ]
        ball = CandidateDetection(TargetType.BALL, BBox(100, 100, 4, 4), 25)
        selected = select_b_candidates(players + [ball], maximum=16)
        self.assertEqual(len(selected), 16)
        self.assertEqual(selected[0].target_type, TargetType.BALL)
        self.assertIn(ball, selected)

    def test_tiled_overlap_deduplicates_same_class_box(self) -> None:
        duplicate = CandidateDetection(TargetType.BALL, BBox(100, 100, 12, 12), 80)
        overlapping_lower_score = CandidateDetection(TargetType.BALL, BBox(101, 100, 12, 12), 71)
        player = CandidateDetection(TargetType.PLAYER, BBox(101, 100, 12, 12), 90)
        self.assertEqual(
            deduplicate_candidates([overlapping_lower_score, player, duplicate]),
            [player, duplicate],
        )


if __name__ == "__main__":
    unittest.main()

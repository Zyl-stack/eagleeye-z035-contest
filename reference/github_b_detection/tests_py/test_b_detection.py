from __future__ import annotations

import io
import unittest
from pathlib import Path

from b_detection import (
    BBox,
    CandidateDetection,
    MultiObjectTracker,
    TargetDescriptor,
    TargetType,
)
from b_detection.stream import read_stream, write_stream


class BDetectionTests(unittest.TestCase):
    def test_target_id_zero_is_reserved(self) -> None:
        target = TargetDescriptor(
            frame_id=1,
            target_id=0,
            target_type=TargetType.PLAYER,
            bbox=BBox(10, 10, 20, 40),
            confidence=90,
            valid=True,
        )
        with self.assertRaises(ValueError):
            target.validate()

    def test_ids_stay_stable_and_types_never_cross_match(self) -> None:
        tracker = MultiObjectTracker(max_center_distance=80.0)
        first = tracker.update(
            100,
            [
                CandidateDetection(TargetType.PLAYER, BBox(100, 80, 40, 80), 94),
                CandidateDetection(TargetType.BALL, BBox(130, 140, 12, 12), 88),
            ],
            frame_size=(640, 360),
        )
        second = tracker.update(
            101,
            [
                CandidateDetection(TargetType.PLAYER, BBox(105, 80, 40, 80), 93),
                CandidateDetection(TargetType.BALL, BBox(136, 140, 12, 12), 87),
            ],
            frame_size=(640, 360),
        )
        self.assertEqual(
            {target.target_type: target.target_id for target in first.targets},
            {target.target_type: target.target_id for target in second.targets},
        )
        ball = next(target for target in second.targets if target.target_type is TargetType.BALL)
        self.assertGreater(ball.vx, 0)

    def test_occlusion_emits_invalid_track_and_reacquires_same_id(self) -> None:
        tracker = MultiObjectTracker(max_missed=2)
        visible = tracker.update(
            1,
            [CandidateDetection(TargetType.BALL, BBox(200, 150, 12, 12), 90)],
            frame_size=(640, 360),
        )
        ball_id = visible.targets[0].target_id
        missing = tracker.update(2, [], frame_size=(640, 360))
        self.assertEqual(missing.targets[0].target_id, ball_id)
        self.assertFalse(missing.targets[0].valid)
        self.assertEqual(missing.targets[0].miss_count, 1)
        reacquired = tracker.update(
            3,
            [CandidateDetection(TargetType.BALL, BBox(205, 150, 12, 12), 89)],
            frame_size=(640, 360),
        )
        self.assertEqual(reacquired.targets[0].target_id, ball_id)
        self.assertTrue(reacquired.targets[0].valid)
        self.assertEqual(reacquired.targets[0].miss_count, 0)

    def test_fast_pass_and_occlusion_keep_one_ball_id(self) -> None:
        tracker = MultiObjectTracker(max_center_distance=100.0, max_missed=3)
        ball_ids: set[int] = set()
        positions = [218, 278, 338, 398, None, 407]
        for frame_id, x in enumerate(positions, start=4):
            detections = [] if x is None else [
                CandidateDetection(TargetType.BALL, BBox(x, 205, 12, 12), 88)
            ]
            frame = tracker.update(frame_id, detections, frame_size=(640, 360))
            balls = [target for target in frame.targets if target.target_type is TargetType.BALL]
            self.assertEqual(len(balls), 1)
            ball_ids.add(balls[0].target_id)
        self.assertEqual(len(ball_ids), 1)

    def test_stream_round_trip_preserves_every_candidate(self) -> None:
        tracker = MultiObjectTracker()
        frame = tracker.update(
            125,
            [
                CandidateDetection(TargetType.PLAYER, BBox(210, 120, 35, 78), 92),
                CandidateDetection(TargetType.PLAYER, BBox(330, 115, 36, 80), 88),
                CandidateDetection(TargetType.BALL, BBox(342, 165, 10, 10), 81),
            ],
            frame_size=(640, 360),
        )
        output = io.StringIO()
        write_stream([frame], output, source_size=(1280, 720))
        self.assertEqual(
            output.getvalue().splitlines()[:2],
            ["EE_B_TARGET_STREAM_V1", "STREAM_INFO,1280,720"],
        )
        restored = list(read_stream(io.StringIO(output.getvalue())))
        self.assertEqual(len(restored), 1)
        self.assertEqual(restored[0], frame)
        self.assertEqual(restored[0].target_count, 3)

    def test_legacy_stream_without_stream_info_remains_compatible(self) -> None:
        legacy = "\n".join(
            [
                "EE_B_TARGET_STREAM_V1",
                "FRAME,4,1",
                "TARGET,4,1,BALL,100,80,10,10,91,1,0,0,0,1,0",
                "END_FRAME",
            ]
        )
        frames = list(read_stream(io.StringIO(legacy)))
        self.assertEqual(len(frames), 1)
        self.assertEqual(frames[0].targets[0].bbox, BBox(100, 80, 10, 10))

    def test_committed_sample_has_pass_and_short_ball_occlusion(self) -> None:
        sample = Path(__file__).resolve().parents[1] / "examples" / "b_target_stream_v1_sample.txt"
        with sample.open(encoding="ascii") as stream:
            frames = list(read_stream(stream))

        self.assertEqual(len(frames), 12)
        self.assertTrue(all(frame.target_count == 3 for frame in frames))
        ball_ids = {
            target.target_id
            for frame in frames
            for target in frame.targets
            if target.target_type is TargetType.BALL
        }
        self.assertEqual(ball_ids, {3})
        frame_8_ball = next(
            target for target in frames[8].targets if target.target_type is TargetType.BALL
        )
        frame_9_ball = next(
            target for target in frames[9].targets if target.target_type is TargetType.BALL
        )
        self.assertFalse(frame_8_ball.valid)
        self.assertTrue(frame_9_ball.valid)
        self.assertEqual(frame_8_ball.target_id, frame_9_ball.target_id)


if __name__ == "__main__":
    unittest.main()

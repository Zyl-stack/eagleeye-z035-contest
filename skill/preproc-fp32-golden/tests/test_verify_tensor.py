import struct
import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from verify_tensor import compare


class TensorComparisonTest(unittest.TestCase):
    def test_exact(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frame.f32"
            path.write_bytes(struct.pack("<4f", 0.0, 1.5, -2.0, 3.25))
            result = compare(path, path, [2, 2], 0.0, 0.0, True)
            self.assertTrue(result["passed"])
            self.assertEqual(result["exact"], 4)

    def test_mismatch_and_length(self):
        with tempfile.TemporaryDirectory() as directory:
            golden = Path(directory) / "golden.f32"
            actual = Path(directory) / "actual.f32"
            golden.write_bytes(struct.pack("<2f", 1.0, 2.0))
            actual.write_bytes(struct.pack("<2f", 1.0, 2.5))
            result = compare(actual, golden, [2], 0.1, 0.0, False)
            self.assertFalse(result["passed"])
            self.assertEqual(result["over_tolerance"], 1)
            with self.assertRaises(ValueError):
                compare(actual, golden, [3], 0.0, 0.0, False)


if __name__ == "__main__":
    unittest.main()

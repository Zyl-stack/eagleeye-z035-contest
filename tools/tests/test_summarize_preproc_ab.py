import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from summarize_preproc_ab import load, summary


class PreprocAbTest(unittest.TestCase):
    def test_complete_and_discard(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / 'cpu.log'
            p.write_text(('PREPROC backend=cpu cycles=0\nTIME capture_ms=1 preprocess_ms=2 inference_ms=3 postprocess_ms=4 total_ms=10\n') * 35)
            result = summary(load(p, 'cpu'))
            self.assertEqual(result['frames_measured'], 30)
            self.assertEqual(result['fps_from_mean_total'], 100.0)

    def test_backend_and_missing_rows_and_fallback(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / 'cpu.log'
            row = 'PREPROC backend=cpu cycles=0\nTIME capture_ms=1 preprocess_ms=2 inference_ms=3 postprocess_ms=4 total_ms=10\n'
            for invalid in [row * 34, row * 35 + 'PREPROC_FALLBACK runtime=error\n', (row * 35).replace('backend=cpu', 'backend=fpga', 1), (row * 35).replace('total_ms=10', 'total_ms=0', 1)]:
                p.write_text(invalid)
                with self.subTest(invalid=invalid[-50:]), self.assertRaises(ValueError):
                    load(p, 'cpu')


if __name__ == '__main__':
    unittest.main()

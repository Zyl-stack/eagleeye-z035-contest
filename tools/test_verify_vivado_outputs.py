#!/usr/bin/env python3
"""Tests for artifact identity checker; all data here is synthetic."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
import zipfile

SCRIPT = Path(__file__).with_name('verify_vivado_outputs.py')

class OutputCheckTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.bit = self.root / 'vivado_work/eagleeye_z035.runs/impl_1/z100_top.bit'
        self.bit.parent.mkdir(parents=True)
        self.bit.write_bytes(b'bit')
        self.xsa = self.root / 'eagleeye_z035.xsa'

    def tearDown(self):
        self.temp.cleanup()

    def run_check(self):
        return subprocess.run([sys.executable, str(SCRIPT), str(self.root)], capture_output=True)

    def archive(self, bit=b'bit', hwh=True):
        with zipfile.ZipFile(self.xsa, 'w') as z:
            z.writestr('design.bit', bit)
            if hwh:
                z.writestr('system.hwh', b'hwh')

    def test_matching(self):
        self.archive()
        self.assertEqual(self.run_check().returncode, 0)

    def test_mismatch_and_missing_hwh(self):
        self.archive(bit=b'other')
        self.assertNotEqual(self.run_check().returncode, 0)
        self.archive(hwh=False)
        self.assertNotEqual(self.run_check().returncode, 0)

if __name__ == '__main__':
    unittest.main(verbosity=2)
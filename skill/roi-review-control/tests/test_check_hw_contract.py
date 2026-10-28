import sys
import json
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from check_hw_contract import check


class ContractTest(unittest.TestCase):
    def test_empty_contract_is_invalid(self):
        with self.assertRaisesRegex(ValueError, "contract contains no checks"):
            check(ET.fromstring("<EDKSYSTEM/>"), {})

    def test_invalid_archive_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "bad.xsa").write_bytes(b"not a zip")
            (root / "contract.json").write_text(json.dumps({"modules": ["sample"]}))
            tool = Path(__file__).resolve().parents[1] / "tools/check_hw_contract.py"
            result = subprocess.run(
                [sys.executable, "-B", str(tool), "--xsa", str(root / "bad.xsa"),
                 "--config", str(root / "contract.json")],
                capture_output=True, text=True,
            )
            self.assertEqual(result.returncode, 2)
            self.assertIn("INVALID:", result.stderr)

    def test_pass_and_fail(self):
        root = ET.fromstring(
            '<EDKSYSTEM><EXTERNALPORTS><PORT NAME="pause_key"/></EXTERNALPORTS>'
            '<MODULES><MODULE INSTANCE="video_mode_switch_top_0"/></MODULES>'
            '<MEMRANGE INSTANCE="fovea_axil_ctrl_0" BASEVALUE="0x40000000"/>'
            '</EDKSYSTEM>'
        )
        contract = {
            "modules": ["video_mode_switch_top_0"],
            "top_ports": ["pause_key"],
            "register_bases": {"fovea_axil_ctrl_0": "0x40000000"},
        }
        self.assertEqual(check(root, contract), [])
        contract["top_ports"].append("mode_key")
        self.assertIn("missing top port: mode_key", check(root, contract))


if __name__ == "__main__":
    unittest.main()

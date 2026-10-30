"""Behavioral regressions for the independently reusable validation helpers."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def module(folder, filename):
    path = ROOT/folder/"scripts"/filename
    spec = importlib.util.spec_from_file_location(filename.replace(".", "_"), path)
    obj = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(obj)
    return obj


F32 = module("02-tensor-preprocess-golden", "compare_f32.py")
ELF = module("03-armv7-inference-deploy", "audit_elf.py")
XSA = module("04-petalinux-xsa-rebuild", "verify_xsa_bit.py")
STREAM = module("06-detection-stream-contract", "check_stream.py")
MANIFEST = module("11-reproducible-evidence-release", "manifest.py")


def stream(rows, size="STREAM_INFO,640,640\n"):
    return "EE_B_TARGET_STREAM_V1\n"+size+rows


def target(frame=0, tid=1, kind="BALL", y=500, valid=1, velocity_valid=0):
    return f"TARGET,{frame},{tid},{kind},10,{y},6,11,26,{valid},0,0,{velocity_valid},1,0\n"


class TemporaryCase(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def f32(self, name, values):
        path = self.root/name
        path.write_bytes(struct.pack("<"+"f"*len(values), *values))
        return path


class FloatTests(TemporaryCase):
    def test_identical_and_signed_zero(self):
        a = self.f32("a", [0.0, 1.5, -2.0])
        b = self.f32("b", [-0.0, 1.5, -2.0])
        self.assertTrue(F32.compare(a, b)["pass"])
        result = F32.compare(a, b, exact=True)
        self.assertFalse(result["pass"])
        self.assertEqual(result["bit_equal"], 2)
        self.assertEqual(result["first_failure"]["index"], 0)

    def test_tolerance_and_first_difference(self):
        a = self.f32("a", [1.0, 2.0, 3.0])
        b = self.f32("b", [1.0, 2.01, 3.0])
        self.assertFalse(F32.compare(a, b, atol=.001)["pass"])
        self.assertTrue(F32.compare(a, b, atol=.02)["pass"])
        self.assertEqual(F32.compare(a, b)["first_failure"]["index"], 1)

    def test_truncated_and_count(self):
        a = self.f32("a", [1.0, 2.0])
        b = self.f32("b", [1.0])
        self.assertFalse(F32.compare(a, b)["pass"])
        self.assertFalse(F32.compare(a, a, count=3)["pass"])

    def test_nonfinite_even_if_bit_exact(self):
        a = self.f32("a", [float("nan"), float("inf")])
        result = F32.compare(a, a, exact=True)
        self.assertFalse(result["pass"])
        self.assertEqual(result["nonfinite_pairs"], 2)
        json.dumps(result, allow_nan=False)

    def test_invalid_format_or_tolerance(self):
        path = self.root/"bad"
        path.write_bytes(b"123")
        with self.assertRaises(ValueError):
            F32.compare(path, path)
        path.write_bytes(b"")
        with self.assertRaises(ValueError):
            F32.compare(path, path)
        with self.assertRaises(ValueError):
            F32.compare(path, path, atol=float("nan"))


class ElfTests(TemporaryCase):
    def make_elf(self, name, flags=0x05000400, machine=40, elf_class=1):
        data = bytearray(64)
        data[:7] = b"\x7fELF"+bytes((elf_class, 1, 1))
        struct.pack_into("<H", data, 18, machine)
        struct.pack_into("<I", data, 20, 1)
        struct.pack_into("<I", data, 36 if elf_class == 1 else 48, flags)
        struct.pack_into("<H", data, 40 if elf_class == 1 else 52, 52 if elf_class == 1 else 64)
        path = self.root/name
        path.write_bytes(data)
        return path

    def test_candidate_and_wrong_architecture(self):
        self.assertTrue(ELF.audit(self.make_elf("arm"))["pass"])
        self.assertFalse(ELF.audit(self.make_elf("a64", machine=183, elf_class=2))["pass"])

    def test_float_flags_and_truncation(self):
        self.assertFalse(ELF.audit(self.make_elf("soft", flags=0x05000200))["pass"])
        self.assertFalse(ELF.audit(self.make_elf("both", flags=0x05000600))["pass"])
        bad = self.root/"truncated"
        bad.write_bytes(b"\x7fELF"+bytes((1, 1, 1))+bytes(10))
        with self.assertRaises(ValueError):
            ELF.audit(bad)


class XsaTests(TemporaryCase):
    def test_identity_and_replaced_bitstream(self):
        xsa, bit = self.root/"hw.xsa", self.root/"hw.bit"
        with zipfile.ZipFile(xsa, "w") as archive:
            archive.writestr("hw/system.bit", b"synthetic-bitstream")
        bit.write_bytes(b"synthetic-bitstream")
        self.assertTrue(XSA.verify(xsa, bit)["pass"])
        bit.write_bytes(b"other")
        self.assertFalse(XSA.verify(xsa, bit)["pass"])

    def test_ambiguous_member_requires_selection(self):
        xsa, bit = self.root/"hw.xsa", self.root/"hw.bit"
        with zipfile.ZipFile(xsa, "w") as archive:
            archive.writestr("a.bit", b"A")
            archive.writestr("b.bit", b"B")
        bit.write_bytes(b"B")
        with self.assertRaises(ValueError):
            XSA.verify(xsa, bit)
        self.assertTrue(XSA.verify(xsa, bit, "b.bit")["pass"])


class StreamTests(unittest.TestCase):
    def test_valid_hold_and_recovery(self):
        rows = "FRAME,0,1\n"+target()+"END_FRAME\n"
        rows += "FRAME,2,1\n"+target(frame=2, valid=0)+"END_FRAME\n"
        rows += "FRAME,4,1\n"+target(frame=4)+"END_FRAME\n"
        result = STREAM.validate(stream(rows))
        self.assertEqual(result["frames"], 3)
        self.assertEqual(result["held_records"], 1)
        self.assertEqual(result["observed_invalid_to_valid"], 1)

    def test_missing_dimensions_and_explicit_override(self):
        text = stream("FRAME,0,1\n"+target()+"END_FRAME\n", size="")
        with self.assertRaises(STREAM.ContractError):
            STREAM.validate(text)
        self.assertTrue(STREAM.validate(text, legacy_size=(640, 640))["pass"])
        with self.assertRaises(STREAM.ContractError):
            STREAM.validate(text, legacy_size=(640, 360))

    def test_truncated_and_count_mismatch(self):
        with self.assertRaises(STREAM.ContractError):
            STREAM.validate(stream("FRAME,0,1\n"+target()))
        with self.assertRaises(STREAM.ContractError):
            STREAM.validate(stream("FRAME,0,2\n"+target()+"END_FRAME\n"))

    def test_duplicate_reserved_and_changed_identity(self):
        for tid in (0, 65535):
            with self.subTest(tid=tid), self.assertRaises(STREAM.ContractError):
                STREAM.validate(stream("FRAME,0,1\n"+target(tid=tid)+"END_FRAME\n"))
        with self.assertRaises(STREAM.ContractError):
            STREAM.validate(stream("FRAME,0,2\n"+target()+target()+"END_FRAME\n"))
        with self.assertRaises(STREAM.ContractError):
            STREAM.validate(stream("FRAME,0,1\n"+target()+"END_FRAME\nFRAME,1,1\n"+
                                   target(frame=1, kind="PLAYER")+"END_FRAME\n"))

    def test_bad_confidence_flags_and_repeated_frame(self):
        good = stream("FRAME,0,1\n"+target()+"END_FRAME\n")
        for bad in (good.replace(",26,1,", ",101,1,"),
                    good.replace(",26,1,", ",26,2,"),
                    good+"FRAME,0,0\nEND_FRAME\n"):
            with self.subTest(bad=bad), self.assertRaises(STREAM.ContractError):
                STREAM.validate(bad)


class ManifestTests(TemporaryCase):
    def release(self):
        root = self.root/"release"
        root.mkdir()
        (root/"config.json").write_text("{}", encoding="utf-8")
        manifest = self.root/"manifest.json"
        MANIFEST.create(root, manifest)
        return root, manifest

    def test_change_extra_and_missing(self):
        root, manifest = self.release()
        self.assertTrue(MANIFEST.check(root, manifest)["pass"])
        (root/"config.json").write_text('{"changed":true}', encoding="utf-8")
        (root/"new.bin").write_bytes(b"X")
        result = MANIFEST.check(root, manifest)
        self.assertEqual(result["changed"], ["config.json"])
        self.assertEqual(result["extra"], ["new.bin"])
        (root/"config.json").unlink()
        self.assertEqual(MANIFEST.check(root, manifest)["missing"], ["config.json"])

    def test_reject_path_escape_and_self_reference(self):
        root, manifest = self.release()
        data = json.loads(manifest.read_text(encoding="utf-8"))
        data["files"]["../outside"] = data["files"]["config.json"]
        manifest.write_text(json.dumps(data), encoding="utf-8")
        with self.assertRaises(ValueError):
            MANIFEST.check(root, manifest)
        with self.assertRaises(ValueError):
            MANIFEST.create(root, root/"manifest.json")

    def test_no_overwrite(self):
        root, manifest = self.release()
        before = manifest.read_bytes()
        with self.assertRaises(ValueError):
            MANIFEST.create(root, manifest)
        self.assertEqual(before, manifest.read_bytes())


class CliTests(TemporaryCase):
    def test_exit_codes(self):
        a = self.f32("a", [1.0])
        b = self.f32("b", [2.0])
        script = Path(F32.__file__)
        def run(*args):
            return subprocess.run([sys.executable, "-B", str(script), *map(str, args)],
                                  capture_output=True, text=True).returncode
        self.assertEqual(run(a, a), 0)
        self.assertEqual(run(a, b), 1)
        self.assertEqual(run(a, self.root/"missing"), 2)


if __name__ == "__main__":
    unittest.main()

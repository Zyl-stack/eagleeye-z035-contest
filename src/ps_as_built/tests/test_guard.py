import pathlib
import struct
import subprocess
import tempfile
import threading
import time
import sys

guard = pathlib.Path(sys.argv[-1]).resolve()
prefix = sys.argv[1:-1]

with tempfile.TemporaryDirectory(prefix="eagleeye_guard_") as directory:
    root = pathlib.Path(directory)
    dt = root / "dt"
    reserved = dt / "reserved-memory"
    node = reserved / "framebuf@1e000000"
    node.mkdir(parents=True)
    (reserved / "#address-cells").write_bytes(struct.pack(">I", 1))
    (reserved / "#size-cells").write_bytes(struct.pack(">I", 1))
    (node / "reg").write_bytes(struct.pack(">II", 0x1e000000, 0x210000))
    (node / "no-map").touch()
    memory = root / "memory"
    with memory.open("wb") as file:
        file.truncate(0x43011000)

    def write(address, value):
        with memory.open("r+b", buffering=0) as file:
            file.seek(address)
            file.write(struct.pack("<I", value))

    def run(mode="inspect", expected=0):
        p = subprocess.run(prefix + [str(guard), mode, "--memory", str(memory), "--dt-root", str(dt),
                            "--timeout-ms", "600"], capture_output=True, text=True, timeout=5)
        assert p.returncode == expected, (p.returncode, p.stdout, p.stderr)
        return p

    write(0x43010034, 1)
    write(0x43000004, 1)
    assert "VIDEO_COLD" in run(expected=10).stdout
    for base, control, status, size, address in [
        (0x43010000, 0x30, 0x34, 0xa0, 0xac), (0x43000000, 0, 4, 0x50, 0x5c)
    ]:
        write(base + control, 0x10003)
        write(base + status, 0x10000)
        for offset, value in [(size, 360), (size+4, 1920), (size+8, 1920)]:
            write(base+offset, value)
        for slot in range(3):
            write(base+address+4*slot, 0x1e000000+slot*0xb0000)
    run()
    write(0x43010034, 0x15810)
    assert "latched" in run().stdout
    run("wait", expected=20)  # Clear-zero slots must never pass readiness.
    for status in (0x10020, 0x10040, 0x10001):
        write(0x43010034, status)
        run(expected=20)
    write(0x43010034, 0x15810)
    write(0x430100a4, 640)
    run(expected=20)
    write(0x430100a4, 1920)
    (node / "reg").write_bytes(struct.pack(">II", 0x1e000000, 0x200000))
    run(expected=20)
    (node / "reg").write_bytes(struct.pack(">II", 0x1e000000, 0x210000))
    (node / "status").write_bytes(b"disabled\0")
    run(expected=20)
    (node / "status").unlink()
    stop = threading.Event()

    def animate():
        value = 1
        while not stop.is_set():
            write(0x1e000000 + (345600 // 31) * 31, value)
            value += 1
            time.sleep(0.08)

    with memory.open("rb") as file:
        file.seek(0x43010000)
        before = file.read(4096)  # Register evidence must not be cleared.
    thread = threading.Thread(target=animate)
    thread.start()
    try:
        assert "VIDEO_READY" in run("wait").stdout
    finally:
        stop.set()
        thread.join()
    with memory.open("rb") as file:
        file.seek(0x43010000)
        assert file.read(4096) == before
    (reserved / "#address-cells").write_bytes(struct.pack(">I", 2))
    (reserved / "#size-cells").write_bytes(struct.pack(">I", 2))
    (node / "reg").write_bytes(struct.pack(">QQ", 0x1e000000, 0x210000))
    run()
print("PASS guard: cold/running geometry, fatal errors, reserved-memory 32/64-bit, timeout, live center changes, read-only registers")

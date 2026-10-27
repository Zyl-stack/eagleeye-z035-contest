import pathlib
import struct
import subprocess
import tempfile
import threading
import time
import sys

program = pathlib.Path(sys.argv[1]).resolve()
base = 0x40000000
with tempfile.TemporaryDirectory(prefix="fovea_mock_") as folder:
    image = pathlib.Path(folder) / "mmio"
    with image.open("wb") as file:
        file.truncate(base + 4096)

    def write(offset, value):
        with image.open("r+b", buffering=0) as file:
            file.seek(base + offset)
            file.write(struct.pack("<I", value))

    def read(offset):
        with image.open("rb") as file:
            file.seek(base + offset)
            return struct.unpack("<I", file.read(4))[0]

    p = subprocess.run([str(program), str(image), "2"], capture_output=True, text=True)
    assert p.returncode == 2 and "signature" in p.stderr
    write(0x64, 0x45454f31)
    write(0x04, 1)
    p = subprocess.run([str(program), str(image), "2"], capture_output=True, text=True)
    assert p.returncode == 2 and "pending" in p.stderr
    assert read(0x00) == 0
    write(0x04, 0)
    stop = threading.Event()

    def accept_once():
        while not stop.is_set():
            if read(0x00) == 3:
                write(0x04, 0x22)  # done + active Lanczos
                return
            time.sleep(0.001)

    simulator = threading.Thread(target=accept_once)
    simulator.start()
    try:
        p = subprocess.run([str(program), str(image), "2"], capture_output=True, text=True, timeout=3)
        assert p.returncode == 0, (p.stdout, p.stderr)
        assert "mode=2" in p.stdout
        assert read(0x08) == 7
        assert read(0x0c) == 0  # no fabricated camera frame id
        assert read(0x10) == 0  # no fabricated target tracker id
        assert read(0x14) == (50 << 16) | 100
        assert read(0x18) == (180 << 16) | 320
        assert read(0x1c) == 2
        assert read(0x00) == 3
    finally:
        stop.set()
        simulator.join(timeout=3)
    write(0x04, 0)
    write(0x00, 0)
    stop.clear()

    def accept_direct():
        while not stop.is_set():
            if read(0x00) == 3:
                write(0x04, 0x02)  # done + active direct mode
                return
            time.sleep(0.001)

    simulator = threading.Thread(target=accept_direct)
    simulator.start()
    try:
        p = subprocess.run([str(program), str(image), "0"], capture_output=True, text=True, timeout=3)
        assert p.returncode == 0 and "mode=0" in p.stdout
        assert read(0x1c) == 0
    finally:
        stop.set()
        simulator.join(timeout=3)
print("PASS Fovea MMIO signature, pending guard, register layout, mode acknowledge and direct restore")

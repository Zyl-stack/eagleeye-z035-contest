import os
import pathlib
import signal
import subprocess
import tempfile
import time
import sys

runner = pathlib.Path(sys.argv[1]).resolve()
shell = sys.argv[2:] or ["sh"]
with tempfile.TemporaryDirectory(prefix="eagleeye_runner_") as directory:
    root = pathlib.Path(directory)
    env = dict(os.environ, SESSION_LOCK=str(root / "session_lock"))
    worker = root / "worker.sh"
    worker.write_text("echo 'PERSON confidence=0.5'\necho 'TIME capture_ms=1'\nexit 37\n")
    p = subprocess.run(shell + [str(runner), str(worker), str(root / "results")],
                       env=env, text=True, capture_output=True, timeout=5)
    assert p.returncode == 37, p
    run = next((root / "results").iterdir())
    assert (run / "exit_code.txt").read_text().strip() == "37"
    assert (run / "summary.txt").read_text().strip() == "inferences=1 boxes=1 empty_inferences=0"
    assert "PERSON confidence=" in (run / "board.log").read_text()
    assert not (root / "session_lock").exists()
    assert not (run / "output.pipe").exists()
    worker.write_text("trap 'exit 130' TERM INT HUP\necho start\nwhile :; do sleep 0.1; done\n")
    p = subprocess.Popen(shell + [str(runner), str(worker), str(root / "long")], env=env,
                         text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        for _ in range(50):
            if list((root / "long").glob("run_*/board.log")):
                break
            time.sleep(0.02)
        else:
            raise AssertionError("runner never started")
        blocked = subprocess.run(shell + [str(runner), str(worker), str(root / "blocked")],
                                 env=env, text=True, capture_output=True, timeout=5)
        assert blocked.returncode == 3
        p.send_signal(signal.SIGTERM)
        p.communicate(timeout=5)
        assert p.returncode == 143
        assert not (root / "session_lock").exists()
        run = next((root / "long").iterdir())
        assert (run / "exit_code.txt").read_text().strip() == "143"
        assert not (run / "output.pipe").exists()
    finally:
        if p.poll() is None:
            p.kill()
            p.wait()
print("PASS runner: logs, real child failure exit code, concurrent-run exclusion, cancellation cleanup")

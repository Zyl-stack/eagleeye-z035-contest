# AX7020 OV5640 PicoDet v3 board test

This is an observation-first test package. It does not contain BOOT.BIN,
image.ub, system.dtb, XSA or a bitstream. Keep the known-good SD package for
rollback. Do not change PL video timing or VDMA geometry during this test.

## 1. Prepare and verify

1. Extract `eagleeye_picodet_v3.tar.gz` into a separate directory on the
   board. Run `sha256sum -c SHA256SUMS` there.
2. Confirm the existing board still has HDMI video. Run as root:

   ```sh
   ./board_diagnostics.sh ./logs/diagnostics_before.log
   ```

   This is read-only. It does not acknowledge status bits.

## 2. Capture-only probe: 100 cycles

Use this first command after boot when the camera is already showing HDMI:

```sh
mkdir -p logs
./vdma_probe --initialize-vdma --cycles 100 --period-ms 40 \
  | tee logs/vdma_probe_100.log
```

It does not load Paddle-Lite and does not run PicoDet. Its default W1C mask is
zero: preserve the first S2MM error scene. Each `PROBE` line records S2MM/MM2S
control and status, PARK_PTR, frame-pointer status, WRTSTR/WRTREF, READSTR/
READREF, dimensions, start addresses, and a 4 KiB composite CRC sampled across
each DDR slot. `changed=1` or a changing CRC proves an observed state change;
PARK_PTR alone does not prove a new frame.

Only after preserving the first log may you run a separate, explicitly marked
W1C observation, for example `--w1c-mask 0x00005000`. Never clear
`VDMAIntErr`, `SOFLateErr`, line errors, or the error interrupt before saving
the original probe log.

## 3. Inference test: 20 runs

After the probe log shows no fatal error, run:

```sh
ITERATIONS=20 ./run_board.sh
```

Acceptance requires 20 increasing `SNAPSHOT frame_sequence` values, no stale
frame returned as new, a `SNAPSHOT_COPY` measurement for each frame, no VDMA
error or deadlock, and coherent images. `TIME ... FPS` is per-inference
throughput, not camera frame rate.

Then run the diagnostic again and retain both logs:

```sh
./board_diagnostics.sh ./logs/diagnostics_after.log
```

## 4. Long run and shutdown

Run capture-only with at least 100 cycles while moving an object through the
camera scene. Then use `ITERATIONS=0 ./run_board.sh`, press Ctrl+C, and check
that it exits cleanly. Start it once more to verify a clean second startup.

Return the two diagnostics logs, `vdma_probe_100.log`, application log, counts
of increasing frame/inference sequences, CRC changes, capture/copy/preprocess/
inference timing, and any `VDMA ERROR` register dump.

## Stop conditions

Stop and report before changing software geometry if any of these appear:

- `VDMAIntErr`, `SOFLateErr`, SOF/EOL error, or error interrupt in DMASR.
- The actual camera/PL stream does not match VSIZE=360, HSIZE=1920,
  STRIDE=1920, RGB888, and the three expected addresses.
- No safe new slot, CRC evidence of overwrite, repeated snapshots presented as
  new frames, tearing, or a thread shutdown failure.

The observed S2MM line/frame timing, TLAST/SOF behavior, cache coherence and
display/write handoff remain board-dependent. This package is not a claim that
continuous acquisition is hardware verified.

# EagleEye: FPGA-assisted video review (draft)

## 1. Problem and contribution

The project explores a camera-based aid for reviewing fast-moving sports
scenes. It keeps the full video visible while a separate ROI view can enlarge
and process the region selected by PicoDet or, during pause, by a USB mouse.
The design is a review aid, not an autonomous officiating decision system.

The submitted scope is the board-verified Eyes system.

## 2. Platform and partition

Target: Zynq-7000 `xc7z035ffg900-2` with OV5640 camera, HDMI display, USB mouse,
keys and two PWM servo outputs. Vivado 2022.2 was used; explain this version
choice and verify the clean-source script before final submission.

```text
OV5640 -> PL video/VDMA -> DDR -> HDMI full view
                      |       |
                      |       +-> PS PicoDet -> ROI register control
                      +-> PL ROI scaling / edge / OSD -> HDMI ROI view
USB mouse, KEY0/KEY1 -> pause/mode/manual ROI control
ROI position -> PL two-axis PWM -> mechanical eye
Captured RGB -> PL bilinear/normalize preprocessor -> PS model tensor
```

The RTL, XDC, BD, custom IP and PS application sources are under `src/`.
The preprocessing IP accepts 640x360 RGB888 and emits three 320x320 FP32
NCHW streams. Its DMA and cache maintenance contract should be documented
with a register/DDR address table in the final report.

## 3. Verified behavior and limits

The user reported successful board operation of camera/HDMI, automatic ROI,
pause, mode switching, mouse-controlled review and mechanical-eye tracking.
`board/DEPLOY_STATUS_ZH.md` records the verified Eyes BOOT identity and the
pause/status board check. A final uncut demonstration recording and test matrix
should be captured and added separately.

The preprocessing IP has a captured input and complete FP32 Golden tensor in
`data/`, source testbenches in `src/ip_repo/preproc/tb/`, and checkers under
`skill/preproc-fp32-golden/`. Do not label its simulation or the older
`board/attach_100.log` as a final Eyes end-to-end FPS measurement.

## 4. Resources and timing

From the copied Eyes placed utilization report: 24,789 slice LUTs, 28,234
slice registers, 303 BRAM tiles, 49 DSPs. The copied routed timing report and
deployment record report WNS +0.505 ns and WHS +0.051 ns. Cite the exact report
revision and tool version in the final table; do not mix these numbers with an
older implementation.

## 5. Performance table to complete

| Configuration | Captured frames | Preprocess ms | Predictor ms | Detection update FPS | HDMI mode | Evidence |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| CPU preprocessing baseline | Pending same-scene run | Pending | Pending | Pending | Pending | Pending |
| FPGA preprocessing, Eyes release | Pending same-scene run | Pending | Pending | Pending | Pending | Pending |

Use one model hash, thread count, threshold, pixel order and camera scene for
the A/B rows. Report mean, p95 and sample count, and separate video refresh
from PicoDet update rate. The older `attach_100.log` is a useful historical
baseline but does not satisfy this paired comparison.

## 6. Attribution and reproducibility

List each team's own RTL/IP contribution separately from ALINX, Digilent,
PicoDet and Paddle-Lite components. Review redistribution licenses before
publishing the model, binaries or third-party IP. The root README describes
the remaining external PetaLinux/Paddle-Lite build inputs. A clean Vivado
recreation and board replay have not yet been run from this compact directory.

## 7. AI collaboration and skills

The two submitted `skill/` entries document FP32 Golden checking and ROI
review control. Lanczos RTL and historical verification records remain in the
engineering sources and archived evidence; the legacy pixel test failed and
its applicability to the current RTL remains unverified. Add dated excerpts of actual prompt, response,
observed failure, correction and validation to the final report. Examples to
index: KEY0 black-screen diagnosis, USB PHY/mouse enumeration, SD filesystem
recovery, and preprocessing Golden/latency measurement. Do not invent missing
chat transcripts or claim a PYNQ-specific Skill: this release uses PetaLinux,
not PYNQ.

## 8. Reproduction

1. Verify `SHA256SUMS.txt` with `build/verify_package.ps1`.
2. Build/validate the Vivado project from `build/recreate_vivado.tcl` using
   Vivado 2022.2; record any IP upgrade or warning resolution.
3. Rebuild PS binaries with the matching PetaLinux sysroot and Paddle-Lite
   handoff, or use the recorded board application archive for replay.
4. Use an independent SD card with the four files named in `board/README.md`.
5. Capture serial output, camera/HDMI behavior and one paired performance run.

These are instructions for the eventual controlled reproduction. This draft
does not authorize writing any currently stable SD card.

# EagleEye Z035: camera-assisted review system

This is a compact source and evidence snapshot for the 2026 FPGA contest's
self-selected junior track. It covers the board-verified Eyes video design,
PicoDet PS inference, FPGA image preprocessing, ROI review controls, and the
two servo signal generators.

## Hardware and tools

- Board/device: EagleEye Z035, `xc7z035ffg900-2`, OV5640 camera, HDMI output.
- FPGA project: Vivado 2022.2. The contest guide recommends newer versions;
  this older version and the source-rebuild procedure must be declared in the
  final report.
- PS software: ARMv7 PetaLinux 2022.2 and Paddle-Lite. Rebuilding the PS
  executable needs the matching PetaLinux cross-toolchain/sysroot and the
  ARMv7 Paddle-Lite include/static-library handoff. Those large third-party
  build dependencies are not copied into this snapshot.

## Contents

| Directory | Contents |
| --- | --- |
| `src/vivado/` | BD, RTL, XDC and small XCI source snapshot from the Eyes project |
| `src/ip_repo/` | Required OV5640, dynamic-clock and preprocessing custom IP sources |
| `src/servo/` | Eyes top wrapper, two-channel PWM RTL, pin constraint and testbench |
| `src/ps/` | Camera/PicoDet/mouse application sources and build/run scripts |
| `sim/` | Entry points for the included testbenches and checkers |
| `build/` | Vivado recreation script and selected final implementation reports |
| `board/` | Verified BOOT candidate, matching SD files, deployment notes and logs |
| `data/` | PicoDet model and one captured preprocessing test frame/Golden tensor |
| `skill/` | Three reusable methods/checkers, each with its own README |
| `report/` | Report outline, AI collaboration cases and remaining evidence checklist |

The original Vivado `.xpr` is not used here because it refers to paths outside
this directory. `build/recreate_vivado.tcl` creates a new local project from
the copied sources. It has **not yet been rerun from this cleaned directory**;
the included implementation reports are from the board-verified source project.
Do not label this snapshot independently reproduced until that check passes.

## Board files

`board/BOOT.BIN` is the Eyes pause/status candidate with SHA256
`54003cd2490c38d5caf208791e2995de7112a8e3b5b826a39b0f1811735e2765`.
The matching SD files are `board/sd_boot/image.ub`, `boot.scr`, and the
mouse-enabled `linux_camera_smooth_trial_v7_20261013.tar.gz`. The BOOT has
user-reported board success; the USB-host image and mouse application have
their own recorded board validation. See `board/README.md` before deployment.
No files in this directory are automatically written to an SD card.

## Evidence boundaries

- The placed Eyes report records 24,789 slice LUTs, 28,234 slice registers,
  303 BRAM tiles and 49 DSPs. The routed timing report and route-status report
  are copied unchanged under `build/`.
- `board/attach_100.log` is an older CPU-preprocessing camera run and is **not**
  an Eyes release FPS measurement. The current preprocessing FPGA timing and
  interactive functions need a clean, labeled final measurement/log for the
  submitted report.
- HDMI video refresh and PicoDet detection update rate are different metrics.
  Do not present HDMI 30/60 Hz as inference FPS.
- A complete standalone report, demo recording, and freshly rerun clean-source
  rebuild are pending. See `report/README.md`.

## Redistribution

The PicoDet model, Paddle-Lite binary, and third-party OV5640/dynamic-clock IP
are not claimed as original work. Review their licenses and attribution before
making this folder public. `src/ps/licenses/NOTICE.txt` records the existing
model/library provenance. Add the team's chosen license only after that review.

# A/C Integration Plan

## Confirmed implementation

A commit `729a8d0` now provides the custom `fovea_axil_ctrl`, a connected Vivado Block Design, Address Editor assignment, and exported XSA. Its register layout matches the C `register_map.h`.

The C adapter remains responsible for converting Scheduler, Motion Predictor, or Joint ROI output into the hardware contract:

```text
center_x = dynamic_roi_x + dynamic_roi_w / 2
center_y = dynamic_roi_y + dynamic_roi_h / 2
fixed_x  = clamp(center_x - 160, 0, 320)
fixed_y  = clamp(center_y - 90, 0, 180)
fixed_w  = 320
fixed_h  = 180
```

Mode is `0=BYPASS`, `1=BILINEAR 2x`, `2=LANCZOS 2x`.

## C write order

`platform_io_vitis.c` uses the Adapter-generated sequence:

```text
REQUEST_ID
SOURCE_FRAME_ID
TARGET_ID
ROI_XY
ROI_WH = (320, 180)
ENHANCE_MODE
OUTPUT_BUFFER
DEADLINE
CONTROL = ENABLE | COMMIT   (always last)
```

The extra OUTPUT_BUFFER and DEADLINE writes are supported by A even though the first A smoke-test note treats them as reserved/currently fixed behavior.

## First hardware communication test

The exported A XSA has now generated the `eagleeye_platform` standalone BSP. Its `xparameters.h` provides `XPAR_FOVEA_AXIL_CTRL_0_BASEADDR` (`0x40000000`) and `XPAR_FOVEA_AXIL_CTRL_0_HIGHADDR` (`0x40000FFF`). `EAGLEEYE_FOVEA_BASEADDR` maps to the generated BASEADDR macro; no numeric address is hardcoded in C source.

The first request is:

```text
request_id      = 1
source_frame_id = 1
target_id       = 1 (test BALL ID)
ROI             = (160, 90, 320, 180)
mode            = 1 (BILINEAR 2x)
```

Expected UART sequence:

```text
FOVEA REQUEST SENT
ROI=(160,90,320,180)
MODE=BILINEAR
WAITING FOR FRAME LATCH
REQUEST LATCHED
```

`REQUEST LATCHED` proves only this path:

```text
PS -> AXI-Lite -> fovea_axil_ctrl -> frame-boundary command latch -> STATUS.done -> PS
```

It does not prove Attention, detection, image quality, a complete enhanced frame, HDMI behavior, or the full football workflow.

## Remaining manual steps

1. A/C agree which checked-in XSA is the handoff artifact; current review used `A_Fovea/fpga_vivado/project/system_wrapper.xsa`.
2. Regenerate or update the Vitis platform from that XSA.
3. Regenerate the standalone BSP.
4. Open the generated `xparameters.h` and record the exact Fovea BASEADDR macro.
5. Define `EAGLEEYE_FOVEA_BASEADDR` as that macro in the Vitis application settings or platform header configuration.
6. Add `a_fovea_adapter.c`, `platform_io_vitis.c`, and `vitis_axi_smoke_test.c` to the application; do not link `platform_io_sim.c`.
7. Build, program the bitstream, run the application, and capture UART output.
8. Confirm STATUS.done and FRAME_SAFE_LATCHED after a frame boundary.

## Boundary

The source-level C/A interface is aligned. Physical-board communication is not yet verified. Any discrepancy found during the first board test must be recorded before changing the frozen register map or core algorithms.

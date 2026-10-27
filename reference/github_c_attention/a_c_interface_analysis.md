# A/C Interface Analysis

Date: 2026-09-15

Source reviewed: `origin/a-fovea` commit `729a8d0 Add files via upload`.
The A branch was inspected read-only and was not modified.

## A implementation now present

- RTL: `A_Fovea/fpga_vivado/rtl/fovea_axil_ctrl.v`
- Vivado Block Design: `A_Fovea/fpga_vivado/project/system.bd`
- Exported hardware: `A_Fovea/fpga_vivado/project/system_wrapper.xsa`
- Interface note: `A_Fovea/fpga_interface/README.md`

`fovea_axil_ctrl` is a custom AXI4-Lite slave with 32-bit data and 6-bit address ports. It uses `s_axi_aclk` and active-low `s_axi_aresetn`.

## Actual A register map

| Offset | Register | Width / encoding | C match |
|---:|---|---|---|
| `0x00` | CONTROL | bit0 ENABLE, bit1 COMMIT, bit2 CANCEL, bit3 CLEAR_ERROR | YES |
| `0x04` | STATUS | bit0 busy/pending, bit1 done, bit2 error, bits[5:4] active_mode | YES |
| `0x08` | REQUEST_ID | 32 bits | YES |
| `0x0C` | SOURCE_FRAME_ID | 32 bits | YES |
| `0x10` | TARGET_ID | low 16 bits | YES |
| `0x14` | ROI_XY | low16 x, high16 y | YES |
| `0x18` | ROI_WH | low16 w, high16 h | YES |
| `0x1C` | ENHANCE_MODE | low 2 bits | YES |
| `0x20` | OUTPUT_BUFFER | low 16 bits | YES |
| `0x24` | DEADLINE | 32 bits | YES |
| `0x28` | CYCLE_COUNT | 32-bit free-running counter | YES |
| `0x2C` | QUALITY_FLAGS | bits 0 through 5 match C masks | YES |

The A offsets and bit definitions match `include/register_map.h` exactly. The C frozen register map was not changed.

## Command and done behavior

On a CONTROL write with COMMIT set, A clears old done/quality state, sets pending, and copies the shadow ROI, target, mode, and output-buffer values to command outputs. `roi_cmd_valid_o` remains asserted while pending and enabled.

`video_mode_switch_top` accepts the command only when it accepts the start of a new input frame. Its `roi_cmd_accept` drives `fovea_axil_ctrl.cmd_accept_i`. A then clears pending, sets STATUS.done, and sets `QUALITY_FLAGS.FRAME_SAFE_LATCHED`.

Therefore:

```text
STATUS.done = REQUEST_LATCHED_AT_FRAME_BOUNDARY
STATUS.done != IMAGE_PROCESSING_COMPLETE
```

## ROI and mode path

- Input coordinate space is `640 x 360`.
- `roi_x` and `roi_y` are top-left coordinates.
- The active video ROI is fixed at `320 x 180` in `video_mode_switch_top` / `fovea_canvas_1280x720_top` parameters.
- A clamps x to `0..320` and y to `0..180`.
- ROI_WH is implemented as readable/writable registers and exposed by the control module, but its width/height outputs are not connected in the current BD. The video path therefore continues to use fixed `320 x 180` parameters.
- Mode is two bits: 0 BYPASS, 1 BILINEAR 2x, 2 LANCZOS 2x.
- `video_mode_switch_top` maps mode 0 to the direct canvas, mode 1 to Fovea with `use_lanczos=0`, and mode 2 to Fovea with `use_lanczos=1`.

The connected control path is:

```text
fovea_axil_ctrl.roi_cmd_valid/x/y/mode
    -> video_mode_switch_top.roi_cmd_valid/x/y/mode
    -> frame-boundary latch
    -> fovea_canvas_1280x720_top roi_x/roi_y/use_lanczos
```

## Vivado and XSA evidence

The Block Design contains `processing_system7_0`, enables `M_AXI_GP0`, and connects it through `processing_system7_0_axi_periph/M05_AXI` to `fovea_axil_ctrl_0/S_AXI`. FCLK, active-low reset, command outputs, command acceptance, and active-mode feedback are connected.

Vivado Address Editor data in both `system.bd` and the exported XSA identifies:

```text
instance     fovea_axil_ctrl_0
base         0x40000000
high         0x40000FFF
range        4K
```

The XSA contains `system.hwh` and an embedded `system_wrapper.bit`. The regenerated `eagleeye_platform` standalone BSP confirmed the generated C macros:

```c
#define XPAR_FOVEA_AXIL_CTRL_0_BASEADDR 0x40000000
#define XPAR_FOVEA_AXIL_CTRL_0_HIGHADDR 0x40000FFF
```

`EAGLEEYE_FOVEA_BASEADDR` now maps to the generated BASEADDR macro unless an application explicitly overrides it.

## Current compatibility result

The C request packing, fixed ROI Adapter, mode values, CONTROL write order, STATUS masks, and quality masks match the A RTL. No new compatibility translation is required.

Hardware status: `READY FOR FIRST VITIS COMMUNICATION SMOKE TEST AFTER BSP REFRESH`, but `NOT HARDWARE VERIFIED`. No PS-to-PL transaction or UART output has been observed on the physical board yet.

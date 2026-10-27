# FPGA Fovea AXI-Lite Interface

Coordinate system:

- Input video coordinate space: 640x360.
- roi_x and roi_y are ROI top-left coordinates, not center coordinates.
- Fixed ROI size in the current PL pipeline: 320x180.
- Valid range:
  - roi_x: 0 to 320.
  - roi_y: 0 to 180.

Mode definition:

- 0: BYPASS / direct display.
- 1: Bilinear 2x.
- 2: Lanczos 2x.

AXI-Lite slave:

- Instance name: fovea_axil_ctrl_0.
- Base address: 0x40000000.
- Address range: 4K.
- High address: 0x40000FFF.

Register map:

| Offset | Name | Description |
| --- | --- | --- |
| 0x00 | CONTROL | bit0 ENABLE, bit1 COMMIT, bit2 CANCEL, bit3 CLEAR_ERROR |
| 0x04 | STATUS | bit0 busy, bit1 done, bit2 error, bits[5:4] active_mode |
| 0x08 | REQUEST_ID | software request id |
| 0x0C | SOURCE_FRAME_ID | source frame id |
| 0x10 | TARGET_ID | selected target id |
| 0x14 | ROI_XY | bits[15:0] roi_x, bits[31:16] roi_y |
| 0x18 | ROI_WH | bits[15:0] roi_w, bits[31:16] roi_h |
| 0x1C | ENHANCE_MODE | 0 BYPASS, 1 Bilinear, 2 Lanczos |
| 0x20 | OUTPUT_BUFFER | reserved, first version fixed to 0 |
| 0x24 | DEADLINE | reserved |
| 0x28 | CYCLE_COUNT | free-running PL counter |
| 0x2C | QUALITY_FLAGS | status flags |

QUALITY_FLAGS:

- bit0 ROI_CLIPPED.
- bit1 MODE_FALLBACK.
- bit2 TARGET_SWITCH.
- bit3 REQUEST_CANCELLED.
- bit4 FRAME_SAFE_LATCHED.
- bit5 OUTPUT_BUFFER_VALID.

Recommended PS write order:

1. Write REQUEST_ID.
2. Write SOURCE_FRAME_ID.
3. Write TARGET_ID.
4. Write ROI_XY.
5. Write ROI_WH. The current PL keeps the value but still uses fixed 320x180.
6. Write ENHANCE_MODE.
7. Write CONTROL = ENABLE | COMMIT.
8. Poll STATUS.done.

Done meaning:

- done means the request has been accepted and latched by PL at a frame boundary.
- done does not mean the stream processor has finished a whole enhanced frame.

First-stage smoke test target:

- PS writes roi_x, roi_y, and mode.
- HDMI ROI frame and enhanced region change accordingly.
- PS reads STATUS.done = 1 after PL frame-boundary latch.

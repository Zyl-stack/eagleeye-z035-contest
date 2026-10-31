# B -> C Interface V1 (Frozen)

## Protocol

Magic header: `EE_B_TARGET_STREAM_V1`

```text
STREAM_INFO,<source_width>,<source_height>
FRAME,<frame_id>,<target_count>
TARGET,<frame_id>,<target_id>,<target_type>,<x>,<y>,<w>,<h>,<confidence>,<valid>,<vx>,<vy>,<velocity_valid>,<age>,<miss_count>
END_FRAME
```

`STREAM_INFO` is optional and, when present, appears once after the magic header and before the first `FRAME`.

## Frozen semantics

- `source_width,source_height` describe the original video-frame coordinate system. Model input dimensions and source-frame dimensions are recorded separately.
- If `STREAM_INFO` is absent, legacy V1 input defaults to a 640x360 source frame.
- `x,y,w,h` and `vx,vy` are expressed in the declared source-video coordinates.
- Bounding box is `x,y,w,h`; `x,y` is the top-left corner.
- A non-empty box must remain completely inside the source frame.
- `confidence` is an integer from 0 through 100.
- `valid=1` means currently visible/valid.
- `valid=0` means temporarily missing or occluded; the retained ID still identifies the same track.
- Target types are `UNKNOWN=0`, `BALL=1`, `PLAYER=2`, `REFEREE=3`.
- `target_id` values are 1 through 65534. Zero and 65535 are reserved.
- `target_count` is at most 16 (`EE_MAX_TARGETS`). IDs must be unique within a frame.
- `vx,vy` are signed pixels per frame.
- `velocity_valid=1` means velocity came from consecutive valid observations.
- `age` is the track age in frames. `miss_count` is the consecutive missing-frame count.
- Frame IDs must be strictly increasing. A duplicate or older frame is rejected.

## C canonical coordinates

The B-to-C adapter is the sole resolution boundary. It converts every accepted source box and velocity to C's fixed 640x360 coordinate system before constructing `EE_GlobalTargetList`; Attention, Scheduler, Possession, Motion Predictor, ROI, and Fovea code therefore remain resolution-independent.

- Boxes are converted by scaling both corners (`x`, `x+w`; `y`, `y+h`) and then deriving `w,h`, so edge rounding cannot create an inconsistent box.
- Converted boxes are clamped to the canonical frame and must remain non-empty. A source box that becomes empty after conversion is rejected by the adapter.
- `vx` scales by `640/source_width` and `vy` by `360/source_height`, preserving sign with safe saturation to `int16_t`.
- The coordinate conversion uses only the declared source-frame dimensions.

V1 field meanings must not be changed in place. Future `team_id`, `jersey_id`, `field_zone`, or timestamp data must use optional extension fields with an explicit compatibility rule, or a new `EE_B_TARGET_STREAM_V2` protocol.

## Parser behavior

The C reader rejects malformed headers, invalid/duplicate/late `STREAM_INFO`, duplicate/non-monotonic frames, count overflow or mismatch, missing `END_FRAME`, invalid target types, confidence outside 0..100, empty/out-of-frame source boxes, duplicate/reserved IDs, and TARGET/FRAME ID mismatch. Errors are returned as `-1` and exposed through `ee_bc_stream_error()`.

The parser is PC software integration only. It does not imply a trained detector, camera input, AXI-Lite transfer, or FPGA validation.

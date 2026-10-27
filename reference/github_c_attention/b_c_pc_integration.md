# B -> C PC Integration

`include/bc_adapter.h` and `src/bc_adapter.c` form the first PC-only bridge from B's ASCII target stream to C's existing attention pipeline. They do not change the frozen `EE_GlobalTargetDescriptor` or any AXI-Lite interface.

Pipeline:

`B ASCII stream -> EE_BParsedFrame -> EE_GlobalTargetList -> Possession -> Attention -> Scheduler -> EE_FoveaRequest`

## Input protocol

The reader accepts B's `EE_B_TARGET_STREAM_V1` records:

```text
FRAME,<frame_id>,<count>
TARGET,<frame_id>,<target_id>,<BALL|PLAYER|REFEREE|UNKNOWN>,<x>,<y>,<w>,<h>,<confidence>,<valid>,<vx>,<vy>,<velocity_valid>,<age>,<miss_count>
END_FRAME
```

The adapter enforces non-decreasing frame IDs, at most `EE_MAX_TARGETS` targets, unique nonzero IDs, `0..100` confidence, and 640x360 in-frame non-empty bounding boxes. It reports rejected records to stderr and makes no hardware call.

`velocity_valid`, `age`, and `miss_count` remain B-local in `EE_BParsedTarget`; C's frozen descriptor is unchanged. C derives `is_ball`, maps BALL to `EE_CLASS_UNKNOWN` and other B types to `EE_CLASS_PEDESTRIAN`, uses `EE_FIELD_NORMAL` until calibrated field zones are available, and converts B confidence to C's 0..255 uncertainty scale.

## Run

From a Visual Studio Developer Command Prompt at repository root:

```bat
run_tests_msvc.bat
build\bc_stream_demo.exe
```

The demo reads `examples/b_target_stream_v1_sample.txt` and prints each target, attention result, selected target, and generated Fovea ROI. The sample preserves PLAYER IDs 1 and 2 and BALL ID 3; BALL is invalid on frame 8 and returns with ID 3 on frame 9.

## Boundary

This is only B->C PC parsing and adaptation. It does not alter A-side registers, the Vitis workspace, FPGA logic, or C's state-machine/attention equations.

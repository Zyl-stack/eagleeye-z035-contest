# B Detection -> C Attention Interface V1

## Responsibility boundary

B emits every detected PLAYER, BALL, and optional REFEREE for every frame. B does not select the focus target and does not calculate possession or attention. C owns Possession Score, Attention Competition, Switch Margin/Hold Time, Motion Predictor policy, Joint ROI, and the final Fovea Request.

The minimum B descriptor is `frame_id`, stable `target_id`, `target_type`, `bbox`, `confidence`, and `valid`. `vx/vy`, `velocity_valid`, `age`, and `miss_count` are B-side transport diagnostics. C's `bc_adapter` parses the transport and decides which diagnostics are retained internally; B must not change C's frozen `EE_GlobalTargetDescriptor` ABI.

## Stable ID contract

- A target keeps the same ID while it is continuously tracked.
- PLAYER and BALL tracks are matched separately and can never exchange IDs.
- BALL uses a wider motion/reacquisition gate than PLAYER because a pass can move many pixels per frame.
- A missed target remains in the stream with the same ID and `valid=0` for a short hold window.
- Reacquisition inside that window restores `valid=1` without allocating a new ID.
- After `max_missed` frames the track is released; a later detection may receive a new ID.
- `target_id=0` is reserved and is never emitted. The tracker allocates IDs from `1`.
- `0xFFFF` is reserved as `EE_INVALID_TARGET_ID` and is never emitted.

## Text stream V1

The first integration format is ASCII and line-oriented so both teams can inspect it without extra libraries:

```text
EE_B_TARGET_STREAM_V1
STREAM_INFO,640,360
FRAME,125,3
TARGET,125,7,PLAYER,210,120,35,78,92,1,3,0,1,26,0
TARGET,125,10,PLAYER,330,115,36,80,88,1,1,0,1,19,0
TARGET,125,1000,BALL,342,165,10,10,81,1,6,1,1,12,0
END_FRAME
```

`TARGET` fields are:

```text
record,frame_id,target_id,target_type,x,y,w,h,confidence,valid,vx,vy,velocity_valid,age,miss_count
```

The optional second line is:

```text
STREAM_INFO,source_width,source_height
```

`source_width` and `source_height` are the original input-video frame resolution used as the coordinate system for every `x/y/w/h` and `vx/vy` value in this stream. They are **not** YOLO's internal `imgsz` inference size. For example, a 640×360 video processed with `imgsz=1280` emits `STREAM_INFO,640,360`. C may normalize these pixel values to its internal 640×360 space before Attention processing.

`STREAM_INFO` is optional for backward compatibility: a legacy V1 file without it means the existing PC-demo default of `640x360`. New B video output always writes it. Coordinates are pixels in the declared original source view; `x/y` is the bounding-box top-left and `w/h` is width/height. Confidence is `0..100`; velocity is signed pixels/frame. One `FRAME` contains exactly the declared number of `TARGET` records.

`velocity_valid=1` means `vx/vy` came from at least one consecutive-frame match; it is `0` on a new track or while the descriptor is being held through an occlusion. `age` and `miss_count` are B-side lifecycle diagnostics. They are deliberately not part of the frozen C descriptor contract.

The fixed soccer type values match `include/eagleeye_types.h`: `BALL=1`, `PLAYER=2` (`UNKNOWN=0`, `REFEREE=3`). The current maximum is `16` simultaneous descriptors per frame. The stream is frame-synchronous: B emits one list per input detection frame, so its effective refresh rate is the detector/camera FPS; the 12-frame sample is not a real-time camera capture.

Python writes the format through `b_detection.stream.write_stream`. C reads it through its isolated `bc_adapter`, which validates the version, frame IDs, target count, numeric ranges, and target types before filling `EE_GlobalTargetList`.

## First integration run

```powershell
py -3 tools\generate_b_detection_demo.py --frames 12
py -3 -m unittest discover -s tests_py -v
```

The generated file is `build/b_target_stream_v1.txt`. It includes two players, one ball, a pass from the first player toward the second, and one-frame ball occlusion. Replace `detections_for_frame()` with YOLO/camera results later; the tracker and B/C stream contract remain unchanged.

## Current boundary

This branch provides the descriptor contract, stable-ID tracker, repeatable integration stream, and B-side tests. The C-side reader belongs to C's `bc_adapter`, so B does not modify C's frozen structs, scheduler, Fovea Request, or A/C and Vitis integration files. This branch does not claim a trained football detector, jersey-number recognition, team classification, or real-camera accuracy.

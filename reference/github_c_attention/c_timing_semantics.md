# C Timing Semantics

This document freezes the timing semantics used by the C pipeline before
A+C hardware bring-up. It documents the current behavior; it does not change
any algorithm parameter.

## Timing Baseline

- Source frame rate: `25 Hz`
- Source frame period: `40 ms`
- Detection frame stride: `2` source frames
- Nominal C update period: `80 ms`
- Nominal C input rate: `12.5 Hz`

The measured `detector_fps=0.817` is offline detector throughput. It is not the
real-time C input rate and must not be used to convert C timing parameters to
wall-clock durations.

## Timing Domains

The current C implementation uses two distinct timing domains.

### Detection-Update Domain

These parameters count C/controller updates. At the current nominal input
cadence, one unit is one detection update, or `80 ms`:

- `min_hold_frames`
- `fast_jump_frames`

### Source-Frame Domain

These parameters use source video frame IDs or source-frame intervals. At
`25 Hz`, one unit is `40 ms`:

- `deadline_frames`
- `prediction_horizon_frames`
- `nominal_input_frame_delta`
- `max_prediction_gap_frames`
- `processing_delay_frames`
- `request_delay_frames`
- `frame_latch_delay_frames`

## Current Parameter Semantics

| Parameter | Current value | Counter unit | Milliseconds per unit | Current nominal duration | Notes |
| --- | ---: | --- | ---: | ---: | --- |
| `min_hold_frames` | 4 | Detection update | 80 ms | 320 ms protected duration | With a valid incumbent, the earliest challenger switch is approximately 400 ms after lock because the hold condition uses `<=`. Switch Margin must also pass. |
| `fast_jump_frames` | 3 | Controller/detection update | 80 ms | 240 ms | The triggering update is the first FAST_JUMP update. |
| `deadline_frames` | 12 | Source frame | 40 ms | 480 ms deadline boundary | Timeout uses `current_frame_id > deadline_frame`; with stride 2, the first observable timeout is at 560 ms. |
| `prediction_horizon_frames` | 1 | Source frame | 40 ms | 40 ms ahead | The horizon remains in source-frame units when the detector stride is 2. |
| `nominal_input_frame_delta` | 2 for the current real stream | Source-frame delta | 40 ms | Represents an 80 ms observation cadence | It affects observation continuity only. It does not rescale velocity or prediction horizon. |
| `max_prediction_gap_frames` | 1 by default | Source-frame delta | 40 ms | Effective accepted cadence is controlled together with the nominal input delta | It prevents stale history from being treated as continuous motion. |
| `processing_delay_frames` | 0 | Source frame | 40 ms | 0 ms | Not yet measured on hardware. |
| `request_delay_frames` | 0 | Source frame | 40 ms | 0 ms | Not yet measured on hardware. |
| `frame_latch_delay_frames` | 0 | Source frame | 40 ms | 0 ms | Not yet measured on hardware. |

## Scheduler Hold

`min_hold_frames=4` counts Scheduler updates rather than source frame IDs.
At the nominal `12.5 Hz` C update rate, the protected duration is `320 ms`.

The implementation increments the hold counter and keeps the current valid
target while:

```text
hold_counter <= min_hold_frames
```

The selected target starts with a counter of zero. The four subsequent updates
at approximately 80, 160, 240, and 320 ms remain protected. The first update
that can accept a challenger is therefore approximately 400 ms after lock.
The challenger must still satisfy the configured Switch Margin. An invalid
incumbent may be replaced immediately.

## ROI Motion Controller

`fast_jump_frames=3` counts controller updates. The triggering update is
included, so FAST_JUMP is active for three updates. At the current nominal
cadence, its duration is `3 * 80 ms = 240 ms`.

## Fovea Timeout

`deadline_frames=12` is measured in source frame IDs. The nominal deadline
boundary is `12 * 40 ms = 480 ms`.

Timeout is reported only when:

```text
current_frame_id > source_frame_id + deadline_frames
```

At exactly 12 source frames after submission, the request has not timed out.
With stride 2, C observes offsets 2, 4, 6, 8, 10, 12, and 14. The first
observable timeout is therefore at offset 14, or `560 ms` after the request's
source frame.

## Motion Prediction

`prediction_horizon_frames=1` means one source frame ahead, or `40 ms`.

`nominal_input_frame_delta=2` permits observations separated by two source
frame IDs to be treated as continuous input. It only affects continuity
validation. It does not change the unit of `vx`/`vy`, multiply velocity by the
stride, or change the prediction horizon.

Velocity remains expressed per source frame. Therefore a horizon of one uses
one `vx`/`vy` step, even when detections arrive every two source frames.

## Latency Model

The following latency fields use source-frame units:

- `processing_delay_frames`
- `request_delay_frames`
- `frame_latch_delay_frames`

All three currently default to zero because real hardware latency has not yet
been measured. They must not be assigned guessed values before bring-up.

## Maintainability Note

The `_frames` suffix currently covers two different timing domains:

- Hold and FAST_JUMP count detection/controller updates.
- Deadline, prediction, cadence, prediction gap, and latency count source
  frames.

This is a **Medium maintainability issue**. The distinction is frozen in this
document, and the interfaces will not be renamed or refactored immediately
before hardware bring-up.

## Freeze Decision

Keep the current Hold, FAST_JUMP, timeout, and prediction-horizon values
unchanged.

Revisit the latency model and timeout only after A+C hardware testing measures:

- B-to-C cadence
- PS processing latency
- AXI request latency
- Frame-latch latency
- `done` and `error` behavior

Until those measurements exist, no claim should be made that the current
values represent measured end-to-end latency.

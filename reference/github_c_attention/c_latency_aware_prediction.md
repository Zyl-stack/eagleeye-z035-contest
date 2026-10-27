# Latency-Aware ROI Prediction

## Purpose

A Fovea ROI command is not useful at the instant C computes it. The target may move while C processes a global frame, sends the request, and waits for the A-side frame-boundary latch. For a moving ball, a current-position ROI can therefore arrive behind the target.

This stage keeps the existing integer constant-velocity predictor and makes its horizon explicit. It does not add a Kalman filter, learned scanpath policy, Fovea feedback, or a change to Attention/Scheduler behavior.

## Data Path

```text
Selected target
  -> ee_motion_predict_with_latency()
  -> desired ROI origin / center
  -> EE_ROISmoother
  -> ee_a_fovea_adapt_request()
  -> FoveaRequest
```

The PC simulation main path and the C visual bridge both use the latency-aware API. The scheduler still chooses the target exactly as before; this feature only changes where the selected target is expected to be when the ROI is used.

## Configuration

`EE_CConfig` owns the model:

```c
typedef struct {
    uint16_t processing_delay_frames;
    uint16_t request_delay_frames;
    uint16_t frame_latch_delay_frames;
} EE_LatencyModel;
```

The total compensation is:

```text
total_prediction_delay_frames = processing_delay_frames
                              + request_delay_frames
                              + frame_latch_delay_frames

effective_horizon_frames = prediction_horizon_frames
                         + total_prediction_delay_frames
```

Defaults are deliberately conservative and preserve V1 behavior:

```text
prediction_horizon_frames = 1
processing_delay_frames = 0
request_delay_frames = 0
frame_latch_delay_frames = 0
```

All delay values are **CONFIGURED / ESTIMATED**, not measured hardware latency. They must remain zero until board timing is characterized.

## Prediction Formula

`vx` and `vy` are signed pixels per frame, supplied by the B-to-C target stream. The predictor uses target bounding-box origin coordinates:

```text
predicted_x = saturate_u16(current_x + vx * effective_horizon_frames)
predicted_y = saturate_u16(current_y + vy * effective_horizon_frames)
```

`ee_motion_predict()` remains available and retains its original one-frame behavior. New code calls:

```c
ee_motion_predict_with_latency(target,
                               config.prediction_horizon_frames,
                               &config.latency_model,
                               &prediction);
```

The predictor still requires a prior valid observation of the same `target_id`; a newly appearing target is not extrapolated.

## Validation Summary

The PC-only test uses target-to-fixed-ROI-center Manhattan error against a known future target center:

| Trajectory | Current-position ROI | Existing one-frame prediction | Latency-aware prediction |
| --- | ---: | ---: | ---: |
| Fast constant-velocity ball, `vx=20`, `vy=5`, 3 delay frames | 100 px | 75 px | 0 px |
| Sudden reversal after request, actual `vx=-20` | 80 px | 100 px | 160 px |

The second row is intentional: constant velocity is not a guarantee under abrupt direction change. The system must replan on later global frames; this stage does not hide that limitation.

Additional tests verify stationary targets, positive and negative velocity, default compatibility, and all four image boundaries. The A-side adapter always emits a fixed `320 x 180` ROI constrained to `x=0..320`, `y=0..180` for the `640 x 360` input contract.

## After Hardware Timing Is Available

Measure each part on the real board, convert it to whole frame units, and configure the model:

1. Measure global-frame processing through C decision completion.
2. Measure AXI-Lite request write/acceptance delay.
3. Measure time from COMMIT to A-side frame-safe latch.
4. Convert each duration using the actual frame period, rounding conservatively upward.
5. Record the measured configuration, board clock, frame rate, and test conditions separately from the default source configuration.

Do not claim these parameters are measured until steps 1-4 are complete on the target hardware.

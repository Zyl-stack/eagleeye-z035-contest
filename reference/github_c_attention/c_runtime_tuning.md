# C Runtime Tuning

`EE_CConfig` groups the competition-time settings while retaining the existing Attention and Scheduler configuration structures. `ee_c_config_default()` reproduces the current behavior.

| Parameter | Default | Purpose | Too small | Too large |
| --- | ---: | --- | --- | --- |
| `scheduler.min_hold_frames` | 4 frames | Minimum focus hold before a valid challenger can switch | Camera focus may chatter | Important action may be followed late |
| `scheduler.switch_margin` | 0.12 | Required relative Attention advantage | Frequent near-tie switching | Strong challenger may not take focus promptly |
| `scheduler.timeout_frames` | 12 frames | Fovea request deadline | Normal latency may time out | Stale requests remain active too long |
| `roi_smoother.max_step_x` | 32 px/frame | Maximum horizontal fixed-ROI movement | Slow horizontal tracking | Abrupt horizontal motion |
| `roi_smoother.max_step_y` | 18 px/frame | Maximum vertical fixed-ROI movement | Slow vertical tracking | Abrupt vertical motion |
| `prediction_horizon_frames` | 1 frame | Documents current constant-velocity horizon | No useful lead | Prediction error grows rapidly |
| `max_prediction_gap_frames` | 1 frame | Maximum observation gap accepted as continuous motion history | Valid skipped frames cannot be predicted | Stale history may drive the ROI |

## Attention weights

The active defaults remain defined by `attention_config.h` and are exposed through `EE_CConfig.attention`:

- Ball Priority: 3
- Motion: 1
- Goal Zone: 3
- Small Target: 2
- Uncertainty: 3
- Ball Proximity/Possession: 2
- Aging: 1

Changing a weight changes competition behavior; no automatic optimization is performed. Always rerun `run_tests_msvc.bat` after tuning.

## Known limitations

The current transaction and prediction code does not implement complete serial-number
arithmetic for 32-bit `frame_id` or `request_id` wraparound. In particular, feedback
deadline/age comparisons, stale-request classification, and predictor continuity across
`UINT32_MAX -> 0` remain known limitations. The medium-risk boundary test reports these
conditions explicitly; its default mode tolerates only these documented limitations,
while `run_medium_risk_tests_msvc.bat --strict` returns failure for them as well.

## ROI smoothing

The smoother converts the desired target center to a fixed 320x180 ROI and limits top-left movement per frame:

```text
new_x = old_x + clamp(desired_x - old_x, -max_step_x, +max_step_x)
new_y = old_y + clamp(desired_y - old_y, -max_step_y, +max_step_y)
```

The result is always clamped to `x=0..320`, `y=0..180`, `w=320`, `h=180`. The first valid request initializes directly to its legal fixed ROI. Scheduler target choice and ROI motion remain separate.

## Diagnostics

`EE_RuntimeLogger` supports `NONE`, `SUMMARY`, and `VERBOSE`; default integration behavior is no extra logging. `bc_stream_demo.exe --summary`, `--verbose`, and `--perf` enable runtime logs and optional standard-C PC timing. PC timing is diagnostic only and is not FPGA cycle timing.

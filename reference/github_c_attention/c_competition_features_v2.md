# C Competition Features V2

This version adds explainable competition-demo features around the existing C core. It does not change the Attention formula, Scheduler selection, Switch Margin, Hold Time, B tracker, A interface, AXI-Lite register map, or FPGA logic.

## Adaptive Enhance Mode

`enhance_policy.c` maps the selected C target to the frozen A-side mode IDs:

- `0 BYPASS`: no valid selected target.
- `1 BILINEAR`: normal tracking, or fast motion where the policy prefers the lower-cost interpolation path.
- `2 LANCZOS`: high Attention, a small target, the estimated ball carrier, or an attack/goal-zone target.

Every decision includes one reason: `NONE`, `NO_TARGET`, `NORMAL`, `HIGH_ATTENTION`, `SMALL_TARGET`, `POSSESSION`, `ATTACK_ZONE`, or `FAST_MOTION`. The deterministic priority is fast motion, possession, attack zone, small target, high Attention, then normal tracking. This is a software policy; it does not claim that Lanczos is always better than Bilinear.

The conservative thresholds live in `EE_CConfig`: high Attention `1500`, small-target component `200`, fast motion `20` pixels/frame (absolute x plus y), default mode `BILINEAR`, high-quality mode `LANCZOS`, and one handover display frame.

## Auto Director State

`director_state.c` observes existing target validity, possession history, and field-zone semantics. It does not select targets and cannot override the Scheduler.

States: `NORMAL_PLAY`, `BALL_CARRIER_FOCUS`, `PASS_HANDOVER`, `ATTACK_ZONE`, and `BALL_LOST`.

Events: `NONE`, `POSSESSION_ACQUIRED`, `PASS_STARTED`, `PASS_COMPLETED`, `BALL_LOST`, `BALL_RECOVERED`, and `ATTACK_ZONE_ENTER`.

Events are one-frame observations. States persist until the corresponding input changes.

## Explainable Decision Snapshot

`decision_snapshot.c` creates one `EE_DecisionSnapshot` per frame. It combines the real Scheduler decision, existing `EE_AttentionBreakdown`, possession result, Director state/event, Enhance decision, smoothed ROI, and request ID. `ee_decision_snapshot_print()` provides a platform-independent text report using only real Attention fields: Ball Priority, Motion, Goal Zone, Small Target, Uncertainty, Ball Proximity, Aging, and Total.

## Foveated Resource Statistics

`fovea_stats.c` uses the current PC source size of `640x360` and fixed ROI size of `320x180`:

```text
full_frame_pixels = 230400
roi_pixels        = 57600
roi_ratio         = 25%
theoretical_pixel_reduction = 75%
```

The 75% number means only that the ROI contains 75% fewer pixels than the full frame. It is a theoretical pixel-workload comparison, not measured bandwidth savings, power savings, latency improvement, or FPGA performance.

Accumulated counters include processed frames, BYPASS/BILINEAR/LANCZOS frames, target switches, ball-lost events, pass handovers, average selected Attention, and average ROI top-left movement.

## Dashboard Fields

The PC visualizer receives all decisions from `visual_demo_bridge.c`. The Decision Panel shows selected target, possession, switch reason, Director state/event, Enhance mode/reason, ROI, and request ID. The Attention panel shows the production breakdown. The Statistics panel shows frame/event/mode counters and the theoretical ROI pixel comparison. Python displays these values but does not reproduce the C decision logic.

## Limits

This remains a PC simulation/demo. The 2x view is Crop plus Bilinear/Lanczos display semantics. It is not a real high-resolution sensor ROI, super resolution, a new camera viewpoint, or hardware-verified image quality. No new Vivado, Vitis, AXI-Lite, DMA/VDMA, camera, or RTL behavior is included.

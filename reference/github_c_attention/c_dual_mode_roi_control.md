# C Dual-Mode ROI Control

## Purpose

`roi_motion_controller` is a small C-side controller between the latency-aware
predicted desired ROI and the existing integer ROI smoother. It does not choose
a target, change Attention, or change Scheduler Hold Time/Switch Margin. Its
only decision is which existing smoother step limit applies to the current
frame.

```text
selected target
  -> latency-aware prediction / joint desired ROI
  -> ROI motion controller (TRACK or FAST_JUMP)
  -> existing ROI smoother
  -> A Fovea adapter
  -> Fovea Request
```

## Modes and defaults

| Mode | Existing smoother step limits | Duration |
|---|---:|---:|
| `TRACK` | `max_step_x=32`, `max_step_y=18` | continuous |
| `FAST_JUMP` | `max_step_x=128`, `max_step_y=72` | 3 frames |

The values are kept in `EE_CConfig`:

- `roi_smoother` remains the backward-compatible TRACK configuration.
- `roi_motion_controller.fast_jump_max_step_x/y` and
  `fast_jump_frames` configure only the temporary fast mode.

No second filter, Kalman filter, learned model, or direct position jump is
introduced. Both modes use `ee_roi_smoother_step()` and retain its fixed
`320x180` ROI and `x=0..320`, `y=0..180` boundary contract.

## FAST_JUMP triggers

The controller enters or renews the three-frame fast window only when a valid
current selected target exists and one of these happens:

1. `PASS_STARTED` from the existing Director.
2. `BALL_RECOVERED` from the existing Director.
3. Scheduler reason is `CURRENT_INVALID` or `CURRENT_LOST` while it has moved
   to a valid selection.
4. A true target-ID change: both previous and current selected IDs are valid
   and unequal.

`PASS_COMPLETED` does not restart the window; it simply consumes a remaining
FAST_JUMP frame. This keeps the pass response bounded.

Trigger inputs are edge/debounce protected. Repeating the same trigger kind
with the same selected/previous target pair does not reload the three-frame
window. Clearing the trigger arms it for a later event. A different trigger
kind or a different target pair is treated as a genuinely new event and may
restart FAST_JUMP, including while a previous fast window is still active.

## Explicit non-triggers

The controller stays in TRACK for the first target, `KEEP_CURRENT`,
`HOLD_ACTIVE`, `MARGIN_NOT_MET`, a pure score change with the same target,
`ATTACK_ZONE_ENTER` without a target change, and `BALL_LOST` with no valid
selection. Thus it does not bypass the Scheduler's anti-thrashing decisions or
blindly jump toward an invalid ball.

## Latency interaction

The latency-aware motion predictor still calculates the desired center first.
The controller only changes the maximum per-frame move used to approach that
desired ROI. Therefore configured latency compensation and FAST_JUMP compose:
the predictor can place the desired ROI ahead of a moving ball, while the
temporary larger smoother limits help the physical ROI catch it quickly.

In the PC Mini Match, a selected player may use the existing Player+Ball joint
desired ROI. This remains an input to the controller; it does not change the
controller's trigger rules or selection logic.

## PC visualizer fields

`visual_demo_bridge.exe` exports the following C-produced JSON fields:

- `roi_motion_mode`: `TRACK` or `FAST_JUMP`
- `fast_jump_remaining`: inclusive count for the current output frame
- `roi_motion_step_x`, `roi_motion_step_y`: limits actually supplied to the
  smoother

`soccer_visualizer.py` renders these fields only. Python does not select a
mode or calculate ROI motion.

## Validation

`tests/test_dual_mode_roi_control.c` verifies:

- TRACK limit and default values.
- Pass comparison: a `320 px` X displacement takes 10 TRACK frames versus 3
  FAST_JUMP frames.
- `BALL_RECOVERED` duration and its automatic return to TRACK.
- HOLD, margin, attack-zone-only, and ball-lost non-trigger behavior.
- True target change, corner bounds, A adapter compatibility, and a
  latency-aware fast-ball prediction case.
- Repeated identical trigger suppression, restart after completion, and an
  explicit restart when a different target switch arrives during FAST_JUMP.

This is PC algorithm validation only. It does not measure PL timing, AXI-Lite
latency, bandwidth, power, or image quality.

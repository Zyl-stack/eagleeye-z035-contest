# Player-driven Attention V3

## Purpose

`PLAYER_DRIVEN_V3` is an A/B mode for football streams where BALL detection is
small, intermittent, or absent. It keeps `SOCCER_V2` intact and leaves V2 as
the default configuration.

In V3, valid PLAYER targets are the only Scheduler candidates:

```text
PLAYER descriptor -> V3 feature extractor -> PLAYER-only candidate view
                  -> existing Scheduler -> Predictor -> ROI -> Fovea transaction
```

BALL records remain in the original `EE_GlobalTargetList`. They may provide an
optional, capped context bonus, but they cannot be selected by the V3 Scheduler.

## Features and Formula

For each valid PLAYER:

```text
Attention = 3 * Motion
          + 3 * Interaction
          + 3 * GoalZone
          + 2 * MotionSaliency
          + 2 * Uncertainty
          + 2 * Aging
          + OptionalBallBonus
```

The six base weights sum to 15. `OptionalBallBonus` is disabled by default and
is capped so it cannot become the primary decision term.

- Motion: `abs(vx) + abs(vy)`, scaled with `player_motion_full_scale`.
- Interaction: nearby PLAYER pairs plus approaching pairs, using Manhattan
  center distance and fixed history keyed by `target_id`.
- GoalZone: disabled by default until calibrated field-zone data is available.
- MotionSaliency: change in the same PLAYER's velocity versus the previous
  continuous observation.
- Uncertainty: `(100 - confidence) * 255 / 100` after confidence clamping.
- Aging: existing Scheduler waiting semantics, computed before the next step.

Small Target is intentionally absent from V3 ranking. It remains available to
Enhance Policy for choosing BILINEAR or LANCZOS.

## Tunable Baseline

`EE_PlayerAttentionConfig` keeps V3-only weights and thresholds separate from
the V2 `EE_AttentionConfig`. Motion, interaction and saliency thresholds are
initial tunable baselines, not ground-truth-optimal values.

The default mode is:

```c
config.attention_mode = EE_ATTENTION_MODE_SOCCER_V2;
```

Enable V3 explicitly for an experiment:

```c
config.attention_mode = EE_ATTENTION_MODE_PLAYER_V3;
```

## PC Real-stream A/B Harness

Run this from a Visual Studio Developer Command Prompt:

```text
run_player_attention_v3_ab_msvc.bat "C:\path\to\real_match_b_target_stream.txt"
```

The harness runs four in-memory experiments without changing the source file:

1. V2 with the original stream.
2. V2 with BALL records masked.
3. V3 with the original stream.
4. V3 with BALL records masked.

It prints selection availability, PLAYER/BALL selection counts, switching,
FAST_JUMP, ROI movement, transaction validity, prediction validity, BALL-loss
windows, V3 feature distributions, and a selected-target sequence hash.

## Director Compatibility

The existing Director remains a BALL-dependent compatibility layer. Therefore
`BALL_LOST`, `BALL_RECOVERED`, and pass events can change when BALL records are
masked. In V3 these events do not mean there is no focus target: PLAYER
selection, ROI generation and Fovea transactions continue independently.

## Scope Boundaries

V3 does not change the B stream format, frozen descriptors, Scheduler core,
Switch Margin, Hold, Predictor, ROI smoother, Fovea transaction logic,
AXI-Lite, Vitis, or FPGA code.

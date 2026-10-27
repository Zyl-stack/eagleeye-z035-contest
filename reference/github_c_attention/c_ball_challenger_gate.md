# BALL Challenger Eligibility Gate

## Purpose

Some short-lived, low-confidence BALL detections can receive a high Attention
score because BALL and small-target features are intentionally important. This
module is an independent Scheduler eligibility filter for those new BALL
challengers. It does not modify the frozen B/C descriptor, confidence, or
Attention score.

## Position

```text
B stream -> bc_adapter -> Possession -> Attention -> BALL gate -> Scheduler
```

The gate creates a Scheduler-only copy of `EE_GlobalTargetList`. The original
list remains available to Possession, logging, Director, Predictor, ROI, and
Fovea transaction code.

## Default Policy

`EE_CConfig` defaults are:

```text
ball_challenger_gate_enable = 1
ball_challenger_min_confidence = 25
```

A valid new BALL with confidence below 25 is excluded from that Scheduler
step. A candidate at 25 or above remains eligible. The value is configurable;
25 was selected from one real football-stream replay and is not claimed to be
optimal for every camera, detector, or match.

## Boundaries

- A currently Scheduler-locked, valid BALL is exempt, even below 25.
- PLAYER targets are never filtered by this module.
- The Scheduler still owns argmax, Hold Time, and Switch Margin decisions.
- Attention weights and scores are unchanged.
- The module does not alter B/C descriptor fields or any A-side interface.

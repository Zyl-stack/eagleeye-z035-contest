# C Feedback-Aware Attention V1

## Purpose

This stage adds one explicit and bounded input to the existing C Attention
calculation:

```text
final_attention = base_attention + feedback_adjustment
```

`base_attention` is the pre-existing score and is unchanged. It remains the
weighted sum of Ball Priority, Motion, Goal Zone, Small Target, Uncertainty,
Ball Proximity, and Aging. `feedback_adjustment` is never written into coarse
confidence, uncertainty, or another old feature, so its effect is visible in
the decision output.

## Feedback policy

`EE_FeedbackAttentionPolicy` is stored in `EE_CConfig` and contains:

- `enable`: disabled by default to retain the stable competition behavior.
- `feedback_bonus`: small positive adjustment for an accepted feedback record.
- `timeout_penalty`: small negative adjustment for a timed-out request.
- `max_abs_adjustment`: hard bound on either adjustment magnitude.
- `max_age_frames`: maximum usable age from the request source frame.

The default policy is disabled. The C visual bridge explicitly uses the
separate `ee_feedback_attention_pc_demo_policy()` experimental profile. This
does not change the default PC regression, Vitis, or board paths.

## When feedback can affect Attention

Only an `ACCEPTED` record can add the configured bounded bonus, and it must
match its stored request in all three fields:

```text
request_id + source_frame_id + target_id
```

It affects only the target whose `target_id` matches. It is also ignored once
`current_frame_id - source_frame_id > max_age_frames`.

`TIMEOUT` may apply the configured bounded negative adjustment to its own
pending target while it is still within the same age limit. `NONE` and
`PENDING` give zero adjustment.

`STALE_FEEDBACK`, request/frame/target mismatch, `DUPLICATE`, and `INVALID`
always yield adjustment zero. They remain diagnostic transaction outcomes;
they cannot influence target selection.

## Causality

The PC bridge freezes a copy of the feedback transaction state immediately
before the current frame is scored. A request submitted or feedback accepted
later in that frame is not visible until the next frame. The flow is therefore:

```text
previous accepted feedback -> current Attention -> Scheduler -> ROI pipeline
-> current FoveaRequest -> simulated feedback -> next-frame Attention
```

The existing Scheduler source, Hold Time, and Switch Margin are unchanged.

## Decision and visualization fields

`EE_AttentionBreakdown`, `EE_DecisionSnapshot`, the C JSON bridge, and the
Python visualizer show:

- `base_attention`
- `feedback_adjustment`
- `final_attention`
- `feedback_attention_reason`
- feedback transaction age

Python only renders these C-produced fields; it does not calculate feedback
or choose a target.

## Verification scope

PC tests verify default-disabled B 12-frame sample stability, eight existing
match regression scenes, bounded accepted feedback, rejected feedback,
expiration, strong-target protection, close-score behavior, and unchanged
Scheduler hold behavior.

This is **PC-simulated, transaction-level feedback**. Current hardware does
not expose refined confidence, refined bbox, or refined class, and the
existing AXI-Lite `QUALITY_FLAGS` bits do not define a high-quality detection
meaning. They are stored and displayed but are not interpreted by this policy.

**PC FEEDBACK-AWARE DECISION VERIFIED**

**NOT HIGH-RES HARDWARE FEEDBACK VERIFIED**

## Future insertion point

When A exposes a transaction-tagged refined result with frozen semantics, a
future policy may define an additional bounded signal from those fields. It
must retain strict transaction matching, target scoping, age limits, and an
explicit configuration default.

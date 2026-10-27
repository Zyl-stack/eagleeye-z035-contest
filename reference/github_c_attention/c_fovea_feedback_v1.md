# C Fovea Feedback V1

## Scope

Feedback V1 verifies the request transaction loop only:

```text
FoveaRequest created -> submitted/pending -> feedback candidate arrives
-> request/frame/target validation -> accepted or rejected -> C feedback store
```

It does not modify Attention, Scheduler selection, Hold Time, Switch Margin,
ROI motion, Enhance Policy, A RTL, or the AXI-Lite register map.

## Why the three IDs are required

`request_id`, `source_frame_id`, and `target_id` together identify a Fovea
transaction. Target ID alone is not enough: the same ball/player can appear in
multiple requests, and a delayed result could otherwise overwrite a newer
transaction.

An accepted record must match all three values exactly. A feedback candidate
with an older request ID is `STALE_FEEDBACK`; a newer/different request ID is
`REQUEST_ID_MISMATCH`; matching request ID but different frame or target is
`FRAME_ID_MISMATCH` or `TARGET_ID_MISMATCH`.

## Data structures

`EE_FoveaFeedback` contains only:

- `request_id`, `source_frame_id`, `target_id`
- `valid`
- C-side `result_status`
- `quality_flags`

`EE_FoveaFeedbackState` keeps the current pending request, most recent
accepted feedback, most recent received feedback/status, age in frames, and
pending/timeout state. All storage is fixed-size C data; no dynamic memory is
used.

The result status is one of `PENDING`, `ACCEPTED`, `TIMEOUT`,
`STALE_FEEDBACK`, `REQUEST_ID_MISMATCH`, `FRAME_ID_MISMATCH`,
`TARGET_ID_MISMATCH`, `INVALID`, or `DUPLICATE`.

## Lifecycle behavior

1. `ee_fovea_feedback_begin()` stores an actual valid `EE_FoveaRequest` and
   enters `PENDING`.
2. `ee_fovea_feedback_advance()` records age and turns a pending request into
   `TIMEOUT` only after its request deadline.
3. `ee_fovea_feedback_receive()` strictly validates the three IDs.
4. Exact feedback is copied to `latest_accepted` and enters `ACCEPTED`.
5. A duplicate does not replace `latest_accepted`. An old/out-of-order result
   cannot overwrite a newer pending request. A result received after timeout
   remains rejected as `TIMEOUT`.

Feedback is exported through `EE_DecisionSnapshot` and the C JSON bridge for
display. It remains observability only in V1.

## PC simulator

`fovea_feedback_sim.c` is a PC test input generator, not an image algorithm.
It produces exact, stale, request/frame/target mismatch, invalid, and quality
flag candidate records. The unit test also exercises timeout, late feedback,
duplicate, out-of-order delivery, and the real C request pipeline:

```text
Scheduler -> Motion Predictor -> TRACK/FAST_JUMP -> ROI smoother
-> A adapter -> EE_FoveaRequest -> feedback validation
```

## Hardware boundary

Current Vitis code can read these real AXI-Lite values:

- `STATUS` (`busy`, `done`, `error`, `active_mode`)
- `CYCLE_COUNT`
- `QUALITY_FLAGS`

`STATUS.done` means command acceptance at a frame boundary, not image
processing completion. Current register definitions do not provide a readable
feedback `request_id`, `source_frame_id`, or `target_id`; therefore the strict
three-ID loop is verified with PC simulation today. The hardware must expose
or transport those tags before an equivalent tagged hardware feedback claim is
possible.

No real refined bbox, refined class, refined confidence, high-resolution
detector result, or super-resolution result is produced by Feedback V1.

## Status

**PC TRANSACTION LOOP VERIFIED**

**NOT HIGH-RES HARDWARE FEEDBACK VERIFIED**

## Future direction

A later Feedback-Aware Attention stage may consume only accepted, transaction-
matched feedback. That future change must explicitly define how feedback
affects Attention and Scheduler behavior; Feedback V1 intentionally does not.

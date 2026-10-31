#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
c="$root/reference/github_c_attention"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cc -std=c99 -O2 -Wall -Wextra -I"$c/include" "$root/tools/bc_stream_handoff.c" \
  "$c/src/bc_adapter.c" "$c/src/ball_challenger_gate.c" "$c/src/c_config.c" \
  "$c/src/c_performance.c" "$c/src/c_runtime_log.c" "$c/src/decision_snapshot.c" \
  "$c/src/director_state.c" "$c/src/enhance_policy.c" "$c/src/fovea_feedback.c" \
  "$c/src/feedback_attention.c" "$c/src/fovea_stats.c" "$c/src/fovea_transaction.c" \
  "$c/src/attention.c" "$c/src/attention_scheduler.c" "$c/src/motion_predictor.c" \
  "$c/src/player_attention_features.c" "$c/src/possession_estimator.c" \
  "$c/src/roi_motion_controller.c" "$c/src/roi_smoother.c" "$c/src/eagleeye_types.c" \
  -o "$tmp/bc_stream_handoff"
"$tmp/bc_stream_handoff" "$c/examples/b_target_stream_v1_sample.txt" > "$tmp/sample.txt"
test "$(grep -c '^FRAME ' "$tmp/sample.txt")" -eq 12
printf 'INVALID\n' > "$tmp/invalid.txt"
if "$tmp/bc_stream_handoff" "$tmp/invalid.txt" > /dev/null 2>&1; then echo 'FAIL invalid stream accepted' >&2; exit 1; fi
printf 'PASS: synthetic attention sample 12 frames; invalid input rejected.\n'
printf 'Scope: PC stream/attention demo only; no model inference or board test.\n'

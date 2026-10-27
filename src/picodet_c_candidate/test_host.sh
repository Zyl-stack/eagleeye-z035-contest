#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
objs=()
for name in bc_adapter ball_challenger_gate c_config c_performance c_runtime_log decision_snapshot director_state enhance_policy fovea_feedback feedback_attention fovea_stats fovea_transaction attention attention_scheduler motion_predictor player_attention_features possession_estimator roi_motion_controller roi_smoother eagleeye_types; do
  cc -std=c99 -O2 -I"$root/c_include" -c "$root/c_src/$name.c" -o "$tmp/$name.o"
  objs+=("$tmp/$name.o")
done
c++ -std=c++14 -O2 -Wall -Wextra -Werror -I"$root/c_include" -I"$root/app/src" "$root/test_c_attention.cpp" "$root/app/src/picodet_c_attention.cpp" "${objs[@]}" -o "$tmp/test"
"$tmp/test"
echo 'PASS: real C attention/scheduler, provisional ID, loss/recovery, invalid/overflow rejection; no hardware'
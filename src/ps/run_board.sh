#!/bin/sh
set -eu
PACKAGE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
RESULTS_ROOT=${RESULTS_ROOT:-/tmp/eagleeye_v7_results}
export ITERATIONS=${ITERATIONS:-100}
export THREADS=${THREADS:-2}
export THRESHOLD=${THRESHOLD:-0.3}
export PIXEL_ORDER=${PIXEL_ORDER:-rgb}
export READY_TIMEOUT_MS=${READY_TIMEOUT_MS:-45000}
export VIDEO_POLICY=${VIDEO_POLICY:-auto}
export FOVEA_ROI=${FOVEA_ROI:-1}
export MOUSE_ROI=${MOUSE_ROI:-1}
export MOUSE_DEVICE=${MOUSE_DEVICE:-auto}
export MOUSE_SENSITIVITY=${MOUSE_SENSITIVITY:-2}
export PACKAGE
export SESSION_LOCK=/tmp/eagleeye-camera-v4-session
exec sh "$PACKAGE/scripts/run_logged.sh" "$PACKAGE/scripts/board_worker.sh" "$RESULTS_ROOT"

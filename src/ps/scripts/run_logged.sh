#!/bin/sh
set -eu
WORKER=$1
RESULTS=$2
LOCK=${SESSION_LOCK:-/tmp/eagleeye-camera-v4-session}
CHILD=
LOGGER=
RUN_DIR=
if ! mkdir "$LOCK" 2>/dev/null; then
    echo "ERROR session already locked at $LOCK; check its pid file; cold reboot removes stale /tmp lock" >&2
    exit 3
fi
cleanup() {
    trap - EXIT INT TERM HUP
    if [ -n "$CHILD" ]; then
        kill -TERM "$CHILD" 2>/dev/null || true
        wait "$CHILD" 2>/dev/null || true
    fi
    if [ -n "$LOGGER" ]; then wait "$LOGGER" 2>/dev/null || true; fi
    if [ -n "$RUN_DIR" ]; then rm -f "$RUN_DIR/output.pipe"; fi
    rm -f "$LOCK/pid"
    rmdir "$LOCK" 2>/dev/null || true
}
signal_exit() {
    if [ -n "$RUN_DIR" ]; then echo "$1" > "$RUN_DIR/exit_code.txt"; fi
    exit "$1"
}
trap cleanup EXIT
trap 'signal_exit 130' INT
trap 'signal_exit 143' TERM
trap 'signal_exit 129' HUP
echo "$$" > "$LOCK/pid"
mkdir -p "$RESULTS"
RUN_DIR=$(mktemp -d "$RESULTS/run_$(date +%Y%m%d_%H%M%S)_XXXXXX")
mkfifo "$RUN_DIR/output.pipe"
echo "Results: $RUN_DIR"
tee "$RUN_DIR/board.log" < "$RUN_DIR/output.pipe" &
LOGGER=$!
sh "$WORKER" "$RUN_DIR" > "$RUN_DIR/output.pipe" 2>&1 &
CHILD=$!
if wait "$CHILD"; then RC=0; else RC=$?; fi
CHILD=
if wait "$LOGGER"; then :; else RC=4; fi
LOGGER=
echo "$RC" > "$RUN_DIR/exit_code.txt"
awk '/^TIME / {n++} /^PERSON confidence=/ {b++} /^PERSON none_above_threshold=/ {z++}
     END {printf "inferences=%d boxes=%d empty_inferences=%d\n",n,b,z}' "$RUN_DIR/board.log" > "$RUN_DIR/summary.txt"
cat "$RUN_DIR/summary.txt"
echo "Exit=$RC Results=$RUN_DIR"
exit "$RC"

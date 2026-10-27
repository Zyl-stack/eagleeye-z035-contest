#!/bin/sh
set -eu
RUN_DIR=$1
CHILD=
stop_child() {
    trap - INT TERM HUP
    if [ -n "$CHILD" ]; then
        kill -TERM "$CHILD" 2>/dev/null || true
        wait "$CHILD" 2>/dev/null || true
    fi
    exit 130
}
trap stop_child INT TERM HUP
run_child() {
    "$@" &
    CHILD=$!
    if wait "$CHILD"; then rc=0; else rc=$?; fi
    CHILD=
    return "$rc"
}
cd "$PACKAGE"
[ "$(id -u)" = 0 ] || { echo "ERROR use sudo -i on the BOARD"; exit 2; }
for value in "$ITERATIONS" "$READY_TIMEOUT_MS"; do
    case "$value" in ''|*[!0-9]*) echo 'ERROR invalid iteration/timeout value'; exit 2;; esac
done
[ "$ITERATIONS" -le 1000000000 ] || exit 2
[ "$READY_TIMEOUT_MS" -ge 100 ] && [ "$READY_TIMEOUT_MS" -le 120000 ] || exit 2
case "$THREADS" in 1|2) :;; *) echo 'ERROR threads must be 1 or 2'; exit 2;; esac
case "$PIXEL_ORDER" in rgb|bgr) :;; *) echo 'ERROR pixel order'; exit 2;; esac
case "$VIDEO_POLICY" in auto|attach) :;; *) echo 'ERROR video policy'; exit 2;; esac
case "$FOVEA_ROI" in 0|1) :;; *) echo 'ERROR FOVEA_ROI must be 0 or 1'; exit 2;; esac
case "$MOUSE_ROI" in 0|1) :;; *) echo 'ERROR MOUSE_ROI must be 0 or 1'; exit 2;; esac
case "$MOUSE_SENSITIVITY" in 1|2|3|4|5|6|7|8) :;; *) echo 'ERROR MOUSE_SENSITIVITY must be 1..8'; exit 2;; esac
awk -v x="$THRESHOLD" 'BEGIN {exit !(x ~ /^[0-9]+([.][0-9]+)?$/ && x>=0 && x<=1)}' || {
    echo 'ERROR threshold must be in [0,1]'; exit 2;
}
sha256sum -c SHA256SUMS
chmod +x bin/video_bootstrap_verified bin/video_guard bin/paddle_person_live bin/fovea_ctl
[ -c /dev/mem ] && [ -c /dev/i2c-0 ] || { echo 'ERROR /dev/mem or /dev/i2c-0 missing'; exit 2; }
for name in paddle_person paddle_person_attach_video paddle_person_stable_snapshot \
            paddle_person_live vdma_probe camera_display_probe_static video_bootstrap_verified; do
    if pidof "$name" >/dev/null 2>&1; then
        echo "ERROR $name is already running; stop it before this test"; exit 3
    fi
done
for address in 43000000 43010000 41210000 43c00000 43c10000; do
    for driver in /sys/bus/platform/devices/"$address".*/driver; do
        if [ -L "$driver" ]; then echo "ERROR kernel already owns $driver"; exit 3; fi
    done
done
date '+%Y-%m-%dT%H:%M:%S%z'
uname -a
cat /proc/iomem > "$RUN_DIR/iomem.txt"
dmesg > "$RUN_DIR/dmesg_before.txt"
if run_child bin/video_guard inspect; then STATE=0; else STATE=$?; fi
if [ "$STATE" = 10 ] && [ "$VIDEO_POLICY" = auto ]; then
    echo 'VIDEO cold start: invoking the byte-identical, board-tested initializer ONCE'
    run_child bin/video_bootstrap_verified 100
elif [ "$STATE" != 0 ]; then
    echo "ERROR video inspect=$STATE; leave state unchanged, save log and cold power-cycle if needed"
    exit "$STATE"
else
    echo 'VIDEO existing compatible DMA; attaching without reset or DDR clear'
fi
echo 'VIDEO readiness: move the camera scene; content-change heuristic, not a frame-completion guarantee'
run_child bin/video_guard wait --timeout-ms "$READY_TIMEOUT_MS"
set --
if [ "$FOVEA_ROI" = 1 ]; then set -- "$@" --fovea-roi; fi
if [ "$MOUSE_ROI" = 1 ]; then
    set -- "$@" --mouse-roi --mouse-device "$MOUSE_DEVICE" --mouse-sensitivity "$MOUSE_SENSITIVITY"
fi
if run_child bin/paddle_person_live --attach-existing-video "$@" --model ./model.nb \
    --labels ./label_list.txt --iterations "$ITERATIONS" --threads "$THREADS" \
    --threshold "$THRESHOLD" --warmup 0 --pixel-order "$PIXEL_ORDER" \
    --dump-first "$RUN_DIR/first_frame.ppm" --json "$RUN_DIR/result.json"; then RC=0; else RC=$?; fi
bin/video_guard inspect > "$RUN_DIR/status_after.txt" 2>&1 || true
dmesg > "$RUN_DIR/dmesg_after.txt"
echo 'VIDEO left running after inference; no register status bits were cleared by guard/inference'
exit "$RC"

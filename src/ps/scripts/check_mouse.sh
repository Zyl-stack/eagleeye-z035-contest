#!/bin/sh
set -eu
echo '=== USB/input devices ==='
cat /proc/bus/input/devices
echo '=== event nodes ==='
ls -l /dev/input/event* 2>/dev/null || true
echo '=== recent kernel messages ==='
dmesg | tail -60

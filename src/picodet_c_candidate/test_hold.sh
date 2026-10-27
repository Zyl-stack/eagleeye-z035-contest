#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
c++ -std=c++14 -O2 -Wall -Wextra -Werror -pthread -I"$root/app/src" "$root/test_hold.cpp" "$root/app/src/roi_animator.cpp" -o "$tmp/test_hold"
"$tmp/test_hold"
echo 'PASS mock Fovea: Hold waits for in-flight commit and prevents later stale movement'
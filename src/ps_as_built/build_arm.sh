#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PROJECT=${PROJECT:-"$HOME/eagleeye_ax7020_build/eagleeye_petalinux_2022_2"}
: "${PADDLE_LITE_DIR:?Set PADDLE_LITE_DIR to the delivered ARMv7 Paddle-Lite include/lib directory}"
COMPONENTS="$PROJECT/build/tmp/sysroots-components"
CXX="$COMPONENTS/x86_64/gcc-cross-arm/usr/bin/arm-xilinx-linux-gnueabi/arm-xilinx-linux-gnueabi-g++"
BINUTILS="$COMPONENTS/x86_64/binutils-cross-arm/usr/bin/arm-xilinx-linux-gnueabi"
SYSROOT="$PROJECT/build/tmp/work/zynq_generic-xilinx-linux-gnueabi/fsbl/1.0-r0/recipe-sysroot"
for file in "$CXX" "$BINUTILS/arm-xilinx-linux-gnueabi-as" "$SYSROOT/usr/include/stdlib.h" \
            "$PADDLE_LITE_DIR/lib/libpaddle_api_light_bundled.a"; do
    [ -e "$file" ] || { echo "Missing build dependency: $file"; exit 1; }
done
mkdir -p "$ROOT/bin" "$ROOT/logs"
TOOLS=$(mktemp -d)
trap 'rm -f "$TOOLS/as" "$TOOLS/ld"; rmdir "$TOOLS"' EXIT
ln -s "$BINUTILS/arm-xilinx-linux-gnueabi-as" "$TOOLS/as"
ln -s "$BINUTILS/arm-xilinx-linux-gnueabi-ld" "$TOOLS/ld"
FLAGS=(-B"$TOOLS/" --sysroot="$SYSROOT" -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard \
       -std=c++14 -O3 -pthread -static -Wall -Wextra -Werror -Wl,-z,noexecstack)
"$CXX" "${FLAGS[@]}" "$ROOT/src/video_guard.cpp" -o "$ROOT/bin/video_guard"
"$CXX" "${FLAGS[@]}" "$ROOT/src/fovea_ctl.cpp" "$ROOT/src/fovea_control.cpp" -o "$ROOT/bin/fovea_ctl"
"$CXX" "${FLAGS[@]}" -isystem "$PADDLE_LITE_DIR/include" \
    "$ROOT/src/paddle_person.cpp" "$ROOT/src/eagleeye_hw.cpp" "$ROOT/src/fovea_control.cpp" \
    "$ROOT/src/roi_animator.cpp" "$ROOT/src/mouse_roi_controller.cpp" \
    "$PADDLE_LITE_DIR/lib/libpaddle_api_light_bundled.a" \
    -lgomp -lpthread -ldl -lm -o "$ROOT/bin/paddle_person_live"
file "$ROOT/bin/video_guard" "$ROOT/bin/fovea_ctl" "$ROOT/bin/paddle_person_live"
"$BINUTILS/arm-xilinx-linux-gnueabi-readelf" -h "$ROOT/bin/paddle_person_live"

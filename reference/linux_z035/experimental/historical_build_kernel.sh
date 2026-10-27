#!/bin/bash
set -euo pipefail
OUT=/mnt/d/EagleEye_XC7Z035/04_Linux_Driver/Isolated_Kernel_20261022
TOOLS=/home/eagleuser/eagleeye_ax7020_build/eagleeye_petalinux_2022_2/build/tmp/sysroots-components/x86_64
G=$TOOLS/gcc-cross-arm/usr/bin/arm-xilinx-linux-gnueabi/arm-xilinx-linux-gnueabi-gcc
B=$TOOLS/binutils-cross-arm/usr/bin/arm-xilinx-linux-gnueabi/arm-xilinx-linux-gnueabi-
export TMPDIR=$OUT/tmp
ARGS=( -C "$OUT/source" O="$OUT/build" ARCH=arm "CROSS_COMPILE=$B" "CC=$G -B$OUT/tool-bin/" )
make "${ARGS[@]}" olddefconfig
diff -u "$OUT/baseline.config" "$OUT/build/.config" > "$OUT/logs/CONFIG.diff" || test $? = 1
make "${ARGS[@]}" -j12 zImage modules
make "${ARGS[@]}" INSTALL_MOD_PATH="$OUT/module_root" modules_install
cp "$OUT/build/arch/arm/boot/zImage" "$OUT/artifacts/zImage"
cp "$OUT/build/vmlinux" "$OUT/artifacts/vmlinux"
cp "$OUT/build/.config" "$OUT/artifacts/kernel.config"
cp "$OUT/build/Module.symvers" "$OUT/artifacts/Module.symvers"
cat "$OUT/build/include/config/kernel.release" > "$OUT/artifacts/kernel.release"
sha256sum "$OUT/artifacts/"* > "$OUT/logs/KERNEL_SHA256.txt"
printf 'KERNEL_BUILD_PASS\n'

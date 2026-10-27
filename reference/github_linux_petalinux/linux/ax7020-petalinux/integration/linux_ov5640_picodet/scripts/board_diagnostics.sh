#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR"
mkdir -p logs
STAMP=$(date +%Y%m%d_%H%M%S)
OUT=${1:-"./logs/diagnostics_${STAMP}.log"}
APP_LOG=${2:-}

read_vdma() {
  label=$1
  address=$2
  if command -v devmem >/dev/null 2>&1; then
    printf '%s (%s) = ' "$label" "$address"
    devmem "$address" 32 2>&1 || true
  else
    echo "$label ($address): devmem unavailable"
  fi
}

{
  echo "=== date ==="
  date '+%Y-%m-%dT%H:%M:%S%z'
  echo "=== uname ==="
  uname -a
  echo "=== cmdline ==="
  cat /proc/cmdline
  echo "=== reserved frame memory ==="
  grep -iE '1e000000-1e20fff[fF]|reserved' /proc/iomem || true
  echo "=== devices ==="
  ls -l /dev/mem /dev/i2c-* 2>&1 || true
  echo "=== relevant dmesg ==="
  dmesg | grep -iE 'xilinx|vdma|vtc|i2c|drm|hdmi|reserved' || true
  echo "=== read-only VDMA registers (no status clear) ==="
  read_vdma 'S2MM CR' 0x43010030
  read_vdma 'S2MM DMASR' 0x43010034
  read_vdma 'S2MM PARK_PTR' 0x43010028
  read_vdma 'S2MM VERSION' 0x4301002c
  read_vdma 'S2MM VSIZE' 0x430100a0
  read_vdma 'S2MM HSIZE' 0x430100a4
  read_vdma 'S2MM STRIDE' 0x430100a8
  read_vdma 'S2MM START0' 0x430100ac
  read_vdma 'S2MM START1' 0x430100b0
  read_vdma 'S2MM START2' 0x430100b4
  read_vdma 'S2MM OBS_HSIZE (valid after EOLEarlyErr)' 0x430100f0
  read_vdma 'S2MM OBS_VSIZE (valid after SOFEarlyErr)' 0x430100f4
  read_vdma 'MM2S CR' 0x43000000
  read_vdma 'MM2S DMASR' 0x43000004
  read_vdma 'MM2S PARK_PTR' 0x43000028
  read_vdma 'MM2S VSIZE' 0x43000050
  read_vdma 'MM2S HSIZE' 0x43000054
  read_vdma 'MM2S STRIDE' 0x43000058
  if [ -n "$APP_LOG" ] && [ -r "$APP_LOG" ]; then
    echo "=== application frame sequence / errors / timeouts ==="
    grep -E 'FRAME completed_sequence|SNAPSHOT frame_sequence|VDMA ERROR|timeout' "$APP_LOG" || true
  fi
} | tee "$OUT"

echo "Diagnostics saved to $OUT"

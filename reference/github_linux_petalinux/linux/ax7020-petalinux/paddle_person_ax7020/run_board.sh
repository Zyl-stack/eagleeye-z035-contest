#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR"

if [ "$(id -u)" -ne 0 ]; then
  echo "ERROR: run as root; /dev/mem and OV5640 I2C access are required" >&2
  exit 1
fi
if [ "$(uname -m)" != "armv7l" ]; then
  echo "WARNING: expected Zynq ARMv7 Linux, current machine is $(uname -m)" >&2
fi
if [ ! -c /dev/mem ]; then
  echo "ERROR: /dev/mem is missing" >&2
  exit 1
fi
if [ ! -c "${I2C_DEVICE:-/dev/i2c-0}" ]; then
  echo "ERROR: ${I2C_DEVICE:-/dev/i2c-0} is missing; enable i2c-dev" >&2
  exit 1
fi
if ! grep -qiE '^1e000000-1e20fff[fF].*reserved' /proc/iomem && \
   ! dmesg | grep -qiE 'created DMA memory pool at 0x1e000000'; then
  echo "ERROR: 0x1e000000..0x1e20ffff was not found in /proc/iomem and no DMA pool creation message was found" >&2
  exit 1
fi

chmod +x ./paddle_person
mkdir -p logs
STAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE=${LOG_FILE:-"./logs/board_${STAMP}.log"}
THREADS=${THREADS:-2}
THRESHOLD=${THRESHOLD:-0.5}
WARMUP=${WARMUP:-1}
ITERATIONS=${ITERATIONS:-0}
PIXEL_ORDER=${PIXEL_ORDER:-rgb}
FRAME_DUMP=${FRAME_DUMP:-./first_frame.ppm}

echo "EagleEye board run: $(date '+%Y-%m-%dT%H:%M:%S%z')" | tee "$LOG_FILE"
echo "kernel=$(uname -a)" | tee -a "$LOG_FILE"
echo "pixel_order=$PIXEL_ORDER threads=$THREADS threshold=$THRESHOLD" | tee -a "$LOG_FILE"

./paddle_person \
  --model ./model.nb \
  --labels ./label_list.txt \
  --threads "$THREADS" \
  --threshold "$THRESHOLD" \
  --warmup "$WARMUP" \
  --iterations "$ITERATIONS" \
  --pixel-order "$PIXEL_ORDER" \
  --i2c "${I2C_DEVICE:-/dev/i2c-0}" \
  --dump-first "$FRAME_DUMP" \
  --json ./result.json 2>&1 | tee -a "$LOG_FILE"

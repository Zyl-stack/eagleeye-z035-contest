#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd -- "$SCRIPT_DIR/../../.." && pwd)
APP="$ROOT/paddle_person_ax7020"
OUTPUT=${OUTPUT:-"$ROOT/deliverables"}
PACKAGE="$OUTPUT/sdcard/eagleeye_picodet"

if [[ ! -x "$APP/paddle_person" || "$APP/src/paddle_person.cpp" -nt "$APP/paddle_person" || \
      "$ROOT/integration/linux_ov5640_picodet/src/eagleeye_hw.cpp" -nt "$APP/paddle_person" ]]; then
  echo "ERROR: paddle_person is missing or stale; run build_app.sh first" >&2
  exit 1
fi
if [[ ! -f "$OUTPUT/images/BOOT.BIN" || ! -f "$OUTPUT/images/image.ub" ]]; then
  echo "ERROR: BOOT.BIN/image.ub are missing; run build_petalinux.sh first" >&2
  exit 1
fi

rm -rf -- "$OUTPUT/sdcard"
mkdir -p "$PACKAGE/logs"
cp -f "$OUTPUT/images/BOOT.BIN" "$OUTPUT/sdcard/BOOT.BIN"
cp -f "$OUTPUT/images/image.ub" "$OUTPUT/sdcard/image.ub"
cp -f "$APP/paddle_person" "$APP/model.nb" "$APP/label_list.txt" \
  "$APP/infer_cfg.yml" "$APP/run_board.sh" "$PACKAGE/"
cp -f "$ROOT/integration/linux_ov5640_picodet/scripts/board_diagnostics.sh" \
  "$PACKAGE/"
chmod +x "$PACKAGE/paddle_person" "$PACKAGE/run_board.sh" \
  "$PACKAGE/board_diagnostics.sh"
(cd "$OUTPUT/sdcard" && sha256sum BOOT.BIN image.ub \
  eagleeye_picodet/paddle_person eagleeye_picodet/model.nb > SHA256SUMS)
(cd "$OUTPUT" && tar -czf eagleeye_ax7020_sd_package.tar.gz sdcard)
echo "Copy the contents of $OUTPUT/sdcard to the SD boot partition/root data area as documented."

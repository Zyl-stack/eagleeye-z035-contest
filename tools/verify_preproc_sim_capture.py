#!/usr/bin/env python3
"""Compare retained Icarus RGB plane output to the reference FP32 bits."""
from pathlib import Path
import struct


def main():
    root = Path(__file__).resolve().parents[1]
    outputs = root / 'sim/preproc_captured_dma_20261027'
    golden = (root / 'data/captured.golden.f32').read_bytes()
    plane_size = 320 * 320
    if len(golden) != 3 * plane_size * 4:
        raise ValueError('unexpected reference size')
    offset = 0
    mismatches = 0
    for channel in 'rgb':
        words = (outputs / f'captured_dma_{channel}.hex').read_text(encoding='ascii').split()
        if len(words) != plane_size:
            raise ValueError(f'{channel}: unexpected value count {len(words)}')
        for word in words:
            if struct.pack('<I', int(word, 16)) != golden[offset:offset + 4]:
                mismatches += 1
            offset += 4
    print(f'Icarus captured DMA: compared={offset // 4} mismatches={mismatches}; NOT BOARD DATA')
    if mismatches:
        raise SystemExit(1)


if __name__ == '__main__':
    main()

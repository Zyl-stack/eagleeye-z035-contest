"""Summarize this 35-frame preprocessor A/B run; do not infer frame equality."""
import argparse
import json
import re
import statistics
from pathlib import Path

FIELDS = ('capture_ms', 'preprocess_ms', 'inference_ms', 'postprocess_ms', 'total_ms')
PAIR = re.compile(r'\b(capture_ms|preprocess_ms|inference_ms|postprocess_ms|total_ms)=([0-9]+(?:\.[0-9]+)?)\b')


def load(path, backend):
    lines = Path(path).read_text(encoding='utf-8', errors='replace').splitlines()
    if any('PREPROC_FALLBACK' in line for line in lines):
        raise ValueError(f'{backend}: fallback detected')
    rows, pending = [], None
    for line in lines:
        if line.startswith('PREPROC backend='):
            if pending is not None:
                raise ValueError(f'{backend}: repeated marker before TIME')
            pending = line.split()[1].split('=', 1)[1]
        elif line.startswith('TIME '):
            pairs = PAIR.findall(line)
            row = {k: float(v) for k, v in pairs}
            if pending != backend or len(pairs) != len(FIELDS) or set(row) != set(FIELDS) or row['total_ms'] <= 0:
                raise ValueError(f'{backend}: invalid marker or TIME record')
            rows.append(row)
            pending = None
    if pending is not None or len(rows) != 35:
        raise ValueError(f'{backend}: expected 35 paired markers and TIME records')
    return rows


def summary(rows):
    retained = rows[5:]
    mean = {field: statistics.mean(row[field] for row in retained) for field in FIELDS}
    return {'frames_observed': len(rows), 'discarded': 5, 'frames_measured': len(retained),
            'mean_ms': mean, 'total_sample_sd_ms': statistics.stdev(row['total_ms'] for row in retained),
            'fps_from_mean_total': 1000.0 / mean['total_ms']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    try:
        cpu = summary(load(args.directory / 'cpu.log', 'cpu'))
        fpga = summary(load(args.directory / 'fpga.log', 'fpga'))
        result = {'cpu': cpu, 'fpga': fpga,
                  'preprocess_time_ratio': cpu['mean_ms']['preprocess_ms'] / fpga['mean_ms']['preprocess_ms'],
                  'fps_ratio': fpga['fps_from_mean_total'] / cpu['fps_from_mean_total'],
                  'scope': 'Log statistics only. CPU bicubic vs FPGA bilinear; sequential live frames differ.'}
    except (OSError, ValueError, ZeroDivisionError) as exc:
        parser.exit(2, f'INVALID: {exc}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()

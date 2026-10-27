#!/usr/bin/env python3
"""Recompute selected ARM98 and V2 R3 recorded metrics; no board access."""
import csv
import hashlib
from pathlib import Path
import re
import statistics

ROOT = Path(__file__).resolve().parents[1] / 'board/measurements'
MODEL = '7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4'

def main():
    arm = ROOT / 'arm98_20261027'
    log = (arm / 'arm98_records.txt').read_text(encoding='utf8')
    if 'kind=REAL_BOARD_MEASURED' not in log or 'model='+MODEL not in log:
        raise ValueError('missing recorded board/model identity')
    frames = re.findall(r'^ARM98_FRAME frame=(\d+) total_ns=(\d+) conv_count=98 hardware_submits=0$', log, re.M)
    if [int(x[0]) for x in frames] != list(range(20)):
        raise ValueError('ARM98 missing or unordered frames')
    with (arm/'conv98.csv').open(encoding='utf8', newline='') as f:
        conv = list(csv.DictReader(f))
    if len(conv) != 98 or any(int(x['samples']) != 20 for x in conv):
        raise ValueError('ARM98 convolution row/sample count')
    with (arm/'layers.tsv').open(encoding='utf8', newline='') as f:
        rows = list(csv.DictReader(f, delimiter='\t'))
    per_frame = {i: [] for i in range(20)}
    for row in rows:
        per_frame[int(row['frame'])].append(row)
    if any(len(xs) != 213 or sum(int(x['conv_id']) > 0 for x in xs) != 98 for xs in per_frame.values()):
        raise ValueError('ARM98 per-frame operator count')
    for row in conv:
        matches = [x for x in rows if x['index'] == row['instruction_index']]
        if len(matches) != 20 or abs(statistics.mean(int(x['total_ns']) for x in matches)/1e6 - float(row['mean_ms'])) > 1e-6:
            raise ValueError('ARM98 CSV/TSV mismatch: ' + row['conv_id'])
    for j in range(2):
        hashes = {hashlib.sha256((arm/f'run{i}_output{j}.bin').read_bytes()).hexdigest() for i in range(20)}
        if len(hashes) != 1:
            raise ValueError('ARM98 repeated fixed-input output mismatch')
    print('ARM98 frames=20 convs=98 hw_submits=0 mean_ms={:.3f} conv_sum_ms={:.3f} repeated_outputs_identical=yes'.format(
        statistics.mean(int(x[1]) for x in frames)/1e6, sum(float(x['mean_ms']) for x in conv)))
    lines=(ROOT/'v2_r3_20261024/serial_measurement_excerpt.txt').read_text(encoding='utf8').splitlines()
    golden=[x for x in lines if x.startswith('BOARD_GOLDEN_RESULT ')]
    accuracy=[x for x in lines if x.startswith('FULL_MODEL_ACCURACY ')]
    profiles=[x for x in lines if x.startswith('EE_FUSION_PROFILE ')]
    if len(golden)!=21 or any(not all(k in x for k in ('errors=0','locked=0','mismatching_final_int32=0')) for x in golden):
        raise ValueError('R3 Golden results incomplete or failing')
    if len(accuracy)!=20 or any('pass=1' not in x or 'unchanged_gate=1' not in x for x in accuracy):
        raise ValueError('R3 numerical comparison incomplete or failing')
    if len(profiles)!=21 or any(not all(k in x for k in ('submitted=1','completed=1','result_returned=1')) for x in profiles):
        raise ValueError('R3 submit/completion incomplete')
    def avg(key):
        return statistics.mean(int(re.search(r'\b'+key+r'=(\d+)',x).group(1)) for x in profiles[-20:])/1e6
    print('V2_R3 golden=21/21 accuracy=20/20 pack_ms={:.3f} decode_ms={:.3f} driver_monitor_ms={:.3f} offload_ms={:.3f}'.format(
        avg('pack_ns'),avg('decode_ns'),avg('driver_compute_monitor_ns'),avg('production_total_ns')))
    for line in lines:
        if line.startswith(('FULL_PICODET_SUMMARY ','FULL_PICODET_COMPARE ')):
            print(line)
    print('Scope: historical board evidence, fixed input; not the current Eyes/PicoDet+C board test or camera FPS.')

if __name__ == '__main__':
    main()
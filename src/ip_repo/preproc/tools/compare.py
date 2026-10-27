#!/usr/bin/env python3
"""Compare three Verilog hex-plane dumps against the full FP32 golden tensor."""
from pathlib import Path
import argparse
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('name')
    ap.add_argument('--output-dir',type=Path,default=ROOT/'build')
    ap.add_argument('--prefix',default=None,help='output file prefix; defaults to name')
    ap.add_argument('--atol',type=float,default=1e-6)
    a=ap.parse_args()
    values=[]
    for c in 'rgb':
        p=a.output_dir/f'{a.prefix or a.name}_{c}.hex'
        words=np.array([int(s,16) for s in p.read_text().split()],dtype='<u4')
        if len(words)!=102400: raise SystemExit(f'{p}: {len(words)} values, expected 102400')
        values.append(words.view('<f4'))
    actual=np.stack(values)
    golden=np.fromfile(ROOT/'vectors'/f'{a.name}.golden.f32',dtype='<f4').reshape(3,320,320)
    delta=np.abs(actual.reshape(3,320,320).astype(np.float64)-golden.astype(np.float64))
    exact=np.count_nonzero(actual.reshape(3,320,320).view('<u4')==golden.view('<u4'))
    print(f'{a.name}: max_abs={delta.max():.9g} mean_abs={delta.mean():.9g} '
          f'exact={exact}/{delta.size} over_atol={np.count_nonzero(delta>a.atol)}')
    if np.any(~np.isfinite(actual)) or np.any(delta>a.atol): raise SystemExit(1)

if __name__=='__main__': main()

#!/usr/bin/env python3
"""Quantify tensor differences to the supplied ARM reference's a=-0.75 bicubic rule.

This is a vectorized mathematical implementation of that CPU routine; it does not
run the PicoDet model. Detection comparison requires the target model/runtime.
"""
from pathlib import Path
import argparse
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
MEAN=np.array([.485,.456,.406],dtype=np.float64)
STD=np.array([.229,.224,.225],dtype=np.float64)

def weight(x):
    a=-.75
    x=np.abs(x)
    return np.where(x<=1,(a+2)*x**3-(a+3)*x**2+1,
                    np.where(x<2,a*x**3-5*a*x**2+8*a*x-4*a,0))

def bicubic(frame):
    yy=(np.arange(320,dtype=np.float64)+.5)*(360/320)-.5
    xx=(np.arange(320,dtype=np.float64)+.5)*2-.5
    iy=np.floor(yy).astype(int); ix=np.floor(xx).astype(int)
    accum=np.zeros((320,320,3),dtype=np.float64)
    weights=np.zeros((320,320,1),dtype=np.float64)
    for ky in range(-1,3):
        sy=np.clip(iy+ky,0,359)
        wy=weight(yy-(iy+ky))[:,None,None]
        for kx in range(-1,3):
            sx=np.clip(ix+kx,0,639)
            ww=wy*weight(xx-(ix+kx))[None,:,None]
            accum+=frame[sy[:,None],sx[None,:]].astype(np.float64)*ww
            weights+=ww
    pix=np.clip(accum/weights,0,255)
    return ((pix/255-MEAN)/STD).transpose(2,0,1).astype('<f4')

def main():
    p=argparse.ArgumentParser()
    p.add_argument('name',nargs='?',default='captured')
    a=p.parse_args()
    raw=np.fromfile(ROOT/'vectors'/f'{a.name}.rgb',dtype=np.uint8).reshape(360,640,3)
    bil=np.fromfile(ROOT/'vectors'/f'{a.name}.golden.f32',dtype='<f4').reshape(3,320,320)
    bic=bicubic(raw)
    diff=np.abs(bil.astype(np.float64)-bic.astype(np.float64))
    print(f'{a.name}: max_abs={diff.max():.9g}, mean_abs={diff.mean():.9g}, '
          f'p99_abs={np.quantile(diff,.99):.9g}, rmse={np.sqrt(np.mean(diff**2)):.9g}, '
          f'changed_values={np.count_nonzero(diff)}/{diff.size}')
    bic.tofile(ROOT/'vectors'/f'{a.name}.bicubic.f32')

if __name__=='__main__': main()

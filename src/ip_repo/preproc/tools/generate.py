#!/usr/bin/env python3
"""Recreate ROM images, deterministic frames, and independent FP32 golden tensors."""
from pathlib import Path
import argparse
import struct
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
W, H, OW, OH = 640, 360, 320, 320
MEAN = np.array([0.485, 0.456, 0.406], dtype=np.float64)
STD = np.array([0.229, 0.224, 0.225], dtype=np.float64)


def preprocess(frame: np.ndarray) -> np.ndarray:
    if frame.shape != (H, W, 3) or frame.dtype != np.uint8:
        raise ValueError('expected 360x640 RGB888')
    x = np.clip((np.arange(OW, dtype=np.float64) + .5) * 2 - .5, 0, W-1)
    y = np.clip((np.arange(OH, dtype=np.float64) + .5) * (H/OH) - .5, 0, H-1)
    x0, y0 = np.floor(x).astype(np.int32), np.floor(y).astype(np.int32)
    x1, y1 = np.minimum(x0+1,W-1), np.minimum(y0+1,H-1)
    fx, fy = (x-x0)[None,:,None], (y-y0)[:,None,None]
    p00 = frame[y0[:,None],x0[None,:]].astype(np.float64)
    p01 = frame[y0[:,None],x1[None,:]].astype(np.float64)
    p10 = frame[y1[:,None],x0[None,:]].astype(np.float64)
    p11 = frame[y1[:,None],x1[None,:]].astype(np.float64)
    pix = (p00*(1-fx)*(1-fy) + p01*fx*(1-fy)
           + p10*(1-fx)*fy + p11*fx*fy)
    return ((pix/255.0-MEAN)/STD).transpose(2,0,1).astype('<f4')


def frames(captured: bytes | None):
    if captured is not None:
        if len(captured) != W*H*3:
            raise ValueError('captured frame must be 691200 bytes')
        yield 'captured', np.frombuffer(captured,dtype=np.uint8).reshape(H,W,3)
    yield 'zero', np.zeros((H,W,3),dtype=np.uint8)
    yield 'full', np.full((H,W,3),255,dtype=np.uint8)
    xx = np.arange(W,dtype=np.uint16)[None,:]
    yy = np.arange(H,dtype=np.uint16)[:,None]
    p = np.empty((H,W,3),dtype=np.uint8)
    p[:,:,0] = (xx*3+yy*5)%256
    p[:,:,1] = (xx*7+yy*11+37)%256
    p[:,:,2] = (xx*13+yy*17+91)%256
    yield 'distinct', p
    p = np.zeros((H,W,3),dtype=np.uint8)
    p[0,0,0] = 255; p[0,W-1,1] = 255; p[H-1,0,2] = 255
    p[H-1,W-1] = [255,127,63]
    yield 'edge', p


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--captured',type=Path,help='supplied 691200-byte RGB frame')
    args = parser.parse_args()
    (ROOT/'vectors').mkdir(exist_ok=True)
    for c,name in enumerate(('r','g','b')):
        with (ROOT/'rtl'/f'normalize_{name}.mem').open('w',newline='\n') as f:
            for n in range(8161):
                value = np.float32((n/(32.0*255.0)-MEAN[c])/STD[c])
                f.write(f'{struct.unpack("<I",value.tobytes())[0]:08x}\n')
    for name,p in frames(args.captured.read_bytes() if args.captured else None):
        raw = p.tobytes()
        (ROOT/'vectors'/f'{name}.rgb').write_bytes(raw)
        with (ROOT/'vectors'/f'{name}.hex').open('w',newline='\n') as f:
            for i in range(0,len(raw),3):
                f.write(f'{raw[i+2]:02x}{raw[i+1]:02x}{raw[i]:02x}\n')
        with (ROOT/'vectors'/f'{name}.hex32').open('w',newline='\n') as f:
            for i in range(0,len(raw),4):
                f.write(f'{int.from_bytes(raw[i:i+4],"little"):08x}\n')
        preprocess(p).tofile(ROOT/'vectors'/f'{name}.golden.f32')
        print(name, len(raw), (ROOT/'vectors'/f'{name}.golden.f32').stat().st_size)


if __name__ == '__main__': main()

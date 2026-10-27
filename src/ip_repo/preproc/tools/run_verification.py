#!/usr/bin/env python3
"""Rebuild the Verilog tests and check all complete frame tensors."""
import os,shutil,subprocess,sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
BUILD=ROOT/'build'
BUILD.mkdir(exist_ok=True)
BIN=Path(os.environ['IVERILOG_BIN']) if 'IVERILOG_BIN' in os.environ else None
def exe(name):
    p=(BIN/(name+'.exe')) if BIN else shutil.which(name)
    if not p: raise SystemExit(f'{name} missing; set IVERILOG_BIN to its bin folder')
    return str(p)
IV=exe('iverilog'); VVP=exe('vvp')
def run(cmd,cwd=ROOT):
    # Some Windows Icarus 12.0 devel builds intermittently exit with 0xC0000005
    # before elaboration. Retry the external process; never accept failed output.
    for attempt in range(3):
        p=subprocess.run([str(x) for x in cmd],cwd=cwd)
        if p.returncode==0: return
        if p.returncode not in (3221225477,-1073741819) or attempt==2:
            raise subprocess.CalledProcessError(p.returncode,cmd)
        print('Icarus process startup fault; retrying',file=sys.stderr)
def compile_test(name,files):
    out=BUILD/('verify_'+name+'.vvp')
    run([IV,'-g2005-sv','-s',name,'-o',out.relative_to(ROOT),*files])
    return out
core=compile_test('tb_preproc',['rtl/preproc_axis_core.v','tb/tb_preproc.v'])
for name in ('zero','full','distinct','edge','captured'):
    run([VVP,core,f'+INPUT=../vectors/{name}.hex',f'+OUTPUT=../build/{name}'],ROOT/'rtl')
    run([sys.executable,ROOT/'tools'/'compare.py',name])
unpack=compile_test('tb_unpack',['rtl/rgb888_mm2s_adapter.v','tb/tb_unpack.v'])
run([VVP,unpack,'+WORDS=vectors/distinct.hex32','+PIXELS=vectors/distinct.hex'])
dma=compile_test('tb_preproc_dma',['rtl/preproc_axis_core.v',
  'rtl/rgb888_mm2s_adapter.v','tb/tb_preproc_dma.v'])
run([VVP,dma,'+INPUT=../vectors/captured.hex32',
     '+OUTPUT=../build/captured_dma'],ROOT/'rtl')
run([sys.executable,ROOT/'tools'/'compare.py','captured','--prefix','captured_dma'])
control=compile_test('tb_control',['rtl/preproc_axis_core.v',
  'rtl/rgb888_mm2s_adapter.v','rtl/picodet_preproc_ip.v','tb/tb_control.v'])
run([VVP,control],ROOT/'rtl')
print('ALL VERIFICATION PASSED')

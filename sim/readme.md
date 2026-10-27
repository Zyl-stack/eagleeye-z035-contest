# Verification entry points

- Preprocessing IP: `../src/ip_repo/preproc/tb/` and
  `../src/ip_repo/preproc/scripts/sim_xsim.tcl`.
- Servo PWM: `../src/servo/tb_roi_servo_pwm.sv`.
- Camera/ROI host checks: `../src/ps/tests/`.
- Golden input and output tensor: `../data/captured.rgb` and
  `../data/captured.golden.f32`.
- Reusable FP32 checker: `../skill/preproc-fp32-golden/tools/verify_tensor.py`.

The large five-frame vector set is deliberately not stored twice. Generate it
from the included captured frame before using the preprocessing XSim script:

```sh
python src/ip_repo/preproc/tools/generate.py --captured data/captured.rgb
cd src/ip_repo/preproc
vivado -mode batch -source scripts/sim_xsim.tcl -tclargs captured
```

`generate.py` needs NumPy. The XSim script uses `PYTHON` if set, otherwise
`python` on `PATH`. Generated `vectors/` and `build/` are test outputs, not
part of this submitted snapshot.

These are the copied source tests; no new simulation was run while assembling
this compact submission snapshot. Record the command, tool version, input
hashes and pass/fail output when producing the final verification log.

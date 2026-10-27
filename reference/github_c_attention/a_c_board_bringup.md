# A/C Board Bring-up Checklist

This checklist is for the first real Zynq PS/PL validation. The repository currently contains PC simulation and Vitis source preparation only. It has not been hardware verified.

## Before powering the board

1. Connect board power, JTAG, and UART. Record the UART COM port and the baud rate configured by the standalone BSP.
2. Open the correct Vitis 2022.2 workspace.
3. Confirm the projects are `eagleeye_platform_v2` and `eagleeye_fovea_smoke_test_v2`, using processor `ps7_cortexa9_0` and domain `standalone_domain`.
4. Confirm the Platform was generated from the same Vivado design as the bitstream. Check the generated BSP `xparameters.h` for `XPAR_FOVEA_AXIL_CTRL_0_BASEADDR`; the expected current value is `0x40000000`, but the generated file is authoritative.

## Level 1: fixed ROI smoke test

The Level 1 application produces `eagleeye_fovea_smoke_test_v2.elf`.

5. In Vivado, program the bitstream and confirm the A video pipeline has a running frame start. These are manual hardware steps; Codex cannot replace them.
6. Build and run the existing fixed `vitis_axi_smoke_test.c` application. It produces `eagleeye_fovea_smoke_test_v2.elf` and sends `request_id=1`, `target_id=1`, `source_frame_id=1`, `ROI=(160,90,320,180)`, and `MODE=BILINEAR`.
7. Observe UART output similar to:

```text
FOVEA REQUEST SENT
ROI=(160,90,320,180)
MODE=BILINEAR
WAITING FOR FRAME LATCH
REQUEST LATCHED
```

`REQUEST LATCHED` means the command was accepted at a frame boundary. It does not mean image processing has completed.

## Level 2: dynamic ROI and mode smoke test

The Level 2 application produces `eagleeye_dynamic_fovea_test_v2.elf`.

8. Build `eagleeye_dynamic_fovea_test_v2`, which contains `vitis_dynamic_fovea_smoke_test.c`, the repository `include/` headers, and these Vitis sources: `src/platform_io_vitis.c`, `src/a_fovea_adapter.c`, `src/attention.c`, `src/enhance_policy.c`, and `src/roi_smoother.c`. It does not instantiate or run the Scheduler.
9. Run `eagleeye_dynamic_fovea_test_v2.elf` only after Level 1 succeeds. It uses five deterministic frames and does not require B detection or a camera:

| Frame | Fake input | Expected policy | Purpose |
|---:|---|---|---|
| 1 | player near center | BILINEAR | ROI near `(160,90)` |
| 2 | player moves right | BILINEAR | smoother movement |
| 3 | player moves right again | BILINEAR | continued ROI movement |
| 4 | high-priority BALL | LANCZOS | dynamic mode selection |
| 5 | no valid target | BYPASS | no invalid AXI request |

Each valid request goes through `roi_smoother -> a_fovea_adapter -> fovea_submit_request`. The adapter must preserve the final contract `roi_w=320`, `roi_h=180`, `roi_x=0..320`, and `roi_y=0..180`. The mode sent to `ENHANCE_MODE @ 0x1C` remains exactly `0/1/2`.

## Troubleshooting

| Symptom | First checks |
|---|---|
| No UART output | COM port, baud rate, PS running, ELF startup, and UART terminal selection. |
| `WAITING FOR FRAME LATCH` but no `REQUEST LATCHED` | A video chain, frame-start signal, and `roi_cmd_accept`/frame-boundary acceptance. Do not immediately blame AXI access. |
| STATUS error | Read back register values, ROI bounds, mode value, request state, and the A-side error/clear behavior. |
| ROI does not move | Check the C `roi_smoother`, `a_fovea_adapter` output, and the AXI-Lite ROI_XY/ROI_WH writes. |
| Mode does not switch | Check `enhance_policy` reason/mode output and the `ENHANCE_MODE` register write at `0x1C`. |
| Base address is unavailable | Refresh the Platform from the matching XSA and inspect generated `xparameters.h`. Never substitute a guessed address in source. |

## Boundaries

- `platform_io_vitis.c` must use the BSP-generated Base Address and must not access address 0 when the macro is unavailable.
- `STATUS.done` is a frame-boundary latch acknowledgement, not an image-completion signal.
- Do not add BSP, `xparameters.h`, ELF, `Debug/`, `export/`, XSA, or workspace metadata to Git.
- Vivado wiring, bitstream generation, Vitis Platform generation, programming, UART observation, and all real timing/image claims require the team and hardware. They cannot be replaced by PC tests or static source review.

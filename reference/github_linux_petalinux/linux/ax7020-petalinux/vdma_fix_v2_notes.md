# AX7020 PicoDet VDMA software test v2

Status: ARM compile/link and host scheduler tests passed. Continuous capture is
NOT hardware verified. Keep the original SD package as rollback.

## Hardware evidence and fault

- Board S2MM CR `0x00010003` had Circular_Park=1. The old application assumed
  the current write store would rotate and treated a pointer change as the only
  proof of a new frame. It failed on its second capture.
- Board S2MM DMASR `0x00015810` contains VDMAIntErr (bit 4), SOFLateErr
  (bit 11), FrmCnt_Irq (bit 12), and Err_Irq (bit 14). The frame-size error
  means the input timing/VSIZE contract needs checking; pointer scheduling
  alone cannot resolve it.
- The supplied XSA identifies `axi_vdma_0` as MM2S, `axi_vdma_1` as S2MM,
  AXI VDMA v6.3, three frame stores and a 24-bit S2MM stream. The existing
  Linux application requests 640x360 RGB888 with 1920-byte stride. The
  bare-metal demo used 1280x720. Neither proves what the live PL stream sends.

## Changes in this isolated copy

Repository scope: this handoff contains the Linux user-space VDMA overlay.
Apply it to the existing AX7020 material package, which retains the supplied
`reference_baremetal/ov5640.c` sensor register table. That unmodified vendor
table is intentionally not duplicated here because its redistribution license
has not been verified.

- VDMA channels now request parking rather than circular mode. Software
  changes the write reference only after observing the S2MM frame interrupt,
  then waits for the current write store to move before publishing the old
  store. Invalid status, unexpected store transitions, repeated completion,
  no free store, and timeout are fatal; no error frame is sent to PicoDet.
- The MM2S display reference follows a completed store only on an MM2S frame
  boundary. The next S2MM store excludes both current and requested display
  stores and any CPU-pinned snapshot. If the display switch does not complete
  in time, capture stops rather than overwriting a read store.
- A polling thread owns frame scheduling. The inference thread waits for a
  newer sequence, pins a completed store, copies it to a private image buffer,
  and releases the store before preprocessing/inference. A mutex and condition
  variable protect publication and error propagation. The thread is joined on
  shutdown; partially initialized DMA channels are stopped on failure.
- Readback validates the configured VSIZE, HSIZE, stride and three buffer
  addresses before capture. `/dev/mem` uses `O_SYNC`; the supplied Device Tree
  reserves `0x1e000000..0x1e20ffff` with `no-map`. Real cache coherence and
  incoming line/frame timing still need board verification.
- Application logs `FRAME completed_sequence`, `SNAPSHOT frame_sequence`,
  checksum, and `SNAPSHOT_COPY ... copy_ms`. On error it logs the VDMA register
  snapshot, timeout count and error count. The BusyBox date form is fixed in
  both board scripts. Diagnostics only read registers; they do not clear W1C.

## Software verification

- Native C++14 `-Wall -Wextra -Werror` scheduler/concurrency test passed.
- `sh -n` passed for both board scripts.
- PetaLinux 2022.2 Yocto Cortex-A9 ARM toolchain compiled and linked the
  statically linked ARM EABI5 `paddle_person_v2` executable. One linker warning
  persists from static `libgomp` using `dlopen`; no link errors.
- The v2 package copies the original SD directory, changes only the
  application and two board scripts, refreshes SHA256SUMS, and preserves
  BOOT.BIN, image.ub, model, labels and configuration byte-for-byte.

## Remaining board-dependent risk

- S2MM `SOFLateErr` may recur if the actual input has more than 360 lines.
  Check sensor output timing, PL AXI4-Stream SOF/TLAST, programmed VSIZE/HSIZE,
  and observed error registers. Do not increase the frame size without checking
  the 0xB0000-byte slot and reserved memory.
- Polling a frame interrupt is not a substitute for measured board timing.
  Verify that the display and write store handoffs leave a free slot before
  the next camera frame. If not, a kernel interrupt/DMA-BUF design or hardware
  timing change may be needed; this test build intentionally fails closed.
- Verify that the `/dev/mem` reserved mapping is CPU/DMA coherent on the board
  and that RGB channel order matches the sensor-to-VDMA path.

No RTL, Vivado BD, XSA, bitstream, BOOT.BIN, image.ub, Device Tree or model
content was edited. No board test was run here.

Register semantics: AMD AXI VDMA v6.3 Product Guide PG020, especially
`S2MM_VDMASR`, `S2MM_VDMACR`, and `PARK_PTR_REG`:
https://docs.amd.com/r/en-US/pg020_axi_vdma/

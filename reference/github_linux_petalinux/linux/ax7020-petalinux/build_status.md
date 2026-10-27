# Build Status

Date: 2026-10-09

## Verified environment

- WSL2: Ubuntu-20.04
- Architecture: x86_64 host
- PetaLinux: 2022.2
- Linux user: `eagleuser`
- Build storage: Linux ext4 under `/home/eagleuser`
- Hardware source: supplied AX7020 `system_wrapper.xsa` and matching bitstream

## Results

- `build_petalinux.sh`: exit code 0
- PetaLinux BitBake: 4,232 tasks attempted; all succeeded
- `make_sd_package.sh`: exit code 0
- `BOOT.BIN`: generated
- `image.ub`: generated
- `system.dtb`: generated
- `SHA256SUMS`: verified
- SD package archive: generated and readable

## Known warnings and limits

Bootgen reported these warnings:

```text
[WARNING]: Partition zynq_fsbl.elf.0 range is overlapped with partition system_wrapper.bit.0 memory range
[WARNING]: Partition system_wrapper.bit.0 range is overlapped with partition system.dtb.0 memory range
```

The generated BIF contains FSBL, the supplied bitstream, U-Boot, and `system.dtb` with an explicit load address of `0x00100000`. The warnings were not resolved by software-only inspection. The hardware owner must confirm the Zynq boot-memory layout and perform board bring-up before treating the image as boot-safe.

The build also reported that `/tftpboot` was unavailable. TFTP was not required for this offline artifact build.

Not verified in this build:

- AX7020 board boot
- UART boot log
- OV5640 sensor capture
- PicoDet inference on target
- real-time throughput, latency, or power
- SD-card boot behavior

## Reproducibility note

The original supplied version probe used an unsupported `petalinux-create --version` option in PetaLinux 2022.2. The Linux copy used for verification contains the authorized one-line check against `PETALINUX_VER=2022.2`. No other build step was changed.

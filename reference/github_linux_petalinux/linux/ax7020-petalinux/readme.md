# AX7020 + OV5640 + PicoDet PetaLinux 2022.2

This directory contains the reviewable software-side build inputs for the EagleEye AX7020 Linux handoff.

## Scope

- Board target: AX7020 with Zynq-7000
- Camera target: OV5640
- Linux build: PetaLinux 2022.2 on Ubuntu 20.04 64-bit
- Build host used for verification: WSL2, user `eagleuser`
- Hardware description: the matching `system_wrapper.xsa` and bitstream are supplied separately by the hardware owner

## Build status

PetaLinux 2022.2 image generation and SD packaging passed. This does **not** mean that the board boots successfully.

AX7020 board boot, OV5640 capture, PicoDet inference, and real-time performance have not yet been verified on hardware.

## Build

1. Use Ubuntu 20.04 64-bit and install PetaLinux 2022.2 using the AMD-provided installer.
2. Source the tool environment:

   ```bash
   source /path/to/petalinux/2022.2/settings.sh
   ```

3. Place the matching hardware files, supplied out of band, at:

   ```text
   hardware/system_wrapper.xsa
   hardware/system_wrapper.bit
   ```

4. Run the supplied scripts from the handoff root:

   ```bash
   chmod +x integration/linux_ov5640_picodet/scripts/*.sh
   ./integration/linux_ov5640_picodet/scripts/build_petalinux.sh
   ./integration/linux_ov5640_picodet/scripts/make_sd_package.sh
   ```

The build script creates a separate PetaLinux project and writes generated files outside this directory by default. Do not build from `/mnt/c`; use Linux ext4 storage.

## Hardware boundary

The matching XSA and bitstream must come from the same Vivado design revision. Do not substitute another board design. The generated Bootgen image contains FSBL, bitstream, U-Boot, and device tree; the Bootgen address-overlap warnings still require hardware-owner review before board bring-up.

## Excluded artifacts

This preview intentionally excludes XSA/bitstream files, BOOT.BIN, image.ub, system.dtb, model files, application binaries, PetaLinux SDK/installers, Yocto caches, and raw build logs. They are large, hardware-specific, licensed, or not yet suitable for public repository storage.

See `BUILD_STATUS.md` and `ARTIFACT_SHA256.md` for the verified local build record.

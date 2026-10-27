# Board package

The SD boot partition for the recorded Eyes configuration used these four
files. This directory is **not** an SD card and contains no automatic writer.

| SD filename | Snapshot location | SHA256 |
| --- | --- | --- |
| `BOOT.BIN` | `board/BOOT.BIN` | `54003cd2490c38d5caf208791e2995de7112a8e3b5b826a39b0f1811735e2765` |
| `image.ub` | `board/sd_boot/image.ub` | `4dabcb81b69988e77bb1d0e550cba4b7de4419dec3e107c24aaac4960e21a7ad` |
| `boot.scr` | `board/sd_boot/boot.scr` | `7013e14923fbe461c4ecd745214fcafcb0ffaddb10865f9135cd8624637359f4` |
| `linux_camera_smooth_trial_v7_20261013.tar.gz` | `board/sd_boot/` | `3a6c372696c3aee29ff3a5db315d6135a013c34ba4a40417aae48d8aec756374` |

`BOOT.BIN` was assembled from the final Eyes bitstream and the preserved FSBL,
U-Boot and DTB. `image.ub` includes the USB PHY host correction used for the
mouse. The application archive is the mouse-autostart variant, not the earlier
CPU-only v7 archive with the same filename. The source deployment notes are
copied here for provenance.

`bootgen_inputs.tar.gz` contains the exact FSBL ELF, bitstream, U-Boot ELF and
DTB used by `bootgen.bif`. Extract it into a separate build directory alongside
that BIF when reproducing BOOT.BIN; do not overwrite the recorded BOOT here.

For a repeatable field test, use a separate backed-up SD card, inspect the
actual boot media and power wiring, read hashes back after copying, and keep
the tested card unchanged until recovery is confirmed. No claim is made that
this package alone contains the external PetaLinux/Paddle-Lite build inputs.

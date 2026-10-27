# EagleEye FPGA Integration Files

This package contains the FPGA/PL side files for the EagleEye competition project.

Current verified PL features:

- OV5640 camera input, 640x360 video stream.
- HDMI 1280x720 output.
- Three display modes:
  - mode 0: BYPASS / direct display.
  - mode 1: Bilinear 2x ROI enhancement.
  - mode 2: Lanczos 2x ROI enhancement.
- Fixed ROI size: 320x180.
- ROI top-left coordinate input range:
  - roi_x: 0 to 320.
  - roi_y: 0 to 180.
- ROI frame overlay and attention heatmap OSD.
- AXI-Lite control module added: fovea_axil_ctrl_0.

AXI-Lite address in the exported XSA:

- Instance: fovea_axil_ctrl_0
- Base address: 0x40000000
- High address: 0x40000FFF

Directory meaning:

- fpga_vivado/rtl: Verilog RTL source files.
- fpga_vivado/constraints: board and video constraints.
- fpga_vivado/project: Vivado project files, BD file, and exported XSA.
- fpga_vivado/ip/rgb2dvi_v1_3: HDMI rgb2dvi IP source.
- fpga_vitis/ov5640_sd/src: current Vitis camera/VDMA/HDMI initialization source.
- fpga_interface: PS/PL register interface notes.

Current integration status:

- Vivado side AXI-Lite slave has been added and connected in the exported XSA.
- Vitis platform/BSP still needs to be refreshed from the new XSA before smoke test.
- The first smoke test should only verify: PS writes ROI/mode, PL latches at frame boundary, STATUS.done is readable.

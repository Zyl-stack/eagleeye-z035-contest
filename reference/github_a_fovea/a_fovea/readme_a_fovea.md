# A Fovea / Vivado Sources

## Purpose

This directory contains the A member's imported bare-metal/Vivado source snapshot for the EagleEye project. It is isolated from the C-side simulation sources in the repository root.

## Imported layout

### `EagleEye/`

The imported Vitis-side application sources and Zynq support files:

- `ov5640_sd/ov5640_sd.prj`
- `ov5640_sd/src/`: OV5640 camera, VDMA, display, I2C, platform, and interrupt C sources
- `system_wrapper/platform.tcl`
- `system_wrapper/hw/`: generated PS initialization source retained as a hardware-export reference

### `Vivado_EagleEye/`

The Vivado project and reusable source/configuration files:

- `ov5640_sd.xpr`
- `ov5640_sd.srcs/constrs_1/`: board constraints
- `ov5640_sd.srcs/sources_1/bd/system/system.bd`: block design
- `ov5640_sd.srcs/sources_1/bd/system/ip/`: checked-in IP configuration files
- `ov5640_sd.srcs/sources_1/imports/rtl/`: video, ROI canvas, scaler, heatmap and motion-radar RTL
- `repo/`: local custom IP repositories for camera, display and clocking support

## Current interface findings

The imported snapshot contains AXI/AXI-Lite interfaces belonging to existing Vivado IP such as AXI VDMA and AXI dynamic clocking. The snapshot does **not** contain a dedicated `fovea_axil_ctrl` module, a `fovea_axil_ctrl_0` instance, or the requested `roi_cmd_*` / `selected_target_id` control interface.

The current custom RTL is primarily video-stream based. For example:

- `video_mode_switch_top`: clock/reset, mode key, AXI4-Stream input/output, and `mode_active`
- `fovea_canvas_1280x720_top`: clock/reset, enable, `use_lanczos`, ROI X/Y, and AXI4-Stream input/output
- `attention_heatmap_osd`: clock/reset, frame tick, pixel coordinates, ROI coordinates, mode and valid; pixel/hit outputs
- `bilinear_scaler_rgb_top`: AXI4-Stream input/output and serial coordinate outputs

These are not a substitute for the frozen C-to-A Fovea AXI-Lite register map. A-side and C-side must still agree on the future control IP module name, register implementation, status behavior, and physical connections before hardware integration.

## Exclusions

Generated Vivado/Vitis caches, run directories, simulation/netlist output, backup snapshots, BSP/libsrc trees, exported build products, logs/journals, and files at or above 100 MB were excluded from this import. The original A directories were not modified.

The Vivado project descriptor still contains historical absolute import paths from the original machine/project. Those paths were reported but not rewritten in this import; teammates should relink or re-add missing sources when opening the project on another machine.

## How C teammates use this branch

Clone the repository and switch to `a-fovea` to inspect A's imported sources. The root `src/` and `include/` remain the C-side simulation implementation. Do not add A RTL directly to the C Visual Studio project. Hardware integration should happen in Vivado/Vitis after the A-side Fovea AXI-Lite control IP and its address/register contract are frozen.

## Provenance

Imported from local snapshots:

- `C:\Users\LENOVO\Desktop\嵌入式\dth\EagleEye\EagleEye`
- `C:\Users\LENOVO\Desktop\嵌入式\dth\Vivado_EagleEye\Vivado_EagleEye`


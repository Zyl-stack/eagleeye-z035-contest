# Eyes 构建工具链与应用复现

本页保存工程快照、工具链和两版应用的构建参数。命令面向按[复现指南](../report/reproduction_zh.md)恢复后的大小写工程目录；当前实测组合见[版本表](../report/evidence/release_identity_zh.md)。

## 工程快照身份

- Z035 Vivado Eyes 工程：本包 `src/vivado/EagleEye_7035_Eyes.srcs` 的 63 个文件与本机 `D:/EagleEye_FPGA_Competition/Vivado_EagleEye_7035_Eyes/EagleEye_7035_Eyes/EagleEye_7035_Eyes.srcs` 逐文件 SHA256 一致。`system.bd` SHA256 为 `2bf8317351d9a591a30f3a5f6f9a3cb40bb52723597723599cc86048f749f222`。
- 与 `board/BOOT.BIN` 配套的实现 BIT SHA256 为 `09486c3e7f0410482aad560d0e4aa22bc6f604fc140518ab61e06ebe203e6397`，它与 `board/bootgen_inputs.tar.gz` 内的 `system.bit` 及原工程 `impl_1/z100_top.bit` 一致。BOOT.BIN SHA256 为 `54003cd2490c38d5caf208791e2995de7112a8e3b5b826a39b0f1811735e2765`。
- 原稳定应用是 `src/ps_as_built/` 的九宫格 `GridPolicy`，模型是 `data/model.nb`。后续 `src/ps/` 的注意力 ROI 版本已编译成独立应用归档，见 `board/attention/`；它与原稳定版共用 BOOT、image.ub、boot.scr。应用日志与功能确认见[板端应用记录](../report/evidence/board_app_evidence_20261028_zh.md)，连续演示录像尚未归档。`src/attention_reference/` 仍只是 C 参考子集，并非板端程序。

## Vivado 2022.2 干净重建入口

把整个 `eagleeye_z035` 目录**复制**到短的纯 ASCII 路径，例如 `D:/EEZ035`。Windows 下 Vivado 2022.2 对深路径、空格和中文路径容易报错。然后运行：

```powershell
& 'D:\Vivado\Vivado\2022.2\bin\vivado.bat' -mode batch -source 'D:\EEZ035\build\recreate_vivado.tcl'
```

脚本从随包的 RTL/BD/XCI/XDC 和 OV5640、动态时钟、预处理 IP Repository 创建新的 `build/vivado_work/eagleeye_z035.xpr`。2026-10-27 在独立复制目录 `D:/EEZ035R6` 实测：Vivado 2022.2 退出码 0，BD Validation 与 Output Products 生成通过。此前失败原因是脚本只复制 BD、遗漏其 `ip/*.xci`，且 BD/XCI 的相对生成路径落到工程外；这两点现已修复。完整综合、实现及 BIT/XSA 使用[全流程构建入口](build_full_vivado.tcl)，结果见[干净复现记录](../report/evidence/clean_reproduction_20261028_zh.md)；板端 BIT 身份见[上板记录](../report/evidence/board_reproduced_20261028_zh.md)。

## PS 已上板应用重编

需 Linux/WSL 中的 PetaLinux 2022.2 构建工程（含 `build/tmp/sysroots-components` 和 fsbl recipe sysroot），交叉编译器 `arm-xilinx-linux-gnueabi-g++` GCC 11.2.0。本机重建所用编译器文件 SHA256 `6c28f429e69aa0138f07933217a917f78a9d61450142053f380d23ed0af1c2bb`。PetaLinux/交叉工具链不包含在小型源码包中。

Paddle-Lite ARMv7 头文件与静态库单独封于 `build/deps/paddle_lite_armv7_build_dep.tar.gz`（SHA256 `858832912944fa9c2e91eada484c1bc127a8ba0a553aa9363e2ef91c872219c2`），解包后库 SHA256 为 `95a60689339c12665c6686f71f694b168fae42d68ee5554ad78898fe4cbbd7dd`。这是第三方/队内依赖；对外再分发前核对许可证和归属，随附 `build/deps/PADDLE_LITE_LICENSE.txt`。

```bash
cd /path/to/eagleeye_z035
tar -xzf build/deps/paddle_lite_armv7_build_dep.tar.gz -C build/deps
export PROJECT=/path/to/eagleeye_petalinux_2022_2
export PADDLE_LITE_DIR="$(pwd)/build/deps/paddle_lite"
bash src/ps_as_built/build_arm.sh
sha256sum src/ps_as_built/bin/paddle_person_live
```

本机从独立源码副本实测生成 SHA256 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，与稳定 SD 应用归档内二进制完全一致。编译后生成的 `bin/`、`logs/` 是输出，不属于源码快照。

注意力版使用相同工具链、依赖和 `src/ps/build_arm.sh` 编译；其 ARM 程序 SHA256 为 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`，已封装进 `board/attention/linux_camera_smooth_trial_v7_20261013.tar.gz`。应用归档 SHA256 为 `b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59`。在归档内应用根目录运行 `sha256sum -c SHA256SUMS` 可核验全部 54 个文件。旧 `src/ps_as_built/` 与新 `src/ps/` 必须分别构建，不要混用日志或源码。

## 部署、时序及未闭环项

原工程快照的 BOOT.BIN、image.ub、boot.scr 和应用归档保存在 `board/`。按恢复目录的 `board/README.md` 校验后使用**独立实验卡**，保留稳定介质。当前可核查的 BIT、BOOT 和应用组合见[版本表](../report/evidence/release_identity_zh.md)，干净构建产物的板测范围见[复现报告](../report/evidence/clean_reproduction_20261028_zh.md)。

原 Eyes routed report：WNS +0.505 ns，WHS +0.051 ns，已约束路径满足设定约束；Methodology 同时报告 `TIMING-18 Missing input or output delay` 16 项及其他时钟相关 Critical Warning。因此不能宣称摄像头/HDMI 全部板级 I/O 时序已经签核。需要基于实际 Z100/OV5640/MS7210 连接和器件时序补充输入/输出延迟、重新实现并上板校验，不能照搬 AX7020 参考工程的约束。

# 干净目录复现与证据边界

2026-10-28 已另从 GitHub 提交 `8588b389aacab349c8cae3132730d7ed1bac6669` 干净克隆并完成独立复现：Vivado 在布线中断后从检查点续跑生成 BIT/XSA；新 BIT 与既有上板 BIT 的配置数据一致，但整文件哈希不同，新 BIT 未实板验证。完整过程、工具依赖、原始日志及未闭环问题见 [本次干净复现记录](clean_reproduction_20261028_zh.md)。可执行 `python -B tools/verify_clean_rebuild_20261028.py` 验证本次归档及 BIT/XSA 的关联。

以下步骤用于重建与验证的独立工作目录，不能覆盖稳定 SD。本仓库包含原工程完整快照和 Git LFS 中的 Xilinx Linux 固定版本源码；已在本机独立目录完成 Eyes 源码综合、实现和 BIT 生成，并完成固定源码的 Linux 内核和模块构建。现场于 2026-10-28 回传的 BIT SHA256 与 2026-10-27 构建产物一致；启动串口覆盖 U-Boot 至 Linux 登录，后续应用日志已收录，现场已确认六项功能正常。启动串口本身不含 FSBL 或登录后应用输出，独立文件身份仍以实际记录为准，具体见 [上板证据](board_reproduced_20261028_zh.md)。

## 1. 获取与验证

FPGA 工程构建使用 **Xilinx Vivado 2022.2**。获取和核验需要 Git LFS、Python 3.10+；Linux 中完整 RTL 测试还需 NumPy 和 Icarus Verilog（本次为 10.3）。

```sh
git lfs install
git clone https://github.com/Zyl-stack/eagleeye-z035-contest.git
cd eagleeye-z035-contest
git lfs pull
python3 -B tools/verify_submission.py
python3 -B tools/verify_kernel_source.py
```

源码包约 240 MiB，若没有 Git LFS 或未拉取对象，源码路径可能只有 LFS 指针；核验脚本会拒绝。`verify_submission.py` 还会核对原始 Eyes 快照、各模块与 Linux 参考快照及清单。哈希不是签名或板端运行证明。

## 2. 恢复大小写并构建 Vivado 工程

从仓库根目录运行，输出目录必须不存在；在 Windows 上建议使用短 ASCII 路径：

```sh
python3 tools/restore_a_repro.py --output /tmp/eagleeye_a_new
vivado -mode batch -source build/build_full_vivado.tcl -tclargs /tmp/eagleeye_a_new/eagleeye_z035 /tmp/eagleeye_vivado_new 4
python3 tools/verify_vivado_outputs.py /tmp/eagleeye_vivado_new
```

先执行 `git lfs pull`。恢复工具将原始快照与补充材料合并并核验 275 项哈希；`build_full_vivado.tcl` 在独立输出目录进行 BD 校验、综合、实现、BIT/XSA 和资源/时序/CDC/DRC 报告。最后的校验脚本确认 XSA 内 BIT 与独立 BIT 一致，并输出 BIT/XSA/HWH SHA256。实际运行以本机 Vivado 2022.2 与许可证为准。上述完整入口已在本机从还原的工程快照实际完成 BD、综合、实现和 BIT/XSA；报告与哈希见 `build/full_rebuild_20261027/`。此前独立构建报告保存在 `build/independent_rebuild_20261027/`。BIT 生成不等于板端验证；缺失 I/O 延迟和 CDC Critical 项仍需检查。

## 3. 预处理 IP RTL 仿真

在上一步恢复的**独立快照目录**内执行：

```sh
python3 src/ip_repo/preproc/tools/generate.py --captured data/captured.rgb
cd src/ip_repo/preproc
python3 tools/run_verification.py
```

脚本生成 ROM 和五类输入/Golden，仿真核心、RGB 字节适配器、适配器加核心及 AXI-Lite。先确保 PATH 中为可用的 `iverilog`、`vvp` 和 Python/NumPy。生成目录 `vectors/`、`build/` 属于临时结果。

本次实际使用 Windows Python 3.13/NumPy 2.5.2 生成与比较，WSL Ubuntu 20.04 中 Icarus 10.3 执行 RTL，命令和输出在 `sim/preproc_iverilog_20261027.log`。五帧各 307200 个值逐位一致；采集帧的输入适配器加核心亦一致。此为仿真，不是板端 DMA/整机结果。

舵机 RTL 在独立目录的标准复现命令：

```sh
iverilog -g2012 -s tb_roi_servo_pwm -o /tmp/servo.vvp src/servo/roi_servo_pwm.v src/servo/tb_roi_servo_pwm.sv
vvp /tmp/servo.vvp
```

实际输出及工具路径见 `sim/servo_iverilog_20261027.log`；仿真通过不证明实物舵机限位、供电或机械安全。

## 4. Linux 源码和构建输入

`reference/linux_z035/source/linux_xlnx_19984dd.tar.gz` 是 Git LFS 保存的 Xilinx Linux 5.15.36 固定提交源码归档。完整上游源码、历史配置、实验补丁和模块符号的版本区别见 `reference/linux_z035/readme.md`。可在独立目录解压后检查：

```sh
mkdir -p /tmp/eagleeye_linux_src
tar -xzf reference/linux_z035/source/linux_xlnx_19984dd.tar.gz -C /tmp/eagleeye_linux_src
patch --dry-run -d /tmp/eagleeye_linux_src -p1 -i "$PWD/reference/linux_z035/experimental/kernel.patch"
```

该补丁仅用于 `eeiso22` 实验内核，不能施用于声称不变的稳定内核。实际构建还需匹配的 PetaLinux 2022.2 交叉工具链、内核配置、设备树/XSA、rootfs 和外部依赖；生成并核对 `kernel.release`、`.config`、`Module.symvers` 与镜像哈希。稳定配置启用 `CONFIG_LOCALVERSION_AUTO`，单纯解压源码并不保证得到原构建版本字符串；必须核验，不能靠版本名推断二进制一致。

实验配置强制模块签名，签名私钥不在仓库；新生成的密钥不证明与已发布模块或板端信任链匹配。已在本机独立目录用稳定配置完成上游内核 zImage 和 172 个模块构建；新 zImage 与历史镜像哈希不同，原因尚未确定。实验补丁只进行了源码核验；没有证明实验模块签名或板端启动。

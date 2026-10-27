# 干净目录复现与证据边界

以下步骤用于重建与验证的独立工作目录，不能证明现场板卡身份，也不能覆盖稳定 SD。本仓库上传了原工程完整快照和 Git LFS 中的 Xilinx Linux 固定版本源码；当前尚未在干净环境完成 Vivado 或 Linux 完整构建。

## 1. 获取与验证

需要 Git LFS、Python 3.10+；Linux 中完整 RTL 测试还需 NumPy 和 Icarus Verilog（本次为 10.3）。

```sh
git lfs install
git clone https://github.com/Zyl-stack/eagleeye-z035-contest.git
cd eagleeye-z035-contest
git lfs pull
python3 -B tools/verify_submission.py
python3 -B tools/verify_kernel_source.py
```

源码包约 240 MiB，若没有 Git LFS 或未拉取对象，源码路径可能只有 LFS 指针；核验脚本会拒绝。`verify_submission.py` 还会核对原始 Eyes 快照、A/B/C/Linux 参考快照及清单。哈希不是签名或板端运行证明。

## 2. 恢复原工程大小写

从仓库根目录：

```sh
mkdir -p /tmp/eagleeye_contest_work
python3 -m zipfile -e original/eagleeye_z035.zip /tmp/eagleeye_contest_work
cd /tmp/eagleeye_contest_work/eagleeye_z035
```

之后按该目录 `README.md`、`build/recreate_vivado.tcl` 和 `src/ps/build_arm.sh` 指引准备 Vivado 2022.2、ARM 交叉编译器、PetaLinux sysroot 和匹配的 Paddle-Lite 静态库。ZIP 保留 `BOOT.BIN` 和工程原有文件名；仓库小写展示副本可能不满足工程中大小写引用。请另选未使用的目录，避免覆盖。

独立 Vivado 启动此前遇到 `0xC0000135`，未进入脚本。本次重试 `vivado.bat -version` 退出非零、没有输出；尚未证明干净重建成功。原有 `build/*.rpt` 是历史结果。

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

实验配置强制模块签名，签名私钥不在仓库；新生成的密钥不证明与已发布模块或板端信任链匹配。本次只核验源码归档和三个实验补丁文件，**没有进行完整内核构建、模块签名或实板启动**。

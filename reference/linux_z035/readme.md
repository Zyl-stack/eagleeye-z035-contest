# Z035 Linux 构建资料与版本边界

本目录提供固定版本 Linux 源码、**配置和补丁**，用于在独立环境重建系统；板端镜像按匹配的启动资料管理。

## 稳定系统资料

`stable/` 包含 A 交接的 PetaLinux 2022.2 内核配置、`Module.symvers`、`kernel.release`、设备树片段及配置片段。`kernel.release` 为 `5.15.36-xilinx-v2022.2`。该 `.config` 的 SHA256 为 `6f1063a6e7a38c1f0d264db258a1987c9e46c7ec70c9a3bfcfb527df5aa005c4`；它与后来实验内核所记录的 baseline 配置一致。

已有构建记录使用 [Xilinx linux-xlnx commit 19984dd147fa7fbb7cb14b17400263ad0925c189](https://github.com/Xilinx/linux-xlnx/tree/19984dd147fa7fbb7cb14b17400263ad0925c189)，其 Makefile 为 Linux 5.15.36。完整上游固定提交源码保存在 `source/linux_xlnx_19984dd.tar.gz`，通过 Git LFS 分发，SHA256 为 `139cbe4ad3b6e21f243f21e0fd272640b295d50737d666c055a54784f0e954a9`。使用 `git lfs pull` 后执行 `python3 tools/verify_kernel_source.py` 校验。归档内容与基线提交重新导出的结果一致（78975 项）；它是通用上游基线，不直接等于现场完整系统。重建还需要匹配的 PetaLinux 2022.2、工具链、设备树/XSA 等依赖。重建后通过文件哈希另行核对与现场 image.ub 的二进制关系。

## 独立实验内核

`experimental/` 是 `5.15.36-xilinx-v2022.2-eeiso22` 隔离实验版本的配置、补丁和模块符号。它与稳定内核独立构建，模块按各自版本和 ABI 配套使用。`historical_build_kernel.sh` 保留原始构建命令，包含本机绝对路径，只用于审阅，不能直接运行以免修改既有工程。签名私钥没有收录；本次未重新构建或验证签名。

本次将补丁应用到独立提取的三个相关文件，其结果与本机实验源码相应文件逐字节一致；见 `sim/kernel_archive_patch_20261027.log`。本项核对三个相关文件的补丁结果，全树、完整构建及板端验证按对应工程另行执行。此前有 eeiso22 内核构建与 QEMU 软件测试记录，但这不等于本仓库完成独立复现或实际板端验证。具体配置与补丁应对照所选源码、工具链、DT、模块 ABI 和镜像逐项核验。不得覆盖稳定 SD 卡。

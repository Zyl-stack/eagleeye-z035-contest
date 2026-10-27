# Z035 Linux 构建资料与版本边界

本目录是从本机交接资料整理的**配置和补丁**，不是已经验证可直接刷写的系统镜像。

## 稳定系统资料

`stable/` 包含 A 交接的 PetaLinux 2022.2 内核配置、`Module.symvers`、`kernel.release`、设备树片段及配置片段。`kernel.release` 为 `5.15.36-xilinx-v2022.2`。该 `.config` 的 SHA256 为 `6f1063a6e7a38c1f0d264db258a1987c9e46c7ec70c9a3bfcfb527df5aa005c4`；它与后来实验内核所记录的 baseline 配置一致。

已有构建记录使用 [Xilinx linux-xlnx commit 19984dd147fa7fbb7cb14b17400263ad0925c189](https://github.com/Xilinx/linux-xlnx/tree/19984dd147fa7fbb7cb14b17400263ad0925c189)，其 Makefile 为 Linux 5.15.36。本仓库没有上传完整上游内核源码；重建需独立获取该固定提交及匹配的 PetaLinux 2022.2、工具链、设备树/XSA 等依赖。版本号和配置相同不能单独证明重建结果与现场 image.ub 字节一致。

## 独立实验内核

`experimental/` 是 `5.15.36-xilinx-v2022.2-eeiso22` 隔离实验版本的配置、补丁和模块符号。它与稳定内核属于不同构建，不能混用模块或当作稳定版本。`historical_build_kernel.sh` 保留原始构建命令，包含本机绝对路径，只用于审阅，不能直接运行以免修改既有工程。签名私钥没有收录；本次未重新构建或验证签名。

此前有 eeiso22 内核构建与 QEMU 软件测试记录，但这不等于本仓库完成独立复现或实际板端验证。具体配置与补丁应对照所选源码、工具链、DT、模块 ABI 和镜像逐项核验。不得覆盖稳定 SD 卡。

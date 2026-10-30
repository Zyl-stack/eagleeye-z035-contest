---
name: 04-petalinux-xsa-rebuild
description: 从已确认的 Zynq 硬件导出重建 PetaLinux 发布物，先检查宿主、工具版本与 XSA/bitstream 身份，再保留完整构建和启动证据。
---

# 硬件导出到系统镜像的可复现构建

## 适用范围

适用于已有 XSA/HDF、希望生成可追溯 BOOT.BIN、内核/rootfs 镜像和 SD 包的任务。先查目标 PetaLinux 版本的官方宿主与导入要求，不能把本项目 2022.2/Ubuntu 20.04 示例推广为所有版本要求。

## 工作流

1. 清点硬件导出、外部 bitstream、板级 DDR/MIO 配置、设备树变更、内核配置与 rootfs 应用。建立输入哈希，识别器件和导出工具版本。
2. 用 `scripts/verify_xsa_bit.py` 比较 XSA 内嵌 `.bit` 与外部 `.bit`。存在多份内嵌 bitstream 时必须显式选择成员；相同只证明这两个文件一致，不证明 PS/板卡配置正确。
3. 检查 `petalinux-create --version`、settings 环境、宿主发行版、64 位、可用磁盘/RAM 和文件系统。构建放入适合该工具的 Linux 原生文件系统；Windows 共享目录可作运输目录。环境不符时记录预检失败，先解决具体依赖。
4. 用目标版本支持的方式创建 Zynq 工程并导入硬件。为应用所需缓冲保留内存，核验内核/用户态谁拥有每个外设。若用户态直接管理 VDMA，不能让已有内核驱动同时启动同一硬件。
5. 保存完整 stdout/stderr 和实际退出码。管道经 `tee` 时启用 `pipefail` 或记录原构建进程退出码，避免把 tee 成功误当构建成功。
6. 构建失败按最早有效错误定位环境、下载、recipe、设备树或硬件导入阶段，不为绕过错误随意替换 XSA、删设备树节点或换旧镜像。
7. 从同一次构建收集 bootloader/FSBL、bitstream、内核、DTB、rootfs 与打包命令；按实际启动格式列清 BOOT.BIN/image.ub 的组成。生成校验清单。
8. 在干净目录或另一台合格宿主复建，最后由目标板验证启动、驱动/保留内存、相机和应用。构建成功与真实启动分别记录。

## 使用方法

```sh
python scripts/verify_xsa_bit.py hardware.xsa hardware.bit
python scripts/verify_xsa_bit.py hardware.xsa hardware.bit --member path/inside/system.bit
```

其余 PetaLinux 命令查目标版本及现有构建脚本。本项目参考入口为 `team_handoff_20261005_v1/integration/linux_ov5640_picodet/scripts/`，不随本技能重复打包。

## 失效条件

启动镜像来自不同硬件版本、构建日志中断、仅创建工程未 build、只保留生成工程而缺源码/recipes、只改扩展名把 XSA 当 HDF，均不满足复现。工具存在、器件安装和授权有效也需要分别检查。

## 已验证效果与失败来源

[E04](../evidence.md#e04镜像重建前先识别工具代际)：XSA 为 2022.2，而原课程环境为 2017.4；本机还曾打开不同 Ubuntu 版本且无 PetaLinux。既有记录据此明确阻塞阶段，未伪造镜像产物。本技能抽取的是预检与绑定方法，不声称这些历史镜像已构建成功。

## 可直接使用的提示词

> 请根据本次 XSA 和板卡信息做环境预检，再用匹配的 PetaLinux 流程重建。先核对内嵌/外部 bitstream、工具代际、宿主与文件系统，保留完整日志和退出码。交付真实生成的镜像、设备树、配置与输入/输出哈希；失败时指出准确阶段和最小修复，不以旧镜像补位。

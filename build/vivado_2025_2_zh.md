# Vivado 2025.2 实验性迁移入口

仓库原始 Eyes 工程使用 Vivado 2022.2；[`build_vivado_2025_2.tcl`](build_vivado_2025_2.tcl) 是为 Vivado 2025.2 准备的独立迁移脚本。**验证状态：已在 2022.2 检查脚本流程及版本拒绝路径，2025.2 的综合、时序、BIT 和板端验证待目标环境执行。**

## 前提

- 安装 Vivado 2025.2，且安装包含 `xc7z035ffg900-2` 器件和工程所需 IP；使用有效许可证。
- 安装 Git LFS 和 Python 3.10+，确保已完整拉取原始快照与补充材料归档。原工程的厂商自定义 IP 随快照恢复；具体许可及新版兼容性须核对。
- 输入必须是 `tools/restore_a_repro.py` 恢复后的 `eagleeye_z035` 目录，不能用仓库中转为小写的展开副本。
- 使用短的、无空格的 ASCII 路径；脚本只创建**尚不存在**的独立输出目录，不覆盖源工程或稳定 SD。

## 使用方法

在仓库根目录，从已配置 Vivado 2025.2 环境的终端执行。以下路径为示例，先确保两个输出路径均不存在；Windows 下可以使用正斜杠。

```sh
git lfs pull
python -B tools/verify_submission.py
python -B tools/restore_a_repro.py --output D:/EE25_SRC
vivado -mode batch -source build/build_vivado_2025_2.tcl -tclargs D:/EE25_SRC/eagleeye_z035 D:/EE25_PROJECT project
```

`project` 模式复制源文件，创建 `D:/EE25_PROJECT/vivado_work/eagleeye_z035.xpr`，尝试升级块设计中的目录 IP、校验 BD、检查锁定 IP 并生成输出产品；它不执行综合或生成 BIT。升级前后 IP 状态和升级日志保存在 `D:/EE25_PROJECT/reports/`，其余信息见 Vivado 日志。Vivado 启动时可能在当前工作目录产生日志，建议在独立工作目录运行并给 `-log`、`-journal` 指定路径。

检查工程及报告后，如需在**另一个不存在的输出目录**尝试综合、布局布线并生成 BIT/XSA：

```sh
vivado -mode batch -source build/build_vivado_2025_2.tcl -tclargs D:/EE25_SRC/eagleeye_z035 D:/EE25_BIT bit 4
```

最后的 `4` 是并行任务数，可为 1～16。`bit` 模式完成后会写出资源、时序、DRC、CDC 和 Methodology 报告，以及 `eagleeye_z035.xsa`；BIT 位于 `vivado_work/eagleeye_z035.runs/impl_1/z100_top.bit`。任何一步报错即停止，不修改 2022.2 原工程。工程升级可能改变 IP 行为、接口或输出；必须重新核查地址、时钟、复位、约束、升级报告及软件兼容性，并通过所需仿真、时序和实板验证，升级后的测试结果按新版本独立记录。

## 已执行的本机检查

2026-10-30 使用 **Vivado 2022.2**，对脚本临时副本仅替换版本检查，运行 `project` 模式：BD 打开和校验、IP 输出产品生成及工程创建成功；过程有原有的 AXI 宽度等警告。该临时副本完成 2022.2 的脚本流程检查，2025.2 运行、综合、BIT 和上板按目标版本单独验证。原始日志见 [2022.2 流程检查](vivado_2025_2_checks/smoke_2022.log)。

对未修改的 2025.2 脚本，在本机 2022.2 运行时按预期退出并拒绝继续，见 [版本检查日志](vivado_2025_2_checks/version_guard_2022.log)。完整脚本使用 `upgrade_ip` 处理目录 IP，不把模块引用当作 IP，也不使用 `get_ips -all`。

参考 AMD 官方文档：[2025.2 IP 升级命令](https://docs.amd.com/r/2025.2-English/ug835-vivado-tcl-commands/upgrade_ip)、[2025.2 块设计升级流程](https://docs.amd.com/r/2025.2-English/ug994-vivado-ip-subsystems/Upgrading-a-Block-Design-in-Project-Mode)。实际出现锁定或升级失败时，应保留升级与 IP 状态报告，而不是强制解锁或忽略错误。

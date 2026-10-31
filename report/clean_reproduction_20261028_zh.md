# GitHub 干净克隆复现记录（2026-10-28）

复现对象：GitHub `main` 提交 `8588b389aacab349c8cae3132730d7ed1bac6669`，独立克隆和独立构建目录；本页的复现对象固定为该源提交。本次没有操作开发板或稳定 SD。原始日志、实际历史命令、报告和 BIT/XSA 归档在 [`build/clean_rebuild_20261028`](../build/clean_rebuild_20261028/)，文件哈希见 [sha256sums.txt](../build/clean_rebuild_20261028/sha256sums.txt)。运行 `python -B tools/verify_clean_rebuild_20261028.py` 可核验归档及 BIT/XSA 对应关系。历史命令保留本机路径以支持复核，在其他电脑执行时按[复现指南](reproduction_zh.md)设置独立目录。

## 获取、源码与功能验证

Git LFS 获取后 `tools/verify_submission.py` 通过（原始文件 226、仓库文件 991）；Linux 固定源码归档哈希核验通过，归档包含 78975 个条目；工程快照恢复核验通过，共 275 项。这里的文件数量对应上述源提交，后续仓库添加材料后会变化。

Skill 单元测试 5 项通过；PicoDet 注意力主机测试及模拟无目标保持测试通过。上述主机测试没有单独留存本次原始日志，仅记录当次执行结果。

RTL 测试采用 Windows Python 3.13 / NumPy 2.5.2 生成和比较，WSL Ubuntu 20.04 中 Icarus Verilog 10.3 运行仿真；实际命令和输出见 [RTL 日志](../build/clean_rebuild_20261028/logs/rtl_simulation.log)及[历史运行器](../build/clean_rebuild_20261028/commands/rtl_runner_historical.txt)。五类输入各 307200 项与参考逐位一致，RGB 适配器、采集帧适配器与核心、AXI-Lite 和舵机 RTL 测试通过。本次是分环境执行，并未原样执行仓库 `run_verification.py`；标准单环境执行方式见[复现说明](reproduction_zh.md)。该阶段完成 RTL 离线验证，板端结果按对应现场记录列示。

## 完整 Vivado 工程

Vivado 2022.2 对恢复后的工程快照完成 BD 校验、26 个 IP 子综合及顶层综合、布局和物理优化。首次运行在布线期间中断；保留[首次日志](../build/clean_rebuild_20261028/logs/vivado_initial.log)和[中断前实现日志](../build/clean_rebuild_20261028/logs/vivado_impl_before_resume.log)。确认已完成物理优化检查点后，仅重置布线步骤并从检查点继续，生成 BIT 和 XSA；[续跑命令](../build/clean_rebuild_20261028/commands/vivado_resume_historical.txt)和[日志](../build/clean_rebuild_20261028/logs/vivado_resume.log)保留原貌。本次以干净输入完成构建，过程包含一次布线中断及检查点续跑，相关日志完整保留。

| 产物或结果 | 本次结果 |
|---|---|
| [BIT](../build/clean_rebuild_20261028/artifacts/z100_top.bit) SHA256 | `7dd0f6ac95d9b8e5f3c62db6707ea8ddad0130bbd23ec0c9cb17faaf413ada70` |
| [XSA](../build/clean_rebuild_20261028/artifacts/eagleeye_z035.xsa) SHA256 | `7c168e50d6a6da58753b0ce1d2c32434f7b34e5474110be2b130cbfd63ed8f41` |
| XSA 内 `system.hwh` SHA256 | `f2e91d38a119b4a0ab7b07efadd9cd286d1d6bcdae08e73e9de221073765c03f` |
| LUT / FF / BRAM36 / DSP | 24986 / 28246 / 303（共500）/ 49（共900） |
| 已约束路径 WNS / WHS | +0.608 ns / +0.007 ns |
| DRC | 65 Warning、4 Advisory |
| Methodology / CDC | 8 Critical Warning / 有 Critical 分类 |
| 外部时序 | 12 个输入、33 个输出缺少延迟约束 |

原始[资源](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[DRC](../build/clean_rebuild_20261028/reports/drc.rpt)、[CDC](../build/clean_rebuild_20261028/reports/cdc.rpt)、[Methodology](../build/clean_rebuild_20261028/reports/methodology.rpt)报告供逐项核查。首次日志亦有 XDC 端口未匹配和管脚告警；必须结合设计与板级资料判断。WNS 描述已约束路径，外部接口和 CDC 的专项签核状态见[告警说明](warning_review_zh.md)。

本次 BIT 与现场此前上板反馈使用的 [`b286f480...`](../board/reproduced_20261028/z100_top.bit) 文件长度相同，仅 7 字节不同，均位于生成日期/时间头部；从同步字 `aa995566` 起的配置数据逐字节一致，SHA256 均为 `02c426b197721f9b367e33d9047dfe3e7c84191c90227a18f45411bd94526076`。新旧 XSA 中 HWH 只有 TIMESTAMP 不同，XSA 内 BIT 均与对应独立 BIT 一致。两文件具有相同配置数据及不同整文件哈希；本次新 BIT 的独立板测为后续验证项。此前现场反馈和串口范围见[既有板端记录](board_reproduced_20261028_zh.md)。

## ARM 程序、QEMU 与 Linux

使用本机外部 PetaLinux 2022.2 ARM 交叉工具链/sysroot、工程快照内 Paddle-Lite 静态库，独立编译 PicoDet 注意力候选，ELF SHA256 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；独立编译原 GridPolicy 稳定应用 SHA256 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，分别与已有候选和稳定记录一致。见[候选构建日志](../build/clean_rebuild_20261028/logs/app_build.log)和[稳定应用构建日志](../build/clean_rebuild_20261028/logs/stable_app_build.log)。链接警告指出静态应用调用 `dlopen` 时仍可能需要匹配的 glibc 动态库，部署仍需核对相应 glibc 运行依赖。

QEMU 固定 RGB、单线程 CPU 预处理用例产生 7 个人体框及 `C_ATTENTION` 请求；无目标用例也执行，见[检测](../build/clean_rebuild_20261028/logs/qemu_person.log)和[无目标](../build/clean_rebuild_20261028/logs/qemu_empty.log)日志。`hardware_commit=not_attempted`；该用例覆盖 QEMU 离线推理及软件请求生成；板端速度、摄像头、HDMI、舵机和 FPGA 行为按实际硬件测试记录。

仓库固定 Linux 源码与稳定配置在独立目录通过 ARM 交叉构建，`kernel.release=5.15.36-xilinx-v2022.2`，生成 zImage 和 172 个模块；配置 SHA256 `6f1063a6e7a38c1f0d264db258a1987c9e46c7ec70c9a3bfcfb527df5aa005c4`，zImage SHA256 `dd442f1a509f453a7931ce47ebf1cdb00d65ce0235275bdb354dbbf8ccaa7bf4`，`Module.symvers` SHA256 `23ff35be01e406a61776b708685a96aa5922309029f7e12eada2d5fdebfb82e0`。见[内核日志](../build/clean_rebuild_20261028/logs/kernel_build.log)和[历史构建命令](../build/clean_rebuild_20261028/commands/kernel_runner_historical.txt)。与实板 `image.ub` 的二进制对应、模块兼容和启动状态待核验；编译依赖本机外部交叉工具链。

## 原始记录与验证范围

日志为保留原始字节的本机输出；部分包含 WSL 启动信息和 NUL 字节，文本查看器可能显示异常，查阅时使用支持原始字节的工具，保持归档内容原样。本次新 BIT 的板端记录、FSBL 原始日志、经帧新鲜度验证的检测更新 FPS，以及外部 I/O 时序和 CDC 告警的闭环依据仍缺。后续收到 Attention 应用日志与现场反馈，具体范围和 VDMA 警告见 [应用证据](board_app_evidence_20261028_zh.md)；另一次[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)逐位通过，其独立 BIT 哈希与本次干净复现 BIT 的关系待核对；各次现场反馈及文件记录按对应版本保留。

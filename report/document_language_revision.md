# EagleEye 文档语言修订记录

历史修订日期：2026-10-30。基准提交：`b1190d173e6889a734129c9a834a16692a945f84`。原轮次调整了 50 份 Markdown、259 处文本块；完整历史对照保留在本文件的 Git 历史中。

2026-10-31 按最终 PicoDet 提交范围整理，本页保留 48 份现存文档的 240 处相关历史对照，其他路线材料已从当前提交移出并在本地归档。Before/After 引用当时准确文字；后续修改以各文档当前版本为准。

## 修改概要

本轮语言调整将功能和验证成果放在前面，把重复说明集中到测试条件、版本说明和工程注意事项。计算结果、原始日志及源码在该轮文档修改中保持原样。

| 文档 | 保留的历史对照数 |
|---|---:|
| [build/vivado_2025_2_zh.md](../build/vivado_2025_2_zh.md) | 3 |
| [readme.md](../readme.md) | 8 |
| [reference/linux_z035/readme.md](../reference/linux_z035/readme.md) | 4 |
| [reference/readme.md](../reference/readme.md) | 3 |
| [report/a_repro_checklist_20261027.md](../report/a_repro_checklist_20261027.md) | 1 |
| [report/a_repro_status_20261027.md](../report/a_repro_status_20261027.md) | 1 |
| [report/a_supplement_results_20261027.md](../report/a_supplement_results_20261027.md) | 1 |
| [report/ai_collaboration_record.md](../report/ai_collaboration_record.md) | 18 |
| [report/board_app_evidence_20261028_zh.md](../report/board_app_evidence_20261028_zh.md) | 10 |
| [report/board_evidence_20261027.md](../report/board_evidence_20261027.md) | 5 |
| [report/board_measurements.md](../report/board_measurements.md) | 5 |
| [report/board_reproduced_20261028_zh.md](../report/board_reproduced_20261028_zh.md) | 5 |
| [report/clean_reproduction_20261028_zh.md](../report/clean_reproduction_20261028_zh.md) | 11 |
| [report/contest_evidence_index_zh.md](../report/contest_evidence_index_zh.md) | 9 |
| [report/design_report_draft.md](../report/design_report_draft.md) | 1 |
| [report/design_report_zh.md](../report/design_report_zh.md) | 38 |
| [report/field_evidence_needed_zh.md](../report/field_evidence_needed_zh.md) | 5 |
| [report/hardware_interfaces_zh.md](../report/hardware_interfaces_zh.md) | 4 |
| [report/preproc_ab_20261028_zh.md](../report/preproc_ab_20261028_zh.md) | 6 |
| [report/preproc_board_golden_20261029_zh.md](../report/preproc_board_golden_20261029_zh.md) | 5 |
| [report/readme.md](../report/readme.md) | 3 |
| [report/release_identity_zh.md](../report/release_identity_zh.md) | 4 |
| [report/reproduction_zh.md](../report/reproduction_zh.md) | 7 |
| [report/submission_audit.md](../report/submission_audit.md) | 2 |
| [report/submission_check_20261028_zh.md](../report/submission_check_20261028_zh.md) | 4 |
| [report/warning_review_zh.md](../report/warning_review_zh.md) | 4 |
| [sim/lanczos_legacy_readme.md](../sim/lanczos_legacy_readme.md) | 3 |
| [sim/preproc_captured_dma_20261027/readme.md](../sim/preproc_captured_dma_20261027/readme.md) | 1 |
| [sim/verification_zh.md](../sim/verification_zh.md) | 11 |
| [skill/01-axi-video-regression/skill.md](../skill/01-axi-video-regression/skill.md) | 1 |
| [skill/02-tensor-preprocess-golden/skill.md](../skill/02-tensor-preprocess-golden/skill.md) | 1 |
| [skill/03-armv7-inference-deploy/skill.md](../skill/03-armv7-inference-deploy/skill.md) | 2 |
| [skill/04-petalinux-xsa-rebuild/skill.md](../skill/04-petalinux-xsa-rebuild/skill.md) | 1 |
| [skill/05-vdma-frame-snapshot/skill.md](../skill/05-vdma-frame-snapshot/skill.md) | 2 |
| [skill/06-detection-stream-contract/skill.md](../skill/06-detection-stream-contract/skill.md) | 1 |
| [skill/08-roi-ablation-evaluation/skill.md](../skill/08-roi-ablation-evaluation/skill.md) | 1 |
| [skill/09-roi-transaction-integration/skill.md](../skill/09-roi-transaction-integration/skill.md) | 1 |
| [skill/10-hls-cross-platform-port/skill.md](../skill/10-hls-cross-platform-port/skill.md) | 1 |
| [skill/11-reproducible-evidence-release/skill.md](../skill/11-reproducible-evidence-release/skill.md) | 1 |
| [skill/evidence.md](../skill/evidence.md) | 10 |
| [skill/experience_notes.md](../skill/experience_notes.md) | 7 |
| [skill/preproc-fp32-golden/readme.md](../skill/preproc-fp32-golden/readme.md) | 7 |
| [skill/pynq-overlay-dma/readme.md](../skill/pynq-overlay-dma/readme.md) | 3 |
| [skill/readme.md](../skill/readme.md) | 2 |
| [skill/roi-review-control/readme.md](../skill/roi-review-control/readme.md) | 7 |
| [skill/validation.md](../skill/validation.md) | 4 |
| [src/attention_reference/readme.md](../src/attention_reference/readme.md) | 2 |
| [src/picodet_c_candidate/readme.md](../src/picodet_c_candidate/readme.md) | 4 |

## 逐项 Before/After

### build/vivado_2025_2_zh.md

#### 原修改 001

原行号：3–3；当时修改后行号：3–3。

位置：Vivado 2025.2 实验性迁移入口

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
仓库原始 Eyes 工程使用 Vivado 2022.2；[`build_vivado_2025_2.tcl`](build_vivado_2025_2.tcl) 是为 Vivado 2025.2 准备的独立迁移脚本。**当前机器只有 2022.2，尚未在 2025.2 运行本脚本；不能将其视为 2025.2 综合、时序、BIT 或板端验证通过。**
~~~~

**After**

~~~~text
仓库原始 Eyes 工程使用 Vivado 2022.2；[`build_vivado_2025_2.tcl`](build_vivado_2025_2.tcl) 是为 Vivado 2025.2 准备的独立迁移脚本。**验证状态：已在 2022.2 检查脚本流程及版本拒绝路径，2025.2 的综合、时序、BIT 和板端验证待目标环境执行。**
~~~~

#### 原修改 002

原行号：31–31；当时修改后行号：31–31。

位置：Vivado 2025.2 实验性迁移入口 / 使用方法

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
最后的 `4` 是并行任务数，可为 1～16。`bit` 模式完成后会写出资源、时序、DRC、CDC 和 Methodology 报告，以及 `eagleeye_z035.xsa`；BIT 位于 `vivado_work/eagleeye_z035.runs/impl_1/z100_top.bit`。任何一步报错即停止，不修改 2022.2 原工程。工程升级可能改变 IP 行为、接口或输出；必须重新核查地址、时钟、复位、约束、升级报告及软件兼容性，并通过所需仿真、时序和实板验证，不能直接套用旧测试结论。
~~~~

**After**

~~~~text
最后的 `4` 是并行任务数，可为 1～16。`bit` 模式完成后会写出资源、时序、DRC、CDC 和 Methodology 报告，以及 `eagleeye_z035.xsa`；BIT 位于 `vivado_work/eagleeye_z035.runs/impl_1/z100_top.bit`。任何一步报错即停止，不修改 2022.2 原工程。工程升级可能改变 IP 行为、接口或输出；必须重新核查地址、时钟、复位、约束、升级报告及软件兼容性，并通过所需仿真、时序和实板验证，升级后的测试结果按新版本独立记录。
~~~~

#### 原修改 003

原行号：35–35；当时修改后行号：35–35。

位置：Vivado 2025.2 实验性迁移入口 / 已执行的本机检查

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
2026-10-30 使用 **Vivado 2022.2**，对脚本临时副本仅替换版本检查，运行 `project` 模式：BD 打开和校验、IP 输出产品生成及工程创建成功；过程有原有的 AXI 宽度等警告。该临时副本仅用于检查脚本流程，不是 2025.2 版本验证，也没有进行综合、BIT 或上板。原始日志见 [2022.2 流程检查](vivado_2025_2_checks/smoke_2022.log)。
~~~~

**After**

~~~~text
2026-10-30 使用 **Vivado 2022.2**，对脚本临时副本仅替换版本检查，运行 `project` 模式：BD 打开和校验、IP 输出产品生成及工程创建成功；过程有原有的 AXI 宽度等警告。该临时副本完成 2022.2 的脚本流程检查，2025.2 运行、综合、BIT 和上板按目标版本单独验证。原始日志见 [2022.2 流程检查](vivado_2025_2_checks/smoke_2022.log)。
~~~~

### readme.md

#### 原修改 004

原行号：7–7；当时修改后行号：7–7。

位置：EagleEye：仿生视觉转播系统

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本项目使用 **Vivado 2022.2** 构建 FPGA 工程；人体检测使用 **PicoDet-XS 与 Paddle-Lite**，模型推理运行在 ARM 端。局部放大是对已有像素的重采样，不代表恢复了原图中不存在的细节；项目用于辅助观察，不是自动裁判系统。
~~~~

**After**

~~~~text
本项目使用 **Vivado 2022.2** 构建 FPGA 工程，采用 **PicoDet-XS 与 Paddle-Lite** 在 ARM 端完成人体检测，并在 PL 侧实现视频处理与模型输入预处理。
~~~~

#### 原修改 006

原行号：31–31；当时修改后行号：31–31。

位置：EagleEye：仿生视觉转播系统 / 系统功能

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
视频显示、检测推理和 ROI 更新具有不同节奏：显示持续更新，并不意味着每一帧都产生了新的检测结果。现有临时目标编号用于相邻检测结果关联，不代表跨遮挡的稳定跟踪。
~~~~

**After**

~~~~text
系统将视频显示、检测推理和 ROI 更新异步组织，使画面展示与智能感知按各自节奏运行。
~~~~

#### 原修改 007

原行号：41–41；当时修改后行号：41–41。

位置：EagleEye：仿生视觉转播系统 / 验证与实现结果

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| 预处理计时 | 已有 CPU 与 FPGA 两组板端记录；两组使用不同缩放算法和不同实时帧，不能直接作为同算法加速比或精度对比。[测试条件与结果](report/preproc_ab_20261028_zh.md) |
~~~~

**After**

~~~~text
| 预处理计时 | CPU 双三次与 FPGA 双线性两种预处理配置均已完成板端计时，输入条件、算法及统计口径见对应报告。[测试条件与结果](report/preproc_ab_20261028_zh.md) |
~~~~

#### 原修改 008

原行号：43–43；当时修改后行号：43–43。

位置：EagleEye：仿生视觉转播系统 / 验证与实现结果

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
技术文档记录的阶段联调观察为检测更新约 **2 FPS**、视频显示约 **30 FPS**；两者是不同指标。现有[应用日志](report/board_app_evidence_20261028_zh.md)可重算检测路径耗时，视频显示帧率未见独立计时记录，不将显示帧率当作检测速度。
~~~~

**After**

~~~~text
阶段联调观察中，检测更新约 **2 FPS**、视频显示约 **30 FPS**，分别对应智能感知和视频展示路径。[应用日志](report/board_app_evidence_20261028_zh.md)提供检测路径的计时记录；30 FPS 为阶段观察值，独立显示帧率计时留待后续测量。
~~~~

#### 原修改 009

原行号：45–45；当时修改后行号：45–47。

位置：验证与实现结果 / 测试条件与工程说明

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
固定输入 Golden 使用独立测试程序，该次测试的独立 BIT 哈希未核实；它不能直接证明其他版本或整网精度。I/O 时序、CDC 和 VDMA 告警的现有记录及限制见 [告警说明](report/warning_review_zh.md)。独立 V2/R3 卷积实验不等同于当前 Eyes 系统已实现 FPGA 卷积卸载。
~~~~

**After**

~~~~text
### 测试条件与工程说明

固定输入 Golden 使用独立测试程序，其 BOOT 身份已记录，独立 BIT 哈希及与视频测试版本的对应关系待核对。CPU/FPGA 计时比较双三次与双线性两种配置，使用先后采集的实时帧。PicoDet 推理保持在 ARM 端，独立 V2/R3 卷积实验作为历史研究资料保存。I/O 时序、CDC 和 VDMA 的处理状态集中见 [工程告警说明](report/warning_review_zh.md)。
~~~~

#### 原修改 010

原行号：76–76；当时修改后行号：78–78。

位置：EagleEye：仿生视觉转播系统 / 获取与复现

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
仓库中展开的文件名采用小写；原工程脚本和约束可能依赖原始大小写。请先按 [复现指南](report/reproduction_zh.md) 使用 `tools/restore_a_repro.py` 将原快照恢复到**新的独立目录**，再运行 `build/build_full_vivado.tcl`。不要直接使用小写副本重建或覆盖稳定 SD。
~~~~

**After**

~~~~text
仓库中展开的文件名采用小写。为保留构建所需的原始大小写，请按 [复现指南](report/reproduction_zh.md) 使用 `tools/restore_a_repro.py` 将快照恢复到**新的独立目录**，再运行 `build/build_full_vivado.tcl`。构建和上板试验均使用独立工作区及实验介质，保留稳定 SD。
~~~~

#### 原修改 011

原行号：78–78；当时修改后行号：80–80。

位置：EagleEye：仿生视觉转播系统 / 获取与复现

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
使用 Vivado 2025.2 的人员可参考 [实验性迁移脚本及说明](build/vivado_2025_2_zh.md)；该脚本尚未在 2025.2 实际验证，不替代已记录的 2022.2 构建结果。
~~~~

**After**

~~~~text
使用 Vivado 2025.2 可参考 [实验性迁移脚本及说明](build/vivado_2025_2_zh.md)。已记录的完整构建基于 2022.2；2025.2 脚本的目标版本运行验证为后续迁移步骤。
~~~~

#### 原修改 012

原行号：80–80；当时修改后行号：82–82。

位置：EagleEye：仿生视觉转播系统 / 获取与复现

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
已记录的干净克隆构建曾在布线阶段中断，随后从检查点续跑生成 BIT/XSA；具体命令、日志和验证范围见 [干净复现记录](report/clean_reproduction_20261028_zh.md)。文件哈希核验、仿真、上板记录分别证明不同内容。
~~~~

**After**

~~~~text
已完成从干净克隆恢复工程、综合与实现；布线中断后从检查点续跑生成 BIT/XSA。具体命令、原始日志及验证范围见 [干净复现记录](report/clean_reproduction_20261028_zh.md)。
~~~~

### reference/linux_z035/readme.md

#### 原修改 014

原行号：3–3；当时修改后行号：3–3。

位置：Z035 Linux 构建资料与版本边界

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本目录是从本机交接资料整理的**配置和补丁**，不是已经验证可直接刷写的系统镜像。
~~~~

**After**

~~~~text
本目录提供固定版本 Linux 源码、**配置和补丁**，用于在独立环境重建系统；板端镜像按匹配的启动资料管理。
~~~~

#### 原修改 015

原行号：9–9；当时修改后行号：9–9。

位置：Z035 Linux 构建资料与版本边界 / 稳定系统资料

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
已有构建记录使用 [Xilinx linux-xlnx commit 19984dd147fa7fbb7cb14b17400263ad0925c189](https://github.com/Xilinx/linux-xlnx/tree/19984dd147fa7fbb7cb14b17400263ad0925c189)，其 Makefile 为 Linux 5.15.36。完整上游固定提交源码保存在 `source/linux_xlnx_19984dd.tar.gz`，通过 Git LFS 分发，SHA256 为 `139cbe4ad3b6e21f243f21e0fd272640b295d50737d666c055a54784f0e954a9`。使用 `git lfs pull` 后执行 `python3 tools/verify_kernel_source.py` 校验。归档内容与基线提交重新导出的结果一致（78975 项）；它是通用上游基线，不直接等于现场完整系统。重建还需要匹配的 PetaLinux 2022.2、工具链、设备树/XSA 等依赖。版本号和配置相同不能单独证明重建结果与现场 image.ub 字节一致。
~~~~

**After**

~~~~text
已有构建记录使用 [Xilinx linux-xlnx commit 19984dd147fa7fbb7cb14b17400263ad0925c189](https://github.com/Xilinx/linux-xlnx/tree/19984dd147fa7fbb7cb14b17400263ad0925c189)，其 Makefile 为 Linux 5.15.36。完整上游固定提交源码保存在 `source/linux_xlnx_19984dd.tar.gz`，通过 Git LFS 分发，SHA256 为 `139cbe4ad3b6e21f243f21e0fd272640b295d50737d666c055a54784f0e954a9`。使用 `git lfs pull` 后执行 `python3 tools/verify_kernel_source.py` 校验。归档内容与基线提交重新导出的结果一致（78975 项）；它是通用上游基线，不直接等于现场完整系统。重建还需要匹配的 PetaLinux 2022.2、工具链、设备树/XSA 等依赖。重建后通过文件哈希另行核对与现场 image.ub 的二进制关系。
~~~~

#### 原修改 016

原行号：13–13；当时修改后行号：13–13。

位置：Z035 Linux 构建资料与版本边界 / 独立实验内核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
`experimental/` 是 `5.15.36-xilinx-v2022.2-eeiso22` 隔离实验版本的配置、补丁和模块符号。它与稳定内核属于不同构建，不能混用模块或当作稳定版本。`historical_build_kernel.sh` 保留原始构建命令，包含本机绝对路径，只用于审阅，不能直接运行以免修改既有工程。签名私钥没有收录；本次未重新构建或验证签名。
~~~~

**After**

~~~~text
`experimental/` 是 `5.15.36-xilinx-v2022.2-eeiso22` 隔离实验版本的配置、补丁和模块符号。它与稳定内核独立构建，模块按各自版本和 ABI 配套使用。`historical_build_kernel.sh` 保留原始构建命令，包含本机绝对路径，只用于审阅，不能直接运行以免修改既有工程。签名私钥没有收录；本次未重新构建或验证签名。
~~~~

#### 原修改 017

原行号：15–15；当时修改后行号：15–15。

位置：Z035 Linux 构建资料与版本边界 / 独立实验内核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本次将补丁应用到独立提取的三个相关文件，其结果与本机实验源码相应文件逐字节一致；见 `sim/kernel_archive_patch_20261027.log`。这不是对实验源码全树的比较，也不是完整构建或板端验证。此前有 eeiso22 内核构建与 QEMU 软件测试记录，但这不等于本仓库完成独立复现或实际板端验证。具体配置与补丁应对照所选源码、工具链、DT、模块 ABI 和镜像逐项核验。不得覆盖稳定 SD 卡。
~~~~

**After**

~~~~text
本次将补丁应用到独立提取的三个相关文件，其结果与本机实验源码相应文件逐字节一致；见 `sim/kernel_archive_patch_20261027.log`。本项核对三个相关文件的补丁结果，全树、完整构建及板端验证按对应工程另行执行。此前有 eeiso22 内核构建与 QEMU 软件测试记录，但这不等于本仓库完成独立复现或实际板端验证。具体配置与补丁应对照所选源码、工具链、DT、模块 ABI 和镜像逐项核验。不得覆盖稳定 SD 卡。
~~~~

### reference/readme.md

#### 原修改 018

原行号：1–1；当时修改后行号：1–1。

位置：视频、检测、注意力与 Linux 参考源码

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
# A、B、C 和 Linux 参考源码
~~~~

**After**

~~~~text
# 视频、检测、注意力与 Linux 参考源码
~~~~

#### 原修改 019

原行号：7–7；当时修改后行号：7–7。

位置：视频、检测、注意力与 Linux 参考源码

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| `github_a_fovea/` | `a-fovea`：`729a8d05a01ecac1b97351dfc5d1d6654294f304` | 视频、ROI 和 Vivado/Vitis 参考；其中 XPR 目标为 XC7Z020，不能直接替代 Z035 工程 |
~~~~

**After**

~~~~text
| `github_a_fovea/` | `a-fovea`：`729a8d05a01ecac1b97351dfc5d1d6654294f304` | 视频、ROI 和 Vivado/Vitis 参考；其中 XPR 以 XC7Z020 为目标，Z035 主工程另行构建 |
~~~~

#### 原修改 021

原行号：16–16；当时修改后行号：16–16。

位置：视频、检测、注意力与 Linux 参考源码

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本地 Z035 稳定系统配置、固定提交的完整 Linux 源码归档及独立 eeiso22 实验内核补丁另见 [linux_z035](linux_z035/readme.md)。完整源码通过 Git LFS 分发；它不是已经证明与现场镜像逐字节一致的构建。
~~~~

**After**

~~~~text
本地 Z035 稳定系统配置、固定提交的完整 Linux 源码归档及独立 eeiso22 实验内核补丁另见 [linux_z035](linux_z035/readme.md)。完整源码通过 Git LFS 分发；与现场镜像的二进制对应关系仍待核验。
~~~~

### report/a_repro_checklist_20261027.md

#### 原修改 022

原行号：3–3；当时修改后行号：3–3。

位置：发给队友的复现清单

原因：C/D：仅调整历史资料导读，原阶段事实与记录继续保留

**Before**

~~~~text
**2026-10-28 更新：**以下为 10 月 27 日的历史材料；A 此后已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能上板正常，Attention 应用日志也已收录。无需再为这些功能重复索要资料；最新范围见 [上板应用证据](board_app_evidence_20261028_zh.md)。
~~~~

**After**

~~~~text
**历史记录导读：**下文保留 2026-10-27 的工程状态与步骤。后续已完成 BIT/XSA 构建，收到六项功能现场确认和 Attention 应用日志；当前结果见 [上板应用记录](board_app_evidence_20261028_zh.md)及[干净复现记录](clean_reproduction_20261028_zh.md)。本页的待办列表按当时日期理解。
~~~~

### report/a_repro_status_20261027.md

#### 原修改 023

原行号：3–3；当时修改后行号：3–3。

位置：工程复现状态说明（2026-10-27）

原因：C/D：仅调整历史资料导读，原阶段事实与记录继续保留

**Before**

~~~~text
**2026-10-28 更新：**以下为 10 月 27 日的历史材料；A 此后已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能上板正常，Attention 应用日志也已收录。无需再为这些功能重复索要资料；最新范围见 [上板应用证据](board_app_evidence_20261028_zh.md)。
~~~~

**After**

~~~~text
**历史记录导读：**下文保留 2026-10-27 的工程状态与步骤。后续已完成 BIT/XSA 构建，收到六项功能现场确认和 Attention 应用日志；当前结果见 [上板应用记录](board_app_evidence_20261028_zh.md)及[干净复现记录](clean_reproduction_20261028_zh.md)。本页的待办列表按当时日期理解。
~~~~

### report/a_supplement_results_20261027.md

#### 原修改 024

原行号：3–3；当时修改后行号：3–3。

位置：现场团队的 2026-10-27 复现材料补充

原因：C/D：仅调整历史资料导读，原阶段事实与记录继续保留

**Before**

~~~~text
**2026-10-28 更新：**以下为 10 月 27 日的历史材料；A 此后已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能上板正常，Attention 应用日志也已收录。无需再为这些功能重复索要资料；最新范围见 [上板应用证据](board_app_evidence_20261028_zh.md)。
~~~~

**After**

~~~~text
**历史记录导读：**下文保留 2026-10-27 的工程状态与步骤。后续已完成 BIT/XSA 构建，收到六项功能现场确认和 Attention 应用日志；当前结果见 [上板应用记录](board_app_evidence_20261028_zh.md)及[干净复现记录](clean_reproduction_20261028_zh.md)。本页的待办列表按当时日期理解。
~~~~

### report/ai_collaboration_record.md

#### 原修改 025

原行号：7–7；当时修改后行号：7–7。

位置：EagleEye 大模型协作与工程实践 / 一、协作总体方案

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
本报告选取能够找到会话记录及相应工程材料的六个案例。用户指令和 Codex 判断使用会话原文节选；[会话证据索引](ai_collaboration_evidence_20261030.md)列出摘录、时间、来源文件、行号和会话文件哈希。索引只收录相关片段，不等同于公开完整对话。
~~~~

**After**

~~~~text
本报告选取能够找到会话记录及相应工程材料的六个案例。用户指令和 Codex 判断使用会话原文节选；[会话证据索引](ai_collaboration_evidence_20261030.md)列出摘录、时间、来源文件、行号和会话文件哈希。索引以相关会话节选和来源元数据支持案例追溯。
~~~~

#### 原修改 026

原行号：15–15；当时修改后行号：15–15。

位置：EagleEye 大模型协作与工程实践 / 一、协作总体方案

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
当前视频系统的模型推理在 ARM 端执行，FPGA 用于视频及图像预处理。文末涉及的 V2 卷积卸载是**历史独立实验**，不代表它已集成到当前 Eyes 视频系统。
~~~~

**After**

~~~~text
当前视频系统的模型推理在 ARM 端执行，FPGA 用于视频及图像预处理。文末的 V2 卷积卸载按**历史独立实验**记录，当前 Eyes 的功能与性能采用视频系统自身的测试资料。
~~~~

#### 原修改 027

原行号：25–25；当时修改后行号：25–25。

位置：二、典型协作案例 / 案例 1：Vivado 无法启动，逐步恢复工程构建

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
这次排查也暴露了操作边界：旧路径处理期间，Vivado 曾在安装目录自动生成临时文件；会话明确记录了这一事实，而不是将其描述为完全没有副作用。首次复现中的源码补充和路径处理与后续仓库构建脚本更新是不同步骤。
~~~~

**After**

~~~~text
这次排查也暴露了操作边界：旧路径处理期间，Vivado 曾在安装目录自动生成临时文件；会话保留了临时文件的生成位置，便于后续排查和恢复环境。首次复现中的源码补充和路径处理与后续仓库构建脚本更新是不同步骤。
~~~~

#### 原修改 028

原行号：27–27；当时修改后行号：27–27。

位置：二、典型协作案例 / 案例 1：Vivado 无法启动，逐步恢复工程构建

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
**验证与后续纠错。** 之后团队再次要求从 GitHub 干净复现。那次布线开始后中断，Codex 先确认综合和布局检查点，再从布线阶段续跑，而非将中间状态当成完成。最终生成 BIT/XSA，已约束路径 WNS **+0.608 ns**、WHS **+0.007 ns**；仍有 I/O、CDC 等告警，不能仅凭正裕量视为全部时序闭环。该次新生成文件未再次上板。
~~~~

**After**

~~~~text
**验证与后续纠错。** 之后团队再次要求从 GitHub 干净复现。那次布线开始后中断，Codex 先确认综合和布局检查点，再从布线阶段续跑，而非将中间状态当成完成。最终生成 BIT/XSA，已约束路径 WNS **+0.608 ns**、WHS **+0.007 ns**；该 STA 结果覆盖已约束路径；I/O 与 CDC 的专项状态见告警说明，该次新文件独立板测为后续验证项。
~~~~

#### 原修改 029

原行号：37–37；当时修改后行号：37–37。

位置：二、典型协作案例 / 案例 2：人体框接入注意力模块时发现 Python 兼容错误

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
**首次方案。** Codex 将保存的检测结果转换为下游可读取的数据流，检查坐标、置信度和人数边界；对无有效目标的情况停止生成旧目标请求，同时清理调度及平滑状态。目标编号使用本地临时关联，不冒充 PicoDet 输出的稳定跟踪 ID。
~~~~

**After**

~~~~text
**首次方案。** Codex 将保存的检测结果转换为下游可读取的数据流，检查坐标、置信度和人数边界；对无有效目标的情况停止生成旧目标请求，同时清理调度及平滑状态。目标编号采用本地临时关联，PicoDet 输出与关联 ID 分别定义。
~~~~

#### 原修改 030

原行号：45–45；当时修改后行号：45–45。

位置：二、典型协作案例 / 案例 2：人体框接入注意力模块时发现 Python 兼容错误

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
Codex 随后将写入方式改为通过文件对象 `open(..., newline='\\n')` 输出，并增加命令行生成与防覆盖检查。定向测试记录为 **5 项通过**；这一结果说明离线适配器经过测试，不代表它已经实时接入硬件。
~~~~

**After**

~~~~text
Codex 随后将写入方式改为通过文件对象 `open(..., newline='\\n')` 输出，并增加命令行生成与防覆盖检查。定向测试记录为 **5 项通过**；该项完成离线适配器验证，实时硬件接入按后续阶段测试。
~~~~

#### 原修改 031

原行号：49–49；当时修改后行号：49–49。

位置：二、典型协作案例 / 案例 2：人体框接入注意力模块时发现 Python 兼容错误

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
现场后来确认使用新候选程序上板功能正常；仓库中另有 AttentionPolicy 的应用日志，两者不是同一个可执行文件，不能混作同一次原始运行记录。
~~~~

**After**

~~~~text
现场后来确认使用新候选程序上板功能正常；仓库中另有 AttentionPolicy 的应用日志，两者对应不同 ELF，现场反馈与应用日志分别归档。
~~~~

#### 原修改 032

原行号：59–59；当时修改后行号：59–59。

位置：二、典型协作案例 / 案例 3：从仿真通过到板端固定输入 Golden

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
**方案与反馈。** 流程要求固定输入图像，保存 FPGA 预处理的完整 FP32 张量，以相同算法的参考张量逐值比较，并记录版本和运行日志。现场返回数据后，Codex 核验归档及内部文件哈希，再用仓库工具复核张量，而没有用先前的 RTL 结果代替板端结果：
~~~~

**After**

~~~~text
**方案与反馈。** 流程要求固定输入图像，保存 FPGA 预处理的完整 FP32 张量，以相同算法的参考张量逐值比较，并记录版本和运行日志。现场返回数据后，Codex 核验归档及内部文件哈希，再用仓库工具复核张量，并将主机复核、RTL 与真实板端结果分别记录：
~~~~

#### 原修改 033

原行号：65–65；当时修改后行号：65–65。

位置：二、典型协作案例 / 案例 3：从仿真通过到板端固定输入 Golden

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
**结果。** 在记录的这一张固定输入上，**307,200/307,200 个 FP32 值逐位一致**，最大和平均绝对误差均为 0。独立 BIT 哈希未核实，该结果不能直接绑定到此前另一次上板使用的 BIT，也不能推广为整网精度或实时视频帧新鲜度通过。
~~~~

**After**

~~~~text
**结果。** 在记录的这一张固定输入上，**307,200/307,200 个 FP32 值逐位一致**，最大和平均绝对误差均为 0。该结果覆盖此固定输入的预处理张量；独立 BIT 关联仍待核对，整网精度和实时视频帧新鲜度由各自专项验证。
~~~~

#### 原修改 034

原行号：67–67；当时修改后行号：67–67。

位置：二、典型协作案例 / 案例 3：从仿真通过到板端固定输入 Golden

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
这里的变化是**新增实板证据后更新结论**，不是声称先前的 RTL 仿真有错误。
~~~~

**After**

~~~~text
这一案例展示了从 RTL 到**新增实板证据后更新结论**的验证递进。
~~~~

#### 原修改 035

原行号：87–87；当时修改后行号：87–87。

位置：二、典型协作案例 / 案例 4：性能统计中区分配置收益与同算法加速比

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
这反映两种实际配置在本次测试中的耗时差异，不单独证明同算法硬件加速比，也不能用 HDMI 刷新率代替检测更新率。日志中的 VDMA 错误位提示和 `freshness=unverified` 同时保留。
~~~~

**After**

~~~~text
这些结果反映两种实际配置的计时收益；同算法性能及 HDMI 刷新、检测更新分别使用对应测量口径。日志中的 VDMA 错误位提示和 `freshness=unverified` 同时保留。
~~~~

#### 原修改 036

原行号：89–89；当时修改后行号：89–89。

位置：二、典型协作案例 / 案例 4：性能统计中区分配置收益与同算法加速比

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
该案例体现的是**性能结论的边界检查**；现有会话没有证据表明模型曾给出相反结论，因此不人为编造一次错误回答。
~~~~

**After**

~~~~text
该案例体现 AI 对配置、计时区间与统计口径的联合核对，帮助团队形成可复算的性能分析。
~~~~

#### 原修改 037

原行号：99–99；当时修改后行号：99–99。

位置：二、典型协作案例 / 案例 5：BusyBox 功能差异暴露现场采集脚本问题

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
这次纠错的关键是承认**通过一种 BusyBox 夹具不等于通过另一台板的实际工具集**。退出码、采集结果及现场启动准入各自有不同含义；这一采集脚本属于历史 V2 实验，不是当前视频应用的运行步骤。
~~~~

**After**

~~~~text
这次纠错补齐了**不同 BusyBox 构建所提供命令选项的兼容性测试**。退出码、采集结果及现场启动准入各自有不同含义；这一采集脚本属于历史 V2 实验，不是当前视频应用的运行步骤。
~~~~

#### 原修改 038

原行号：109–109；当时修改后行号：109–109。

位置：二、典型协作案例 / 案例 6：历史独立卷积实验——从驱动拒绝到整网变慢

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
**真实结果与反思。** 后续独立实验记录驱动初始化成功，单次及 20 次重复 Golden 共 **21/21 零差异**；但固定输入的完整 Predictor 平均耗时为 ARM **815.568 ms**、V2 **861.566 ms**。日志中的 pack、decode 和驱动监测区间分别约为 51.685、22.308 和 16.209 ms；监测区间已包含在完整卸载耗时中，不能再次相加，也不等同于纯 PL 计算时间。
~~~~

**After**

~~~~text
**真实结果与反思。** 后续独立实验记录驱动初始化成功，单次及 20 次重复 Golden 共 **21/21 零差异**；但固定输入的完整 Predictor 平均耗时为 ARM **815.568 ms**、V2 **861.566 ms**。日志中的 pack、decode 和驱动监测区间分别约为 51.685、22.308 和 16.209 ms；驱动监测区间包含等待，并作为完整卸载的子区间统计；纯 PL 周期由独立硬件计时获得。
~~~~

#### 原修改 039

原行号：111–111；当时修改后行号：111–111。

位置：二、典型协作案例 / 案例 6：历史独立卷积实验——从驱动拒绝到整网变慢

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
这个案例纠正了“局部计算正确就会提升整网 FPS”的推断：数值正确、真实完成和整网净收益需要分别验证。它属于历史独立实验，不是当前 Eyes 视频系统的摄像头 FPS 或已发布卷积卸载能力。
~~~~

**After**

~~~~text
这个案例纠正了“局部计算正确就会提升整网 FPS”的推断：数值正确、真实完成和整网净收益需要分别验证。该实验为历史架构评估提供实测依据，当前 Eyes 的摄像头检测 FPS 与功能另由自身记录呈现。
~~~~

#### 原修改 040

原行号：143–143；当时修改后行号：143–143。

位置：EagleEye 大模型协作与工程实践 / 四、Skill 与工程经验复用

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
这些技能包含适用范围、使用方法、示例或测试与失败条件。以主机夹具验证的内容不能代替目标硬件验证；通用 PYNQ 示例目前没有 PYNQ 实板及真实跨项目验证记录。各项结果见 [Skill 验证说明](../skill/validation.md)。
~~~~

**After**

~~~~text
这些技能包含适用范围、使用方法、示例或测试与失败条件。主机夹具用于验证工具行为，目标硬件按板端流程验收；PYNQ 示例已完成离线模拟，实板及真实跨项目复用留待后续迁移验证。各项结果见 [Skill 验证说明](../skill/validation.md)。
~~~~

#### 原修改 041

原行号：147–147；当时修改后行号：147–147。

位置：EagleEye 大模型协作与工程实践 / 五、协作成果与反思

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
本项目中，AI 的价值体现在把复杂工程问题拆成能够检查的接口、命令和结果，并在工具失败或收到现场反馈后调整后续步骤。实际过程也显示出局限：一种开发机环境的测试可能漏掉板端工具差异；局部 Golden 正确不能保证整网更快；脚本生成 BIT 也不等同于外设时序和现场功能已经全部验证。
~~~~

**After**

~~~~text
本项目中，AI 将复杂工程问题转化为可执行的接口检查、构建命令和验证脚本，帮助团队快速定位依赖、复核数据并组织可复现材料。现场反馈推动工具迭代：BusyBox 的真实选项差异进入回归，局部数值结果与整网性能分开评估，BIT 构建与外设时序及板端功能分别验收。
~~~~

#### 原修改 042

原行号：149–149；当时修改后行号：149–149。

位置：EagleEye 大模型协作与工程实践 / 五、协作成果与反思

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
团队对目标、约束和真实现场状态的反馈，是修正这些判断的必要输入。报告中没有计算 AI 节省工时的量化数据，不以代码量或工具调用次数代替工程成果。原始会话、部分历史实验构建目录及现场完整串口未全部纳入当前仓库；未能取得的过程不补写成逐字对话或测试通过。
~~~~

**After**

~~~~text
团队负责目标取舍、现场操作和成果确认，AI 通过可追溯的分析与自动化工具辅助执行。协作价值由上述工程案例及验证结果体现，工时节省暂无量化统计。仓库保留相关会话节选与测试依据，完整会话及部分历史构建、串口仍存于本地来源资料。
~~~~

### report/board_app_evidence_20261028_zh.md

#### 原修改 043

原行号：5–5；当时修改后行号：5–5。

位置：实板应用日志及功能反馈（2026-10-28） / 回传与核验

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
现场回传 `eagleeye_z035_repro_with_app_log_20261028.zip`，SHA256 `6e7210373081177ac8f14c0f63833afb4799ccca874509478b20f4cd2524c070`，ZIP CRC 通过；包内 `SHA256SUMS.txt` 的 280 项逐项核验通过。本仓库只新增与本次证据相关的 [应用日志原文](../board/reproduced_20261028/attention_app_20261028.log)与 [现场结果说明原文](../board/reproduced_20261028/board_result_app_source_zh.md)；不需要重复上传包中已经保存的 BIT、BOOT 和启动串口。
~~~~

**After**

~~~~text
现场回传 `eagleeye_z035_repro_with_app_log_20261028.zip`，SHA256 `6e7210373081177ac8f14c0f63833afb4799ccca874509478b20f4cd2524c070`，ZIP CRC 通过；包内 `SHA256SUMS.txt` 的 280 项逐项核验通过。本仓库收录 [应用日志原文](../board/reproduced_20261028/attention_app_20261028.log)与 [现场结果说明原文](../board/reproduced_20261028/board_result_app_source_zh.md)，并与已有 BIT、BOOT 和启动串口记录共同归档。
~~~~

#### 原修改 044

原行号：11–11；当时修改后行号：11–11。

位置：实板应用日志及功能反馈（2026-10-28） / 回传与核验

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| Attention 应用归档 | `b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59` | 已存于 `original/a_repro_delta_20261027.zip`。包内可执行文件哈希 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`；模型哈希 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。这些是归档身份与交接说明，不能替代板上可执行文件和模型的独立读回哈希。 |
~~~~

**After**

~~~~text
| Attention 应用归档 | `b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59` | 已存于 `original/a_repro_delta_20261027.zip`。包内可执行文件哈希 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`；模型哈希 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。以上为归档及交接身份，现场运行文件的独立读回哈希仍待核对。 |
~~~~

#### 原修改 045

原行号：13–13；当时修改后行号：13–13。

位置：实板应用日志及功能反馈（2026-10-28） / 回传与核验

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
旧版 [U-Boot 至登录串口和 BIT 记录](board_reproduced_20261028_zh.md) 已收录。现场团队的归档说明将本次应用日志与同一 BOOT、Attention 归档关联；两份日志没有足够的共同会话标识可以仅凭文件独立证明同一启动会话。
~~~~

**After**

~~~~text
旧版 [U-Boot 至登录串口和 BIT 记录](board_reproduced_20261028_zh.md) 已收录。现场团队的归档说明将本次应用日志与同一 BOOT、Attention 归档关联；两份日志按交接关系归档，共同启动会话标识仍待核对。
~~~~

#### 原修改 046

原行号：17–17；当时修改后行号：17–17。

位置：实板应用日志及功能反馈（2026-10-28） / 日志实际内容

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
按行首统计，日志含 179 条 `ATTENTION`、179 条 `PREPROC backend=fpga`、179 条 `TIME` 和 179 条 `SNAPSHOT`；未发现 `PREPROC_FALLBACK`。其中 79 条注意力记录为 `no_player`，100 条含本地 `target_id`。这些是应用记录，ID 为相邻帧临时匹配，不证明遮挡后稳定跟踪。
~~~~

**After**

~~~~text
按行首统计，日志含 179 条 `ATTENTION`、179 条 `PREPROC backend=fpga`、179 条 `TIME` 和 179 条 `SNAPSHOT`；未发现 `PREPROC_FALLBACK`。其中 79 条注意力记录为 `no_player`，100 条含本地 `target_id`。这些记录呈现应用行为，ID 用于相邻推理结果临时匹配；跨遮挡稳定性另需跟踪验证。
~~~~

#### 原修改 047

原行号：19–19；当时修改后行号：19–19。

位置：实板应用日志及功能反馈（2026-10-28） / 日志实际内容

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
179 条 `SNAPSHOT` 均为 `freshness=unverified`；日志明确警告 `WARNING VDMA error bits present; snapshot consistency is not frame-completion proof`。因此不能把本次记录表述成每帧 DMA 正常、帧完成或帧新鲜度已验证，也不能将预处理后端日志直接当成张量 Golden 正确的证明。
~~~~

**After**

~~~~text
179 条 `SNAPSHOT` 均为 `freshness=unverified`；日志明确警告 `WARNING VDMA error bits present; snapshot consistency is not frame-completion proof`。该日志用于分析应用调用与计时；DMA 帧完成、新鲜度及张量 Golden 一致性按各自专项测试记录。
~~~~

#### 原修改 048

原行号：21–21；当时修改后行号：21–21。

位置：实板应用日志及功能反馈（2026-10-28） / 日志实际内容

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
运行 `python -B tools/summarize_board_log.py board/reproduced_20261028/attention_app_20261028.log` 可复核 179 条时间记录：平均捕获 16.290 ms、预处理 18.329 ms、推理 477.883 ms、后处理 0.028 ms、`total` 512.566 ms，`1000 / mean(total)` 为 **1.951 FPS**；日志中 FPS 字段的算术均值为 1.953 FPS，两种统计口径不同。以上仅为这份日志记录的统计，不是同条件 CPU/FPGA 配对加速比，也不是 HDMI 刷新率或已验证的连续检测更新率。日志中 RTC 为 2018 年，不能用该时钟推断实际测试日期。
~~~~

**After**

~~~~text
运行 `python -B tools/summarize_board_log.py board/reproduced_20261028/attention_app_20261028.log` 可复核 179 条时间记录：平均捕获 16.290 ms、预处理 18.329 ms、推理 477.883 ms、后处理 0.028 ms、`total` 512.566 ms，`1000 / mean(total)` 为 **1.951 FPS**；日志中 FPS 字段的算术均值为 1.953 FPS，两种统计口径不同。以上为日志计时区间的统计；CPU/FPGA 配置对比见专项报告，HDMI 刷新及经帧新鲜度确认的检测更新率分别测量。日志 RTC 为 2018 年，现场测试日期按回传记录标注。
~~~~

#### 原修改 049

原行号：23–23；当时修改后行号：23–23。

位置：实板应用日志及功能反馈（2026-10-28） / 日志实际内容

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
日志未出现 `C_ATTENTION`；应用归档所含 ELF 的哈希不同于此前独立 PicoDet 注意力候选 `1421b51b...`。因此本应用日志证明的是交接所描述的 AttentionPolicy 应用运行记录，不能单凭它证明独立 PicoDet 注意力候选的调用链；用户此前对注意力候选功能正常的反馈单独保留，不与该日志混称。
~~~~

**After**

~~~~text
**应用版本：**该日志对应 AttentionPolicy，未出现 `C_ATTENTION`；归档 ELF 与独立 PicoDet 注意力候选 `1421b51b...` 分别管理。后者的板端功能已有现场确认，其当次原始日志尚待归档。
~~~~

#### 原修改 050

原行号：25–25；当时修改后行号：25–25。

位置：实板应用日志及功能反馈（2026-10-28） / 现场功能与后续验证

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 现场功能确认与未测试项目
~~~~

**After**

~~~~text
## 现场功能与后续验证
~~~~

#### 原修改 051

原行号：27–27；当时修改后行号：27–27。

位置：实板应用日志及功能反馈（2026-10-28） / 现场功能与后续验证

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能均正常；按用户要求，将现场确认作为本次功能验证结论，不再要求重复核验。该结论的来源为现场确认，本包未包含逐项操作时间、独立功能表或相应照片/视频，因此不添加未经记录的具体数值。原始应用日志中的 VDMA 提示同时保留，不据此否定现场功能确认。
~~~~

**After**

~~~~text
现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能均正常，作为本次功能验证结论。记录来源为现场确认；逐项操作时间及照片/视频未随包归档。应用日志中的 VDMA 提示保留在[告警说明](warning_review_zh.md)，与功能记录共同呈现工程状态。
~~~~

#### 原修改 052

原行号：29–29；当时修改后行号：29–31。

位置：现场功能与后续验证 / 后续补充与测试范围

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
现场在本次应用资料回传时确认：同一固定输入的板端预处理输出与 Golden 逐值对比尚未测试；此前的 307,200 值逐位一致为 RTL 仿真。后续新增一次[独立板端固定输入 Golden 测试](preproc_board_golden_20261029_zh.md)，307,200 值逐位一致，但独立 BIT 哈希未核实。另有 [CPU/FPGA A/B 计时](preproc_ab_20261028_zh.md)，两组算法和实时帧不同；独立注意力候选原始应用记录、完整时序和长期稳定性证据仍缺。无需重复提供已回传的 BIT、BOOT、启动串口或本次应用日志。
~~~~

**After**

~~~~text
### 后续补充与测试范围

此次应用回传之后，[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)已补齐 307,200 值逐位一致的记录，独立 BIT 哈希待核对；此前对应结果为 RTL 仿真。[CPU/FPGA A/B 计时](preproc_ab_20261028_zh.md)比较两种缩放算法及先后采集的实时帧。独立注意力候选原始应用日志、完整时序签核及长期稳定性为后续验证项；已有 BIT、BOOT、启动串口及本页应用日志均已归档。
~~~~

### report/board_evidence_20261027.md

#### 原修改 053

原行号：3–3；当时修改后行号：3–3。

位置：本地实板证据补充（2026-10-27）

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本页将两次不同的历史开发板实验单独归档；它们不证明当前 Eyes 完整视频程序的性能或本次上板 BIT 的身份。原始交接包保留在本地，仓库仅收录分析所需记录和输出，未上传启动镜像、设备序列号及现场个人信息。
~~~~

**After**

~~~~text
本页汇总两次历史独立开发板实验：ARM98 逐层计时与 V2/R3 卷积卸载。它们为后续架构选择提供实测依据，与当前 Eyes 视频程序的测试按独立版本归档。仓库收录分析记录和输出，启动镜像、设备序列号及现场个人信息保留在本地原始交接资料中。
~~~~

#### 原修改 054

原行号：9–10；当时修改后行号：9–10。

位置：本地实板证据补充（2026-10-27） / ARM98：纯 ARM 的完整 Predictor 与逐层计时

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 20 帧完整 Predictor 平均 **698.013 ms**；逐卷积均值之和为 **593.708 ms**。这是固定输入下的模型计时，不是摄像头到检测结果的 FPS，也不包含 HDMI 刷新性能。
- 同一输出张量在 20 次运行中逐字节一致；这证明本次重复运行的输出一致，不能代替与 Golden 的精度对照。
~~~~

**After**

~~~~text
- 20 帧完整 Predictor 平均 **698.013 ms**；逐卷积均值之和为 **593.708 ms**。该计时覆盖固定输入的完整模型，摄像头到检测结果及 HDMI 刷新另设测量区间。
- 同一输出张量在 20 次运行中逐字节一致；该项验证重复运行一致性，Golden 精度按独立对照评估。
~~~~

#### 原修改 055

原行号：16–18；当时修改后行号：16–18。

位置：本地实板证据补充（2026-10-27） / V2/R3：历史独立卷积 Golden 和固定输入推理

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 实验 BOOT 及板端读回 SHA256 均为 `f6bf4128938e67080444f300c5f9f80932f3101c3badc1bf561ce449bbf315b0`；实验 FIT SHA256 `ca6dd08854da465b0bd72dbdfd45c39982d019f3e326b0a1491bb8ca0d500e55`。PL 身份读回为 `bcb63d27093a3c122052896eb2e4982cc3ea3ce29d43ebfd1fbc6570ee36f8e5`。这些身份只属于该次实验，不能用于认定当前 Eyes BIT。
- 实板日志记录单次及随后 20 次 Golden 共 **21/21** 次完成，最终 int32 输出差异为 0，错误和锁定状态均为 0。该验证为当时 V2 `conv43/44` 独立实验，不代表 V3 或当前视频工程的 Golden 结果。
- 20 次固定输入全模型准确性记录 `pass=1`；其范围是本次输入及日志指定的检查，不能表述为 14 图全模型资格通过。实验日志同时标记 `diagnostic_only=1`、`formal_offload=0`。
~~~~

**After**

~~~~text
- 实验 BOOT 及板端读回 SHA256 均为 `f6bf4128938e67080444f300c5f9f80932f3101c3badc1bf561ce449bbf315b0`；实验 FIT SHA256 `ca6dd08854da465b0bd72dbdfd45c39982d019f3e326b0a1491bb8ca0d500e55`。PL 身份读回为 `bcb63d27093a3c122052896eb2e4982cc3ea3ce29d43ebfd1fbc6570ee36f8e5`。这些身份绑定本次独立实验，当前 Eyes BIT 按自己的版本表核验。
- 实板日志记录单次及随后 20 次 Golden 共 **21/21** 次完成，最终 int32 输出差异为 0，错误和锁定状态均为 0。该验证覆盖当时 V2 `conv43/44`，V3 与当前视频工程按各自 Golden 记录管理。
- 20 次固定输入全模型准确性记录 `pass=1`；其范围为本次固定输入和指定检查；14 图全模型资格属于另一测试集合。实验日志同时标记 `diagnostic_only=1`、`formal_offload=0`。
~~~~

#### 原修改 056

原行号：20–20；当时修改后行号：20–20。

位置：本地实板证据补充（2026-10-27） / V2/R3：历史独立卷积 Golden 和固定输入推理

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 20 次卸载明细的平均值：输入 pack **51.685 ms**、输出 decode **22.308 ms**、驱动计算监测区间 **16.209 ms**、完整卸载 **105.256 ms**。监测区间包含等待并已计入完整卸载时间，不能再次相加；这些数据不能代表纯 FPGA 计算时间。
~~~~

**After**

~~~~text
- 20 次卸载明细的平均值：输入 pack **51.685 ms**、输出 decode **22.308 ms**、驱动计算监测区间 **16.209 ms**、完整卸载 **105.256 ms**。驱动监测区间包含等待，作为完整卸载的子区间统计；纯 PL 计算需单独硬件计时。
~~~~

#### 原修改 057

原行号：24–24；当时修改后行号：24–24。

位置：本地实板证据补充（2026-10-27） / 可复核性与现有缺口

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
运行 `python tools/verify_field_evidence.py` 可从收录数据复核帧数、硬件提交数、逐层统计、重复输出一致性及 V2 Golden/耗时摘要。核验脚本检查原始记录内部一致性，不代替现场验证，也不认定两次实验的配置相同。
~~~~

**After**

~~~~text
运行 `python tools/verify_field_evidence.py` 可从收录数据复核帧数、硬件提交数、逐层统计、重复输出一致性及 V2 Golden/耗时摘要。核验脚本检查归档数据的内部一致性，两次实验的条件与版本分别列于上文。
~~~~

### report/board_measurements.md

#### 原修改 058

原行号：5–5；当时修改后行号：5–5。

位置：已有板端数据与口径

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
复现命令：`python tools/summarize_board_log.py board/attach_100.log`。脚本只计算日志字段，不推断硬件身份或两次测试是否可比较。
~~~~

**After**

~~~~text
复现命令：`python tools/summarize_board_log.py board/attach_100.log`。脚本从日志字段重算统计，硬件身份及配对条件见各次测试说明。
~~~~

#### 原修改 059

原行号：17–17；当时修改后行号：17–17。

位置：已有板端数据与口径

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
该记录用于说明已有历史 CPU 基线；文件自身及原 Eyes README 说明它不是当前 Eyes FPGA 预处理版本的同条件对照。不能把这组数与不同版本或不同场景的 FPGA 结果相除宣称加速比。`board/attach_100_result.json` 仅对应第 100 帧，不能代表平均值。
~~~~

**After**

~~~~text
该记录提供历史 CPU 基线；与当前 Eyes FPGA 预处理测试的版本及场景不同，按各自条件列示。`board/attach_100_result.json` 保存第 100 帧结果，均值由完整 100 条记录计算。
~~~~

#### 原修改 060

原行号：21–21；当时修改后行号：21–21。

位置：已有板端数据与口径

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
另有历史 ARM98 单线程实板逐层计时和独立 V2/R3 实板 Golden 与固定输入推理记录，原始数据摘录、输出及其验证方法见 [本地实板证据补充](board_evidence_20261027.md)。这些实验不能替代当前 Eyes 的同条件测试。
~~~~

**After**

~~~~text
另有历史 ARM98 单线程实板逐层计时和独立 V2/R3 实板 Golden 与固定输入推理记录，原始数据摘录、输出及其验证方法见 [本地实板证据补充](board_evidence_20261027.md)。这些固定输入实验与当前 Eyes 视频测试按各自条件统计。
~~~~

#### 原修改 061

原行号：25–25；当时修改后行号：25–25。

位置：已有板端数据与口径 / 新收到的 Attention 应用记录

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
2026-10-28 应用日志含 179 条 `TIME`，平均预处理 18.329 ms、平均 total 512.566 ms，`1000 / 平均 total` 为 1.951 FPS；详见 [日志证据及限制](board_app_evidence_20261028_zh.md)。日志含 VDMA 错误位警告且 `freshness=unverified`；该数据不能直接与上述历史 CPU 记录比较为加速比，也不能当成已验证的检测更新 FPS。
~~~~

**After**

~~~~text
2026-10-28 应用日志含 179 条 `TIME`，平均预处理 18.329 ms、平均 total 512.566 ms，`1000 / 平均 total` 为 1.951 FPS；详见 [日志证据及限制](board_app_evidence_20261028_zh.md)。日志含 VDMA 错误位警告且 `freshness=unverified`；该数据属于本次日志计时区间的结果，与历史 CPU 记录分别列示；帧新鲜度确认后的检测更新率仍待测量。
~~~~

#### 原修改 062

原行号：29–29；当时修改后行号：29–29。

位置：已有板端数据与口径 / CPU/FPGA 预处理 A/B 实测

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
现场团队的实板记录经独立重算，30 帧均值：CPU 双三次预处理 416.960 ms、FPGA 双线性预处理 17.537 ms；`total_ms` 分别为 903.150 ms 与 502.665 ms。两组算法及实时帧不同，不能据此宣称相同算法的加速比或数值一致。完整条件和原始日志见 [A/B 证据](preproc_ab_20261028_zh.md)。
~~~~

**After**

~~~~text
现场团队的实板记录经独立重算，30 帧均值：CPU 双三次预处理 416.960 ms、FPGA 双线性预处理 17.537 ms；`total_ms` 分别为 903.150 ms 与 502.665 ms。两组比较不同算法和实时输入配置；同算法收益及数值一致性由相应专项测试评估。完整条件和原始日志见 [A/B 证据](preproc_ab_20261028_zh.md)。
~~~~

### report/board_reproduced_20261028_zh.md

#### 原修改 063

原行号：16–16；当时修改后行号：16–16。

位置：复现 BIT 的上板记录（2026-10-28） / 版本和资料来源

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
**后续补充：**现场已回传登录后的 Attention 应用日志，并确认六项功能上板正常；范围、VDMA 警告和仍未完成的输出对比见 [应用证据](board_app_evidence_20261028_zh.md)。下文对“不含应用输出”的描述仅指原启动串口文件。
~~~~

**After**

~~~~text
**后续验证：**登录后的 Attention 应用日志和六项功能确认见 [应用证据](board_app_evidence_20261028_zh.md)，固定输入预处理结果见[独立板端 Golden](preproc_board_golden_20261029_zh.md)。原启动串口与登录后应用记录分别归档。
~~~~

#### 原修改 064

原行号：20–23；当时修改后行号：20–23。

位置：复现 BIT 的上板记录（2026-10-28） / 已核验的启动记录和版本关系

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 用 Vivado 2022.2 的 Bootgen 只读解析 BOOT，发现 `zynq_fsbl.elf`、`system.bit`、`u-boot.elf` 和 `system.dtb` 四个分区。经字节序转换，`system.bit` 分区从配置同步字起至末尾的内容与交接 BIT 的相应部分一致；该校验是文件内容核验，并非硬件配置读回。
- 串口直接记录了 U-Boot 2022.01 查找并执行 `boot.scr`、FIT 内核/ramdisk/DT 的 SHA256 哈希检查、Linux 5.15.36 启动，最终出现 PetaLinux `login:`。日志出现环境区无效并采用默认环境的提示；单凭日志不能推断其对所有运行场景的影响。
- FIT 日志中的哈希检查是完整性检查，不能据此宣称签名认证通过。日志不含 FSBL 阶段，也没有登录后的 `ATTENTION`、`PREPROC`、`TIME` 等应用输出。
- 现场已确认使用本次复现 BIT 上板后，摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 均正常。本项依据现场功能确认；原始包未包含逐项自动化测试表或量化性能对比，不补写未记录的数字。
~~~~

**After**

~~~~text
- 用 Vivado 2022.2 的 Bootgen 只读解析 BOOT，发现 `zynq_fsbl.elf`、`system.bit`、`u-boot.elf` 和 `system.dtb` 四个分区。经字节序转换，`system.bit` 分区从配置同步字起至末尾的内容与交接 BIT 的相应部分一致；该项完成 BOOT 与 BIT 的文件配置数据核验，硬件配置读回另作验证。
- 串口直接记录了 U-Boot 2022.01 查找并执行 `boot.scr`、FIT 内核/ramdisk/DT 的 SHA256 哈希检查、Linux 5.15.36 启动，最终出现 PetaLinux `login:`。日志出现环境区无效并采用默认环境的提示；该提示对运行场景的影响待专项确认。
- FIT 日志记录 SHA256 完整性检查，来源签名认证属于单独机制。日志不含 FSBL 阶段，也没有登录后的 `ATTENTION`、`PREPROC`、`TIME` 等应用输出。
- 现场已确认使用本次复现 BIT 上板后，摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 均正常。本项依据现场功能确认；量化性能由后续专项测试记录。
~~~~

#### 原修改 065

原行号：25–25；当时修改后行号：25–25。

位置：复现 BIT 的上板记录（2026-10-28） / 验证范围与后续检查

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 尚未证明
~~~~

**After**

~~~~text
## 验证范围与后续检查
~~~~

#### 原修改 066

原行号：27–27；当时修改后行号：27–27。

位置：复现 BIT 的上板记录（2026-10-28） / 验证范围与后续检查

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
当前记录不能独立核对实验卡的 BOOT 读回清单、板上正在运行的应用及模型哈希；本次新增应用日志的 `TIME` 统计见应用证据；随后已收到 [CPU/FPGA 实际配置 A/B 计时](preproc_ab_20261028_zh.md)及另一次[独立固定输入板端 Golden](preproc_board_golden_20261029_zh.md)。后者 BIT 独立哈希未核实，不能当作本次 BIT 的身份验证。同算法性能对照、经帧新鲜度验证的检测更新 FPS 和长期稳定性记录仍无结果。旧 `board/attach_100.log` 及独立 V2/R3 数据属于历史实验，不能代替本次证据。复现实现报告里的外部 I/O 约束和 CDC 等未闭环问题也不能因现场反馈正常而视为时序签核完成。
~~~~

**After**

~~~~text
本页覆盖复现 BIT 文件关联、U-Boot 至 Linux 登录及现场功能确认。实验卡 BOOT 读回清单和运行应用、模型的独立读回哈希待核对。后续 [应用日志](board_app_evidence_20261028_zh.md)、[CPU/FPGA 配置计时](preproc_ab_20261028_zh.md)及[独立固定输入 Golden](preproc_board_golden_20261029_zh.md)按各自条件补充验证，独立 Golden 的 BIT 关联仍待核对。同算法性能、经帧新鲜度确认的检测更新率及长期稳定性为后续测试项。旧 `board/attach_100.log` 和独立 V2/R3 实验分别归档，外部 I/O 与 CDC 的专项状态见[告警说明](warning_review_zh.md)。
~~~~

#### 原修改 067

原行号：29–29；当时修改后行号：29–29。

位置：复现 BIT 的上板记录（2026-10-28） / 验证范围与后续检查

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
用户提供的日期为本次记录日期；启动日志中的 U-Boot 构建时间和 Linux RTC 时间不应当被当成本次现场测试时间。
~~~~

**After**

~~~~text
本次日期按现场回传记录标注；U-Boot 构建时间和 Linux RTC 保留为各自时间字段。
~~~~

### report/clean_reproduction_20261028_zh.md

#### 原修改 068

原行号：3–3；当时修改后行号：3–3。

位置：GitHub 干净克隆复现记录（2026-10-28）

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
复现对象：GitHub `main` 提交 `8588b389aacab349c8cae3132730d7ed1bac6669`，独立克隆和独立构建目录；本记录提交自身当然不包含在被复现的版本中。本次没有操作开发板或稳定 SD。原始日志、实际历史命令、报告和 BIT/XSA 归档在 [`build/clean_rebuild_20261028`](../build/clean_rebuild_20261028/)，文件哈希见 [sha256sums.txt](../build/clean_rebuild_20261028/sha256sums.txt)。运行 `python -B tools/verify_clean_rebuild_20261028.py` 可核验归档及 BIT/XSA 对应关系。历史命令含本机绝对路径，仅供复核，不能不经调整直接用于其他电脑。
~~~~

**After**

~~~~text
复现对象：GitHub `main` 提交 `8588b389aacab349c8cae3132730d7ed1bac6669`，独立克隆和独立构建目录；本页的复现对象固定为该源提交。本次没有操作开发板或稳定 SD。原始日志、实际历史命令、报告和 BIT/XSA 归档在 [`build/clean_rebuild_20261028`](../build/clean_rebuild_20261028/)，文件哈希见 [sha256sums.txt](../build/clean_rebuild_20261028/sha256sums.txt)。运行 `python -B tools/verify_clean_rebuild_20261028.py` 可核验归档及 BIT/XSA 对应关系。历史命令保留本机路径以支持复核，在其他电脑执行时按[复现指南](reproduction_zh.md)设置独立目录。
~~~~

#### 原修改 069

原行号：9–9；当时修改后行号：9–9。

位置：GitHub 干净克隆复现记录（2026-10-28） / 获取、源码与功能验证

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
Skill 单元测试 5 项通过；PicoDet 注意力主机测试及模拟无目标保持测试通过。足球检测参考代码的 12 项 PC 测试及样例流解析通过，不能据此宣称它接入当前 Z035。上述主机测试没有单独留存本次原始日志，仅记录当次执行结果。
~~~~

**After**

~~~~text
Skill 单元测试 5 项通过；PicoDet 注意力主机测试及模拟无目标保持测试通过。足球检测参考代码的 12 项 PC 测试及样例流解析通过，其范围为足球参考路线的 PC 软件验证。上述主机测试没有单独留存本次原始日志，仅记录当次执行结果。
~~~~

#### 原修改 070

原行号：11–11；当时修改后行号：11–11。

位置：GitHub 干净克隆复现记录（2026-10-28） / 获取、源码与功能验证

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
RTL 测试采用 Windows Python 3.13 / NumPy 2.5.2 生成和比较，WSL Ubuntu 20.04 中 Icarus Verilog 10.3 运行仿真；实际命令和输出见 [RTL 日志](../build/clean_rebuild_20261028/logs/rtl_simulation.log)及[历史运行器](../build/clean_rebuild_20261028/commands/rtl_runner_historical.txt)。五类输入各 307200 项与参考逐位一致，RGB 适配器、采集帧适配器与核心、AXI-Lite 和舵机 RTL 测试通过。本次是分环境执行，并未原样执行仓库 `run_verification.py`；标准单环境执行方式见[复现说明](reproduction_zh.md)。仿真不等于上板验证。
~~~~

**After**

~~~~text
RTL 测试采用 Windows Python 3.13 / NumPy 2.5.2 生成和比较，WSL Ubuntu 20.04 中 Icarus Verilog 10.3 运行仿真；实际命令和输出见 [RTL 日志](../build/clean_rebuild_20261028/logs/rtl_simulation.log)及[历史运行器](../build/clean_rebuild_20261028/commands/rtl_runner_historical.txt)。五类输入各 307200 项与参考逐位一致，RGB 适配器、采集帧适配器与核心、AXI-Lite 和舵机 RTL 测试通过。本次是分环境执行，并未原样执行仓库 `run_verification.py`；标准单环境执行方式见[复现说明](reproduction_zh.md)。该阶段完成 RTL 离线验证，板端结果按对应现场记录列示。
~~~~

#### 原修改 071

原行号：15–15；当时修改后行号：15–15。

位置：GitHub 干净克隆复现记录（2026-10-28） / 完整 Vivado 工程

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
Vivado 2022.2 对恢复的 工程快照完成 BD 校验、26 个 IP 子综合及顶层综合、布局和物理优化。首次运行在布线期间中断；保留[首次日志](../build/clean_rebuild_20261028/logs/vivado_initial.log)和[中断前实现日志](../build/clean_rebuild_20261028/logs/vivado_impl_before_resume.log)。确认已完成物理优化检查点后，仅重置布线步骤并从检查点继续，生成 BIT 和 XSA；[续跑命令](../build/clean_rebuild_20261028/commands/vivado_resume_historical.txt)和[日志](../build/clean_rebuild_20261028/logs/vivado_resume.log)保留原貌。**因此这是干净输入、一次中断后续跑完成，并非一次不中断完成的全流程。**
~~~~

**After**

~~~~text
Vivado 2022.2 对恢复的 工程快照完成 BD 校验、26 个 IP 子综合及顶层综合、布局和物理优化。首次运行在布线期间中断；保留[首次日志](../build/clean_rebuild_20261028/logs/vivado_initial.log)和[中断前实现日志](../build/clean_rebuild_20261028/logs/vivado_impl_before_resume.log)。确认已完成物理优化检查点后，仅重置布线步骤并从检查点继续，生成 BIT 和 XSA；[续跑命令](../build/clean_rebuild_20261028/commands/vivado_resume_historical.txt)和[日志](../build/clean_rebuild_20261028/logs/vivado_resume.log)保留原貌。本次以干净输入完成构建，过程包含一次布线中断及检查点续跑，相关日志完整保留。
~~~~

#### 原修改 072

原行号：28–28；当时修改后行号：28–28。

位置：GitHub 干净克隆复现记录（2026-10-28） / 完整 Vivado 工程

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
原始[资源](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[DRC](../build/clean_rebuild_20261028/reports/drc.rpt)、[CDC](../build/clean_rebuild_20261028/reports/cdc.rpt)、[Methodology](../build/clean_rebuild_20261028/reports/methodology.rpt)报告供逐项核查。首次日志亦有 XDC 端口未匹配和管脚告警；必须结合设计与板级资料判断。WNS 为正并不能证明未约束的外部接口或所有 CDC 已闭环。
~~~~

**After**

~~~~text
原始[资源](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[DRC](../build/clean_rebuild_20261028/reports/drc.rpt)、[CDC](../build/clean_rebuild_20261028/reports/cdc.rpt)、[Methodology](../build/clean_rebuild_20261028/reports/methodology.rpt)报告供逐项核查。首次日志亦有 XDC 端口未匹配和管脚告警；必须结合设计与板级资料判断。WNS 描述已约束路径，外部接口和 CDC 的专项签核状态见[告警说明](warning_review_zh.md)。
~~~~

#### 原修改 073

原行号：30–30；当时修改后行号：30–30。

位置：GitHub 干净克隆复现记录（2026-10-28） / 完整 Vivado 工程

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本次 BIT 与现场此前上板反馈使用的 [`b286f480...`](../board/reproduced_20261028/z100_top.bit) 文件长度相同，仅 7 字节不同，均位于生成日期/时间头部；从同步字 `aa995566` 起的配置数据逐字节一致，SHA256 均为 `02c426b197721f9b367e33d9047dfe3e7c84191c90227a18f45411bd94526076`。新旧 XSA 中 HWH 只有 TIMESTAMP 不同，XSA 内 BIT 均与对应独立 BIT 一致。**整文件哈希不同；本次新 BIT 尚未在板上验证。**此前现场反馈和串口范围见[既有板端记录](board_reproduced_20261028_zh.md)。
~~~~

**After**

~~~~text
本次 BIT 与现场此前上板反馈使用的 [`b286f480...`](../board/reproduced_20261028/z100_top.bit) 文件长度相同，仅 7 字节不同，均位于生成日期/时间头部；从同步字 `aa995566` 起的配置数据逐字节一致，SHA256 均为 `02c426b197721f9b367e33d9047dfe3e7c84191c90227a18f45411bd94526076`。新旧 XSA 中 HWH 只有 TIMESTAMP 不同，XSA 内 BIT 均与对应独立 BIT 一致。两文件具有相同配置数据及不同整文件哈希；本次新 BIT 的独立板测为后续验证项。此前现场反馈和串口范围见[既有板端记录](board_reproduced_20261028_zh.md)。
~~~~

#### 原修改 074

原行号：34–34；当时修改后行号：34–34。

位置：GitHub 干净克隆复现记录（2026-10-28） / ARM 程序、QEMU 与 Linux

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
使用本机外部 PetaLinux 2022.2 ARM 交叉工具链/sysroot、工程快照内 Paddle-Lite 静态库，独立编译 PicoDet 注意力候选，ELF SHA256 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；独立编译原 GridPolicy 稳定应用 SHA256 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，分别与已有候选和稳定记录一致。见[候选构建日志](../build/clean_rebuild_20261028/logs/app_build.log)和[稳定应用构建日志](../build/clean_rebuild_20261028/logs/stable_app_build.log)。链接警告指出静态应用调用 `dlopen` 时仍可能需要匹配的 glibc 动态库，不应仅凭静态 ELF 宣称运行环境零依赖。
~~~~

**After**

~~~~text
使用本机外部 PetaLinux 2022.2 ARM 交叉工具链/sysroot、工程快照内 Paddle-Lite 静态库，独立编译 PicoDet 注意力候选，ELF SHA256 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；独立编译原 GridPolicy 稳定应用 SHA256 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，分别与已有候选和稳定记录一致。见[候选构建日志](../build/clean_rebuild_20261028/logs/app_build.log)和[稳定应用构建日志](../build/clean_rebuild_20261028/logs/stable_app_build.log)。链接警告指出静态应用调用 `dlopen` 时仍可能需要匹配的 glibc 动态库，部署仍需核对相应 glibc 运行依赖。
~~~~

#### 原修改 075

原行号：36–36；当时修改后行号：36–36。

位置：GitHub 干净克隆复现记录（2026-10-28） / ARM 程序、QEMU 与 Linux

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
QEMU 固定 RGB、单线程 CPU 预处理用例产生 7 个人体框及 `C_ATTENTION` 请求；无目标用例也执行，见[检测](../build/clean_rebuild_20261028/logs/qemu_person.log)和[无目标](../build/clean_rebuild_20261028/logs/qemu_empty.log)日志。`hardware_commit=not_attempted`；QEMU 耗时和 FPS 不是实板速度，也不证明摄像头、HDMI、舵机或 FPGA 推理。
~~~~

**After**

~~~~text
QEMU 固定 RGB、单线程 CPU 预处理用例产生 7 个人体框及 `C_ATTENTION` 请求；无目标用例也执行，见[检测](../build/clean_rebuild_20261028/logs/qemu_person.log)和[无目标](../build/clean_rebuild_20261028/logs/qemu_empty.log)日志。`hardware_commit=not_attempted`；该用例覆盖 QEMU 离线推理及软件请求生成；板端速度、摄像头、HDMI、舵机和 FPGA 行为按实际硬件测试记录。
~~~~

#### 原修改 076

原行号：38–38；当时修改后行号：38–38。

位置：GitHub 干净克隆复现记录（2026-10-28） / ARM 程序、QEMU 与 Linux

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
仓库固定 Linux 源码与稳定配置在独立目录通过 ARM 交叉构建，`kernel.release=5.15.36-xilinx-v2022.2`，生成 zImage 和 172 个模块；配置 SHA256 `6f1063a6e7a38c1f0d264db258a1987c9e46c7ec70c9a3bfcfb527df5aa005c4`，zImage SHA256 `dd442f1a509f453a7931ce47ebf1cdb00d65ce0235275bdb354dbbf8ccaa7bf4`，`Module.symvers` SHA256 `23ff35be01e406a61776b708685a96aa5922309029f7e12eada2d5fdebfb82e0`。见[内核日志](../build/clean_rebuild_20261028/logs/kernel_build.log)和[历史构建命令](../build/clean_rebuild_20261028/commands/kernel_runner_historical.txt)。尚未证明二进制与实板 `image.ub` 相同、可混用模块或可启动；编译依赖本机外部交叉工具链。
~~~~

**After**

~~~~text
仓库固定 Linux 源码与稳定配置在独立目录通过 ARM 交叉构建，`kernel.release=5.15.36-xilinx-v2022.2`，生成 zImage 和 172 个模块；配置 SHA256 `6f1063a6e7a38c1f0d264db258a1987c9e46c7ec70c9a3bfcfb527df5aa005c4`，zImage SHA256 `dd442f1a509f453a7931ce47ebf1cdb00d65ce0235275bdb354dbbf8ccaa7bf4`，`Module.symvers` SHA256 `23ff35be01e406a61776b708685a96aa5922309029f7e12eada2d5fdebfb82e0`。见[内核日志](../build/clean_rebuild_20261028/logs/kernel_build.log)和[历史构建命令](../build/clean_rebuild_20261028/commands/kernel_runner_historical.txt)。与实板 `image.ub` 的二进制对应、模块兼容和启动状态待核验；编译依赖本机外部交叉工具链。
~~~~

#### 原修改 077

原行号：40–40；当时修改后行号：40–40。

位置：GitHub 干净克隆复现记录（2026-10-28） / 原始记录与验证范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 原始日志及尚缺结果
~~~~

**After**

~~~~text
## 原始记录与验证范围
~~~~

#### 原修改 078

原行号：42–42；当时修改后行号：42–42。

位置：GitHub 干净克隆复现记录（2026-10-28） / 原始记录与验证范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
日志为保留原始字节的本机输出；部分包含 WSL 启动信息和 NUL 字节，文本查看器可能显示异常，不能当作加密数据或修改后再冒充原始日志。本次新 BIT 的板端记录、FSBL 原始日志、经帧新鲜度验证的检测更新 FPS，以及外部 I/O 时序和 CDC 告警的闭环依据仍缺。后续收到 Attention 应用日志与现场反馈，具体范围和 VDMA 警告见 [应用证据](board_app_evidence_20261028_zh.md)；另一次[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)逐位通过，但本次 BIT 哈希未核实，不能认定是此处干净复现 BIT 的板测。历史现场反馈不能替代这些证据。
~~~~

**After**

~~~~text
日志为保留原始字节的本机输出；部分包含 WSL 启动信息和 NUL 字节，文本查看器可能显示异常，查阅时使用支持原始字节的工具，保持归档内容原样。本次新 BIT 的板端记录、FSBL 原始日志、经帧新鲜度验证的检测更新 FPS，以及外部 I/O 时序和 CDC 告警的闭环依据仍缺。后续收到 Attention 应用日志与现场反馈，具体范围和 VDMA 警告见 [应用证据](board_app_evidence_20261028_zh.md)；另一次[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)逐位通过，其独立 BIT 哈希与本次干净复现 BIT 的关系待核对；各次现场反馈及文件记录按对应版本保留。
~~~~

### report/contest_evidence_index_zh.md

#### 原修改 079

原行号：3–3；当时修改后行号：3–3。

位置：比赛材料与证据索引（2026-10-29）

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本索引按已收到的比赛 3.3.5 截图整理。它是材料导航和验证范围说明，不代替赛事组织方对完整规则、格式与评分的审核。原始文件、失败记录和现场功能确认均保留。
~~~~

**After**

~~~~text
本索引按比赛 3.3.5 的工程包、Skill 和设计报告要求组织材料，连接源码、构建产物、验证结果及版本记录。提交格式与评分以赛事完整规则为准。
~~~~

#### 原修改 080

原行号：8–10；当时修改后行号：8–10。

位置：比赛材料与证据索引（2026-10-29）

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| 仿真或验证 | [仿真说明](../sim/verification_zh.md)、[原始预处理仿真日志](../sim/preproc_iverilog_20261027.log)、[舵机仿真日志](../sim/servo_iverilog_20261027.log)、[固定输入板端 Golden](preproc_board_golden_20261029_zh.md) | 五类帧仿真逐位一致；一次独立板端固定输入比较 307,200 值逐位一致。Lanczos 历史独立工程通过日志和旧失败日志分别保留，不能直接代表当前 Eyes RTL 的测试结果，见[记录范围](../sim/lanczos_legacy_readme.md)。 |
| 综合与实现 | [干净复现报告](clean_reproduction_20261028_zh.md)、[资源报告](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序报告](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[告警说明](warning_review_zh.md) | Vivado 2022.2 实现记录含资源和 WNS/WHS；WNS/WHS 只覆盖已约束路径。外部 I/O、CDC 和方法学告警尚未闭环，不能称为完整时序签核。 |
| 上板、运行及实测 | [现场团队的 BIT 和启动记录](board_reproduced_20261028_zh.md)、[应用日志与功能确认](board_app_evidence_20261028_zh.md)、[CPU/FPGA 预处理 A/B 计时](preproc_ab_20261028_zh.md)、[板端 Golden](preproc_board_golden_20261029_zh.md) | 现场确认六项功能正常；应用日志含 VDMA 错误位提示、帧新鲜度未验证。独立板端固定输入 Golden 通过；A/B 两组缩放算法及实时帧不同，不是同算法精度或加速比结论。 |
~~~~

**After**

~~~~text
| 仿真或验证 | [仿真说明](../sim/verification_zh.md)、[原始预处理仿真日志](../sim/preproc_iverilog_20261027.log)、[舵机仿真日志](../sim/servo_iverilog_20261027.log)、[固定输入板端 Golden](preproc_board_golden_20261029_zh.md) | 五类帧仿真逐位一致；一次独立板端固定输入比较 307,200 值逐位一致。Lanczos 历史独立工程通过日志和旧失败日志分别保留，各自绑定历史测试版本，见[记录范围](../sim/lanczos_legacy_readme.md)。 |
| 综合与实现 | [干净复现报告](clean_reproduction_20261028_zh.md)、[资源报告](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序报告](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[告警说明](warning_review_zh.md) | Vivado 2022.2 实现记录含资源和 WNS/WHS；WNS/WHS 只覆盖已约束路径。外部 I/O、CDC 和方法学专项检查仍待闭环，当前 STA 结果覆盖已约束路径。 |
| 上板、运行及实测 | [现场团队的 BIT 和启动记录](board_reproduced_20261028_zh.md)、[应用日志与功能确认](board_app_evidence_20261028_zh.md)、[CPU/FPGA 预处理 A/B 计时](preproc_ab_20261028_zh.md)、[板端 Golden](preproc_board_golden_20261029_zh.md) | 现场确认六项功能正常；应用日志含 VDMA 错误位提示、帧新鲜度未验证。独立板端固定输入 Golden 通过；A/B 两组缩放算法及实时帧不同，按两种系统配置的计时结果报告。 |
~~~~

#### 原修改 081

原行号：12–13；当时修改后行号：12–13。

位置：比赛材料与证据索引（2026-10-29）

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| Skill | [技能索引](../skill/index.md)、[独立复用测试](../sim/skill_portability_20261028.txt)、[工具验证记录](../skill/validation.md)、[PYNQ 模拟测试](../sim/pynq_skill_mock_20261028.txt) | 技能按用途统一整理，已提供运行示例、适用范围和验证边界。PYNQ 没有实板及真实跨项目验证，不宣称加分已取得。 |
| 设计报告 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)、[版本说明](release_identity_zh.md) | 有背景、相关工作、原理、分工、性能、资源和复现说明；AI 协作材料为整理稿。现有 CPU/FPGA 计时和两份硬件构建不能分别冒充同算法加速比和资源优化前后对照。 |
~~~~

**After**

~~~~text
| Skill | [技能索引](../skill/index.md)、[独立复用测试](../sim/skill_portability_20261028.txt)、[工具验证记录](../skill/validation.md)、[PYNQ 模拟测试](../sim/pynq_skill_mock_20261028.txt) | 技能按用途统一整理，已提供运行示例、适用范围和验证边界。PYNQ 已完成离线模拟，实板与真实跨项目复用为后续验证项。 |
| 设计报告 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)、[版本说明](release_identity_zh.md) | 有背景、相关工作、原理、分工、性能、资源和复现说明；AI 协作包含真实会话节选和六个案例；性能按配置条件比较，资源按各构建版本列示。 |
~~~~

#### 原修改 082

原行号：30–30；当时修改后行号：30–30。

位置：比赛材料与证据索引（2026-10-29） / 本地已有结果复核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
第三条从已保存日志重算：两组各记录 35 帧、去除前 5 帧；CPU 双三次预处理平均 416.960 ms，FPGA 双线性预处理平均 17.537 ms，`total_ms` 分别为 903.150 ms 和 502.665 ms。完整输入和输出见[A/B 记录](preproc_ab_20261028_zh.md)；该核验只重算原日志，不证明相同输入、张量一致或板端每帧完成。
~~~~

**After**

~~~~text
第三条从已保存日志重算：两组各记录 35 帧、去除前 5 帧；CPU 双三次预处理平均 416.960 ms，FPGA 双线性预处理平均 17.537 ms，`total_ms` 分别为 903.150 ms 和 502.665 ms。完整输入和输出见[A/B 记录](preproc_ab_20261028_zh.md)；该核验重算已保存日志，输入配对、张量一致性及硬件帧完成按专项记录核验。
~~~~

#### 原修改 083

原行号：32–32；当时修改后行号：32–32。

位置：比赛材料与证据索引（2026-10-29） / 验证范围与后续完善

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 仍需核对或取得新证据
~~~~

**After**

~~~~text
## 验证范围与后续完善
~~~~

#### 原修改 084

原行号：34–34；当时修改后行号：34–34。

位置：比赛材料与证据索引（2026-10-29） / 验证范围与后续完善

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
1. 已收到一次独立预处理固定输入板端 Golden 通过记录；独立 BIT 哈希未核实，不能推断与此前上板 BIT 身份相同，也不能推广为所有输入或完整视频链通过。
~~~~

**After**

~~~~text
1. 已收到一次独立预处理固定输入板端 Golden 通过记录；独立 BIT 哈希未核实，按该次固定输入版本记录，跨版本 BIT 关联和完整视频链按各自测试核验。
~~~~

#### 原修改 085

原行号：36–36；当时修改后行号：36–36。

位置：比赛材料与证据索引（2026-10-29） / 验证范围与后续完善

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
3. 明确最终演示选用的应用版本，按[版本说明](release_identity_zh.md)绑定证据；已有现场功能确认不能与其他程序的原始日志混用。
~~~~

**After**

~~~~text
3. 明确最终演示选用的应用版本，按[版本说明](release_identity_zh.md)绑定证据；现场确认及原始日志分别绑定对应程序。
~~~~

#### 原修改 086

原行号：38–38；当时修改后行号：38–38。

位置：比赛材料与证据索引（2026-10-29） / 验证范围与后续完善

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
5. PYNQ 实板与真实跨项目验证属于可选补充；现无记录，模拟候选如实提交。
~~~~

**After**

~~~~text
5. PYNQ 以离线模拟示例提交；实板与真实跨项目复用记录作为可选扩展。
~~~~

#### 原修改 087

原行号：40–40；当时修改后行号：40–40。

位置：比赛材料与证据索引（2026-10-29） / 验证范围与后续完善

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
已有 BIT、BOOT、启动日志、Attention 应用日志、功能确认、模型和 A/B 计时不应重复索要。
~~~~

**After**

~~~~text
BIT、BOOT、启动日志、Attention 应用日志、功能确认、模型及 A/B 计时均已收录，后续材料按上述专题增补。
~~~~

### report/design_report_draft.md

#### 原修改 088

原行号：2–1；当时修改后行号：2–3。

位置：EagleEye: FPGA-assisted video review (draft)

原因：C/D：仅调整历史资料导读，原阶段事实与记录继续保留

**Before**

（原位置为空，本项为新增。）

**After**

~~~~text

> Historical English draft. The current design, board evidence and performance results are maintained in the [Chinese design report](design_report_zh.md); the text below records the earlier drafting stage.
~~~~

### report/design_report_zh.md

#### 原修改 089

原行号：3–3；当时修改后行号：3–3。

位置：EagleEye 仿生视觉转播系统——设计报告

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
**状态：提交整理稿。**已有内容依据仓库源码、工具报告、实板日志及现场确认编写；已有 CPU/FPGA 预处理 A/B 耗时记录，以及一次[独立板端固定输入 Golden 对比](preproc_board_golden_20261029_zh.md)；两次测试的 BIT 身份关系和最终发布版本仍待确认。团队主要职责按确认记录。 本报告对应 Z035 Eyes 基线与后续板端测试；具体 BIT、应用和模型身份见 [版本说明](release_identity_zh.md)。不把独立 V2/V3 卷积实验当作该系统已经集成的功能。
~~~~

**After**

~~~~text
本报告介绍 EagleEye 在 XC7Z035 上的仿生视觉系统设计与实现，涵盖视频处理、FPGA 预处理、ARM 人体检测、注意力决策和机械眼联动。项目已完成源码构建、RTL 仿真、板端功能确认、CPU/FPGA 预处理计时及[独立固定输入 Golden 对比](preproc_board_golden_20261029_zh.md)。具体 BIT、应用和模型身份见 [版本说明](release_identity_zh.md)，各项测试条件和验证范围集中于第 3、4 节。
~~~~

#### 原修改 090

原行号：11–11；当时修改后行号：11–11。

位置：EagleEye 仿生视觉转播系统——设计报告 / 1. 背景、目标和范围

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
系统采用 XC7Z035、OV5640、HDMI 和 ARM/Paddle-Lite：PS 承担人体检测与视觉决策，PL 承担视频处理、模型输入预处理和 PWM 输出。ROI 放大是对局部画面的重采样，不代表新增真实图像细节；系统用于辅助观察，不自动作出裁判结论。
~~~~

**After**

~~~~text
系统采用 XC7Z035、OV5640、HDMI 和 ARM/Paddle-Lite：PS 承担人体检测与视觉决策，PL 承担视频处理、模型输入预处理和 PWM 输出。设计定位为辅助观察，ROI 通过局部重采样改善展示，裁判判断由使用者完成。
~~~~

#### 原修改 091

原行号：17–17；当时修改后行号：17–17。

位置：1. 背景、目标和范围 / 1.1 选题动机与实际需求

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
团队因此选择“全景观察与局部复判相结合”作为本项目的切入点：在一套嵌入式设备上保留全景画面，尝试根据人体检测结果提出关注区域，并提供暂停后由鼠标调整 ROI 的方式，供使用者观察细节；双路舵机用于演示随 ROI 变化的机械控制。这里的“复判”指对**当前暂停画面**的人工观察；现有材料不证明具备多机位、历史片段回放、慢动作或轨迹回放功能。
~~~~

**After**

~~~~text
团队选择“全景观察与局部复判相结合”作为切入点：在嵌入式设备上保留全景，根据人体检测结果提出关注区域，并支持暂停后用鼠标调整 ROI，双路舵机随关注位置联动。这里的“复判”指对**当前暂停画面**的人工观察；多机位、历史片段、慢动作及轨迹回放属于本原型范围之外的功能。
~~~~

#### 原修改 092

原行号：19–19；当时修改后行号：19–19。

位置：1. 背景、目标和范围 / 1.1 选题动机与实际需求

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
这一方案希望减少使用者在全景和局部之间寻找目标、调整观察区域的操作负担。但目前没有针对观察效率、使用体验或判断准确率的对照实验，不能宣称已经量化证明效果；当前模型输出的是人体框，不包含已验证的足球检测或球员身份识别。单摄像头视角、画面遮挡、低分辨率、检测误差和放大不产生原始画面中不存在的细节，均限制观察结果。
~~~~

**After**

~~~~text
该方案面向全景与局部之间的目标定位和观察操作。当前模型输出人体框，足球检测、球员身份识别和观察效率对照评估留待后续研究。系统的使用条件包括单摄像头视角、遮挡、输入分辨率和检测误差；放大所呈现的细节来自原始采样。
~~~~

#### 原修改 093

原行号：21–21；当时修改后行号：21–21。

位置：1. 背景、目标和范围 / 1.1 选题动机与实际需求

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本项目定位为教学与研究性质的画面观察辅助原型，**不是正式 VAR 系统，也不能自动作出比赛裁判结论**。其设计目标和已有功能应与实际采集、原始测试记录和最终演示版本一起理解，不能把目标直接写成实测效果。
~~~~

**After**

~~~~text
本项目定位为教学与研究性质的画面观察辅助原型，专业 VAR 的认证和裁判流程属于另一应用层级。报告以源码、工具输出及现场记录介绍已实现功能，使用效果的量化评估按实际测试范围展开。
~~~~

#### 原修改 094

原行号：27–29；当时修改后行号：27–29。

位置：1. 背景、目标和范围 / 1.2 相关工作及本项目的区别

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| 专业视频回看与 VAR | FIFA 的场馆技术说明介绍了使用比赛转播画面进行视频复核，以及可用于分析的光学跟踪系统 [1]；IFAB 的 VAR 协议规定了受限的复核情形、程序和裁判最终决定权 [2]。 | EagleEye 结合单摄像头全景、暂停画面局部观察与人工 ROI 控制，旨在做观察辅助。现有工程没有证明实现专业 VAR 的多机位回放、赛事流程、认证或裁判系统能力，因此不能称其为 VAR 替代品。 |
| 轻量目标检测 | Yu 等提出 PP-PicoDet，研究轻量骨干、检测结构及移动端精度与效率的权衡 [3]；PaddleDetection 提供 PicoDet 的模型配置和部署说明 [4]。 | 本项目使用已有 PicoDet/Paddle-Lite 方案在 PS 侧产生人体框，并将其接入注意力和 ROI 控制。PicoDet 不是团队原创；论文中的移动设备性能也不是 XC7Z035 的实测性能。已有临时 ID 不等于跨遮挡稳定跟踪。 |
| FPGA 图像处理与预处理 | AMD 的 Vitis Vision 库提供颜色转换、分辨率变换等硬件图像处理功能 [5]，说明这类运算已有成熟的可编程逻辑实现思路。 | 本项目将已有工程中的 PL 预处理接入摄像头、PicoDet 与视频观察链路，并以张量 Golden 校验指定仿真输入。不能把缩放或 FPGA 预处理本身表述成首次提出；当前已有不同预处理算法的板端 A/B 计时；同算法及固定输入对照仍待测试。 |
~~~~

**After**

~~~~text
| 专业视频回看与 VAR | FIFA 的场馆技术说明介绍了使用比赛转播画面进行视频复核，以及可用于分析的光学跟踪系统 [1]；IFAB 的 VAR 协议规定了受限的复核情形、程序和裁判最终决定权 [2]。 | EagleEye 采用单摄像头全景、暂停画面局部观察与人工 ROI 控制，面向嵌入式观察辅助；专业 VAR 的多机位回放、赛事流程和认证形成了明确的应用范围对照。 |
| 轻量目标检测 | Yu 等提出 PP-PicoDet，研究轻量骨干、检测结构及移动端精度与效率的权衡 [3]；PaddleDetection 提供 PicoDet 的模型配置和部署说明 [4]。 | 本项目复用 PicoDet/Paddle-Lite 在 PS 侧产生人体框，接入规则型注意力及 ROI 控制；其贡献在系统适配与协同。文献性能对应论文平台，XC7Z035 结果见本报告实测部分，临时 ID 用于相邻检测结果关联。 |
| FPGA 图像处理与预处理 | AMD 的 Vitis Vision 库提供颜色转换、分辨率变换等硬件图像处理功能 [5]，说明这类运算已有成熟的可编程逻辑实现思路。 | 本项目将 PL 预处理集成到摄像头、PicoDet 与视频观察链路，完成张量 Golden 仿真及独立固定输入板端验证，并比较两种预处理配置的板端耗时；同算法的配对性能与检测精度比较为后续评估项。 |
~~~~

#### 原修改 095

原行号：31–31；当时修改后行号：31–31。

位置：1. 背景、目标和范围 / 1.2 相关工作及本项目的区别

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
**本项目拟体现的贡献**是上述功能面向同一观察场景的软硬件组合：将视频显示和预处理、PS 人体检测、ROI 人工接管及舵机控制组织成一套 XC7Z035 原型，并保留对应源码与复现材料。已有工作分别覆盖视频复核、检测和硬件图像处理；本报告没有与其他完整系统开展同条件实验，因此不声称本原型在成本、精度、速度或适用性上优于它们。具体实现与验证边界见下表及第 3、4 节。
~~~~

**After**

~~~~text
**本项目的设计贡献**在于面向快速运动观察场景的软硬件协同：将视频显示、PL 预处理、PS 人体检测、ROI 人工接管及舵机控制组织成一套 XC7Z035 原型，并提供源码、测试记录和复现工具。相关工作提供技术基础，本报告以本项目的实现与测试说明系统价值，与其他完整系统的同条件横向评价留待后续开展。
~~~~

#### 原修改 096

原行号：39–39；当时修改后行号：39–39。

位置：1. 背景、目标和范围 / 1.2 相关工作及本项目的区别

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
5. AMD. [Vitis Vision Library User Guide](https://docs.amd.com/r/en-US/Vitis_Libraries/vision/overview.html?contentId=tllvbqJ7TwA66BDW6JeAcw). 图像处理库功能参考；该页面不证明本项目直接调用了该库。
~~~~

**After**

~~~~text
5. AMD. [Vitis Vision Library User Guide](https://docs.amd.com/r/en-US/Vitis_Libraries/vision/overview.html?contentId=tllvbqJ7TwA66BDW6JeAcw). 图像处理库功能参考，用于说明相关技术背景。
~~~~

#### 原修改 098

原行号：45–45；当时修改后行号：45–45。

位置：1. 背景、目标和范围 / 1.3 四项可核查的设计贡献

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
**一、视觉关注与机械眼协同。** 系统将关注区域与双路舵机动作联系起来，为屏幕上的 ROI 提供可观察的物理反馈。[`roi_servo_pwm.v`](../src/servo/roi_servo_pwm.v) 将 ROI 坐标映射为水平、垂直 PWM 目标，并设置中心、范围和渐变步进；暂停后也可通过鼠标调整 ROI。现场已确认鼠标 ROI、按键和舵机等功能正常，[舵机 RTL 仿真](../sim/servo_iverilog_20261027.log)覆盖中心、周期及渐变。该方案是视觉关注与机械动作协同的原型：舵机没有位置反馈闭环，现有记录也没有量化机械跟踪误差或响应延迟。
~~~~

**After**

~~~~text
**一、视觉关注与机械眼协同。** 系统将关注区域与双路舵机动作联系起来，为屏幕上的 ROI 提供可观察的物理反馈。[`roi_servo_pwm.v`](../src/servo/roi_servo_pwm.v) 将 ROI 坐标映射为水平、垂直 PWM 目标，并设置中心、范围和渐变步进；暂停后也可通过鼠标调整 ROI。现场已确认鼠标 ROI、按键和舵机等功能正常，[舵机 RTL 仿真](../sim/servo_iverilog_20261027.log)覆盖中心、周期及渐变。机械眼采用 PWM 开环位置控制；机械误差与响应延迟的量化测量列为后续验证项。
~~~~

#### 原修改 099

原行号：47–47；当时修改后行号：47–47。

位置：1. 背景、目标和范围 / 1.3 四项可核查的设计贡献

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
**二、人体检测驱动的规则型注意力与 ROI 决策。** [`fovea_roi.hpp`](../src/ps/src/fovea_roi.hpp) 对 PicoDet 人体框进行筛选、去重和相邻推理结果的临时关联，并综合区域、运动、向画面中心接近、小目标和检测不确定度评分；保持次数与切换门槛用于减少目标频繁切换。选中的 ROI 可经过轨迹平滑，暂停时支持人工调整。处理流程为“人体框 → 候选关联 → 多因素评分 → 保持或切换 → ROI 平滑 → 视频观察与控制”。[Attention 应用日志](board_app_evidence_20261028_zh.md)保存了相应输出；权重是工程规则参数，不是训练得到的神经注意力；临时 ID 不代表跨遮挡稳定跟踪，不同应用版本的日志也不能混用。
~~~~

**After**

~~~~text
**二、人体检测驱动的规则型注意力与 ROI 决策。** [`fovea_roi.hpp`](../src/ps/src/fovea_roi.hpp) 对 PicoDet 人体框进行筛选、去重和相邻推理结果的临时关联，并综合区域、运动、向画面中心接近、小目标和检测不确定度评分；保持次数与切换门槛用于减少目标频繁切换。选中的 ROI 可经过轨迹平滑，暂停时支持人工调整。处理流程为“人体框 → 候选关联 → 多因素评分 → 保持或切换 → ROI 平滑 → 视频观察与控制”。[Attention 应用日志](board_app_evidence_20261028_zh.md)保存了相应输出。权重采用工程规则参数，临时 ID 服务于相邻推理结果关联；其验证范围及应用版本对应关系见第 3 节。
~~~~

#### 原修改 100

原行号：53–53；当时修改后行号：53–53。

位置：1. 背景、目标和范围 / 1.3 四项可核查的设计贡献

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
如图所示，固定倍率下输出位置分为四种相位：整数相位直接取样，水平和垂直半像素分别将六个抽头对称合并为三组，双向半像素将 6×6 邻域合并为九组。这是 RTL 的运算组织方式，显式乘法表达式数量不能直接等同于综合后的 DSP 数量。
~~~~

**After**

~~~~text
如图所示，固定倍率下输出位置分为四种相位：整数相位直接取样，水平和垂直半像素分别将六个抽头对称合并为三组，双向半像素将 6×6 邻域合并为九组。该图描述 RTL 运算组织，实际 DSP 用量以综合与实现报告统计。
~~~~

#### 原修改 101

原行号：55–55；当时修改后行号：55–55。

位置：1. 背景、目标和范围 / 1.3 四项可核查的设计贡献

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
团队记录该设计节省约 67% DSP；目前没有随附的优化前后综合报告供独立重算。新增历史独立工程 XSim 日志分别记录单通道 768 个、RGB 1,536 个输出像素零错误；这些日志不直接证明当前 Eyes RTL 的逐像素结果。当前工程的[资源和时序记录](clean_reproduction_20261028_zh.md)、现场功能确认，以及各版[仿真记录的范围](../sim/lanczos_legacy_readme.md)分别保留。
~~~~

**After**

~~~~text
团队记录该设计节省约 67% DSP；该比例的独立复算仍需优化前后综合报告。历史独立工程 XSim 日志记录单通道 768 个、RGB 1,536 个输出像素零错误，当前 Eyes 工程另有[资源和时序记录](clean_reproduction_20261028_zh.md)及现场功能确认。不同测试版本的对应关系集中见[仿真记录说明](../sim/lanczos_legacy_readme.md)。
~~~~

#### 原修改 102

原行号：57–57；当时修改后行号：57–57。

位置：1. 背景、目标和范围 / 1.3 四项可核查的设计贡献

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
**四、与 PicoDet 对接的 FPGA 预处理 IP。** PL 侧将 640×360 RGB888 输入经双线性缩放和归一化，形成 `1×3×320×320` FP32 NCHW 张量；数据经一路输入 DMA 和三路 R/G/B 输出 DMA 在 IP 与 DDR 之间搬运，PS 侧负责将张量交给 Paddle-Lite 并提供 CPU 回退。见[预处理源码](../src/ip_repo/preproc/)及[硬件接口表](hardware_interfaces_zh.md)。五类帧 RTL 仿真与 Golden 逐位一致；另一次[独立板端固定输入测试](preproc_board_golden_20261029_zh.md)中 307,200/307,200 个值逐位一致，本次独立 BIT 哈希未核实。[A/B 计时](preproc_ab_20261028_zh.md)中 CPU 双三次和 FPGA 双线性预处理的平均耗时分别为 416.960 ms、17.537 ms；由于算法及实时帧不同，这只是两种系统配置的计时，不能称为同算法加速比或整网精度对照。模型推理仍由 ARM 执行。
~~~~

**After**

~~~~text
**四、与 PicoDet 对接的 FPGA 预处理 IP。** PL 侧将 640×360 RGB888 输入经双线性缩放和归一化，形成 `1×3×320×320` FP32 NCHW 张量；数据经一路输入 DMA 和三路 R/G/B 输出 DMA 在 IP 与 DDR 之间搬运，PS 侧负责将张量交给 Paddle-Lite 并提供 CPU 回退。见[预处理源码](../src/ip_repo/preproc/)及[硬件接口表](hardware_interfaces_zh.md)。五类帧 RTL 仿真与 Golden 逐位一致；另一次[独立板端固定输入测试](preproc_board_golden_20261029_zh.md)中 307,200/307,200 个值逐位一致。[A/B 计时](preproc_ab_20261028_zh.md)中 CPU 双三次和 FPGA 双线性预处理的平均耗时分别为 416.960 ms、17.537 ms。两组属于不同算法及实时输入的系统配置比较，测试条件见第 4 节；模型推理由 ARM 执行。
~~~~

#### 原修改 103

原行号：59–59；当时修改后行号：59–59。

位置：1. 背景、目标和范围 / 1.3 四项可核查的设计贡献

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
作为配套工作，仓库还保存测试输入、参考张量、RTL 日志、构建报告、BIT/XSA/HWH 和核验脚本；见[比赛材料与证据索引](contest_evidence_index_zh.md)。这些证据分别覆盖指定产物、仿真或现场记录，不等同于长期稳定性证明。
~~~~

**After**

~~~~text
项目还提供测试输入、参考张量、RTL 日志、构建报告、BIT/XSA/HWH 和核验脚本，形成由实现到验证的资料链；见[比赛材料与证据索引](contest_evidence_index_zh.md)。
~~~~

#### 原修改 106

原行号：71–71；当时修改后行号：71–71。

位置：EagleEye 仿生视觉转播系统——设计报告 / 2. 设计原理与软硬件分工

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
图为本地已有的系统架构说明图，不单独构成实板验证。PL 侧包括摄像头/视频通路、VDMA、ROI 放大及 OSD、图像预处理和 PWM；PS 侧负责 PicoDet 人体检测、ROI 策略、鼠标及控制交互。源码分别位于 `src/vivado/`、`src/ip_repo/`、`src/servo/` 和 `src/ps/`。
~~~~

**After**

~~~~text
系统架构如图所示：PL 侧包括摄像头/视频通路、VDMA、ROI 放大及 OSD、图像预处理和 PWM；PS 侧负责 PicoDet 人体检测、ROI 策略、鼠标及控制交互。源码分别位于 `src/vivado/`、`src/ip_repo/`、`src/servo/` 和 `src/ps/`。
~~~~

#### 原修改 107

原行号：75–75；当时修改后行号：75–75。

位置：EagleEye 仿生视觉转播系统——设计报告 / 2. 设计原理与软硬件分工

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
预处理示例将 640×360 RGB888 转为 `1×3×320×320` FP32 NCHW；具体位宽、DMA/cache、时钟、地址和复位以匹配版本的源码、XSA 和板端配置为准，不能仅凭图确认。
~~~~

**After**

~~~~text
预处理将 640×360 RGB888 转为 `1×3×320×320` FP32 NCHW。位宽、DMA/cache、时钟、地址和复位的版本化定义见匹配的源码、XSA 与[接口表](hardware_interfaces_zh.md)。
~~~~

#### 原修改 108

原行号：79–79；当时修改后行号：79–79。

位置：2. 设计原理与软硬件分工 / 数据流与接口边界

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
1. 摄像头数据进入 Eyes 视频链路；PS 程序从视频帧获取检测输入。HDMI 显示与检测更新是不同速率，不能用画面刷新率代表检测 FPS。
~~~~

**After**

~~~~text
1. 摄像头数据进入 Eyes 视频链路，PS 程序从视频帧获取检测输入。HDMI 显示与检测更新异步运行，分别记录显示和感知速率。
~~~~

#### 原修改 109

原行号：82–82；当时修改后行号：82–82。

位置：2. 设计原理与软硬件分工 / 数据流与接口边界

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
4. 鼠标与 Fovea 请求经过已有控制路径，舵机由相应 PWM RTL 控制。独立 `src/picodet_c_candidate/` 还将人体框送入注意力与调度逻辑；它与稳定 GridPolicy、后续 AttentionPolicy 是不同软件版本，不应混称同一程序。
~~~~

**After**

~~~~text
4. 鼠标与 Fovea 请求经过已有控制路径，舵机由相应 PWM RTL 控制。独立 `src/picodet_c_candidate/` 还将人体框送入注意力与调度逻辑；该候选、稳定 GridPolicy 和后续 AttentionPolicy 按独立软件版本管理。
~~~~

#### 原修改 110

原行号：84–84；当时修改后行号：84–84。

位置：2. 设计原理与软硬件分工 / 数据流与接口边界

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
接口源码说明预期行为，不单独证明现场调用、DDR 一致性或舵机动作。现场已确认新候选上板功能正常；现已收到 AttentionPolicy 版本的登录后应用日志，但日志对应的应用归档与独立 PicoDet 注意力候选 ELF 不同，不能用其代替后者的实板日志。详见 [应用证据](board_app_evidence_20261028_zh.md)。
~~~~

**After**

~~~~text
项目以接口源码定义数据与控制行为，并通过现场确认记录系统功能。AttentionPolicy 登录后应用日志与独立 PicoDet 注意力候选的现场反馈分别绑定各自 ELF，详见 [应用证据](board_app_evidence_20261028_zh.md)。
~~~~

#### 原修改 111

原行号：92–92；当时修改后行号：92–92。

位置：2. 设计原理与软硬件分工 / 2.1 关键接口及版本

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
Fovea AXI-Lite、预处理 IP、四路预处理 DMA 和视频 VDMA 的地址，已按现场提供的 HWH 与 AttentionPolicy 应用源码逐项整理在 [硬件与软件接口表](hardware_interfaces_zh.md)。该表同时列出帧缓冲与预处理张量的尺寸、格式、源码出处，以及时钟与复位的核对边界。它描述特定版本的配置，不可直接当成其他板卡的地址或资源归属。
~~~~

**After**

~~~~text
Fovea AXI-Lite、预处理 IP、四路预处理 DMA 和视频 VDMA 的地址，已按现场提供的 HWH 与 AttentionPolicy 应用源码逐项整理在 [硬件与软件接口表](hardware_interfaces_zh.md)。该表同时列出帧缓冲与预处理张量的尺寸、格式、源码出处，以及时钟与复位的核对边界。跨板移植时，按目标 XSA、设备树和资源归属更新配置。
~~~~

#### 原修改 112

原行号：94–94；当时修改后行号：94–94。

位置：2. 设计原理与软硬件分工 / 2.1 关键接口及版本

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
系统将连续视频接入、显示和格式转换放在 PL 侧，以匹配视频接口和固定数据流；PS 侧承担模型推理、目标选择及交互控制，便于复用 Paddle-Lite 和调整策略。该划分描述设计意图，当前没有同条件实验可据此量化证明端到端性能优势。
~~~~

**After**

~~~~text
系统将连续视频接入、显示和格式转换放在 PL 侧，以匹配视频接口和固定数据流；PS 侧承担模型推理、目标选择及交互控制，便于复用 Paddle-Lite 和调整策略。实际预处理配置的性能收益见第 4 节，分工本身的独立贡献仍需同条件实验量化。
~~~~

#### 原修改 113

原行号：106–106；当时修改后行号：106–106。

位置：2. 设计原理与软硬件分工 / 2.2 五因素注意力与动态中央凹

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
其中，“区域”是图像中心区域的空间先验，不等同于球场语义区域；“接近”描述相对画面中心的距离变化，并非物理速度或对球门的识别；“不确定性”由检测置信度构造，是工程代理值。这些权重是当前策略参数，并非经消融实验确定的最优值。
~~~~

**After**

~~~~text
其中，“区域”采用图像中心的空间先验；“接近”描述目标到画面中心的距离变化；“不确定性”由检测置信度构造。五项均为图像空间的工程特征，权重为当前策略配置；球场语义、物理速度和权重最优性的评估属于后续研究。
~~~~

#### 原修改 114

原行号：112–112；当时修改后行号：112–112。

位置：2. 设计原理与软硬件分工 / 2.2 五因素注意力与动态中央凹

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
选中目标后，系统在 640×360 全局画面上生成固定 320×180 ROI，并对原点进行边界钳位。局部分支横纵各放大 2 倍，形成 640×360 观察窗口；ROI 面积占全局画面的四分之一，但这不等于整机计算量、功耗或 DDR 流量减少 75%。ROI 动画按步长向目标移动，每轴单次最多 12 像素；其平滑更新不会产生新的检测观测，不能用作检测 FPS。
~~~~

**After**

~~~~text
选中目标后，系统在 640×360 全局画面上生成固定 320×180 ROI，并对原点进行边界钳位。局部分支横纵各放大 2 倍，形成 640×360 观察窗口；ROI 面积占全局画面的四分之一；该几何比例及其补数 75% 仅描述画面面积，整机计算、功耗和 DDR 流量按各自路径单独测量。ROI 动画按步长向目标移动，每轴单次最多 12 像素；平滑更新负责显示运动，检测 FPS 按新的推理结果统计。
~~~~

#### 原修改 115

原行号：116–116；当时修改后行号：116–116。

位置：2. 设计原理与软硬件分工 / 2.3 团队分工与接口

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
按团队确认的 A、B、C 职责，并结合原仓库四个分支的源码整理。分工说明用于归纳主要工作范围；集成过程中具体文件可能由多人协作，不能仅凭分支名称推断每个文件的唯一作者。
~~~~

**After**

~~~~text
按团队确认的 A、B、C 职责，并结合原仓库四个分支的源码整理。分工表归纳主要职责，文件实现与系统集成由团队协作完成。
~~~~

#### 原修改 119

原行号：129–129；当时修改后行号：129–129。

位置：2. 设计原理与软硬件分工 / 2.3 团队分工与接口

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
在 `src/ps/src/fovea_roi.hpp` 的 AttentionPolicy 示例中，五项分数依次以 3、1、3、2、3 相乘后求和；这些是人体目标选择策略参数，不表示球参与评分，也不证明数值经准确率或跟踪稳定性实验最优。独立注意力候选应以自己的源码和配置为准。
~~~~

**After**

~~~~text
AttentionPolicy 的五项权重为 3、1、3、2、3，用于人体目标选择。评分对象和图像空间特征见第 2.2 节；独立注意力候选采用自身源码和配置。
~~~~

#### 原修改 120

原行号：135–145；当时修改后行号：135–145。

位置：EagleEye 仿生视觉转播系统——设计报告 / 3. 验证、资源与时序

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| 预处理张量比较器 | `sim/host_preproc.log`：2 项主机测试通过 | 比较工具测试，不是当前系统板端 DMA 输出 |
| 预处理 RTL 仿真 | `sim/preproc_iverilog_20261027.log`：五类帧及采集帧适配器联测逐位一致，每帧 307,200 值 | Icarus 10.3 仿真；不证明实板 DMA、缓存或整个视频系统 |
| 预处理板端固定输入 | [独立板端 Golden](preproc_board_golden_20261029_zh.md)：307,200/307,200 个 FP32 值逐位一致 | 一次独立测试，BOOT 哈希已记录，独立 BIT 哈希和此前版本关系未核实；不证明摄像头帧新鲜度或整网精度 |
| 舵机 PWM RTL 仿真 | `sim/servo_iverilog_20261027.log`：中心保持、周期、渐变及反向测试通过 | 离线 RTL 仿真，不证明实物舵机机械安全 |
| ROI 硬件合同检查器 | `sim/host_roi.log`：1 项主机测试通过 | 检查器测试，不证明按键、鼠标和舵机实板功能 |
| 预处理 IP 独立历史记录 | `sim/preproc_standalone_readme.md` 报告五类帧；六个 RTL/ROM 哈希见 `sim/preproc_standalone_rtl_sha256.txt` | 这是交接文档的历史结果，原始逐帧仿真日志未随本材料提供；不是整个 Eyes 系统的板端输出 |
| Lanczos 源码来源 | `sim/host_lanczos_source.log`：7 个文件哈希匹配 | 不验证像素；原始旧 XSim 日志 `sim/lanczos_legacy_xsim.txt` 显示 768 项错误，不能写成 PASS |
| Lanczos 历史独立仿真 | [单通道日志](../sim/lanczos_single_xsim_pass_legacy.txt)、[RGB 日志](../sim/lanczos_rgb_xsim_pass_legacy.txt) | 分别记录 768 和 1,536 个输出像素零错误；不等同于当前 Eyes RTL 同版本的逐像素验证 |
| 历史实现资源 | `build/z100_top_utilization_placed.rpt`：LUT 24,789；FF 28,234；BRAM 303/500；DSP 49 | 历史 Eyes 布局报告，不是本次重新综合 |
| 历史时序 | `build/z100_top_timing_summary_routed.rpt`：WNS +0.505 ns，WHS +0.051 ns | Vivado 2022.2，已约束路径；不能代替外设时序和稳定性验证 |
| 独立 Eyes 重建 | `build/full_rebuild_20261027/`：LUT 24,986、FF 28,246、BRAM 303、DSP 49；WNS +0.608 ns、WHS +0.007 ns | 本机独立源码副本完成实现；12 项输入/33 项输出延迟缺失，CDC 和方法学仍有 Critical，不能代表完整时序签核 |
~~~~

**After**

~~~~text
| 预处理张量比较器 | `sim/host_preproc.log`：2 项主机测试通过 | 主机比较工具的正反例验证 |
| 预处理 RTL 仿真 | `sim/preproc_iverilog_20261027.log`：五类帧及采集帧适配器联测逐位一致，每帧 307,200 值 | Icarus 10.3 离线数值及流协议验证；板端 DMA/cache 按独立测试记录 |
| 预处理板端固定输入 | [独立板端 Golden](preproc_board_golden_20261029_zh.md)：307,200/307,200 个 FP32 值逐位一致 | 一次固定输入独立程序测试；BOOT 已记录，独立 BIT 关联、摄像头帧新鲜度及整网精度为另外的验证项目 |
| 舵机 PWM RTL 仿真 | `sim/servo_iverilog_20261027.log`：中心保持、周期、渐变及反向测试通过 | 离线 PWM 行为验证；实物限位及供电按现场安全条件核验 |
| ROI 硬件合同检查器 | `sim/host_roi.log`：1 项主机测试通过 | 主机接口检查器测试；实板功能见现场确认 |
| 预处理 IP 独立历史记录 | `sim/preproc_standalone_readme.md` 报告五类帧；六个 RTL/ROM 哈希见 `sim/preproc_standalone_rtl_sha256.txt` | 历史独立 IP 测试说明；原始逐帧日志待归档，完整 Eyes 板端输出按专项测试记录 |
| Lanczos 源码来源 | `sim/host_lanczos_source.log`：7 个文件哈希匹配 | 七文件来源核对；旧 XSim 像素测试另记于 `sim/lanczos_legacy_xsim.txt`，结果为 768 项错误 |
| Lanczos 历史独立仿真 | [单通道日志](../sim/lanczos_single_xsim_pass_legacy.txt)、[RGB 日志](../sim/lanczos_rgb_xsim_pass_legacy.txt) | 分别记录 768 和 1,536 个输出像素零错误；当前 Eyes RTL 的同版本对应关系待核对 |
| 历史实现资源 | `build/z100_top_utilization_placed.rpt`：LUT 24,789；FF 28,234；BRAM 303/500；DSP 49 | 历史 Eyes 布局报告，按该构建版本记录 |
| 历史时序 | `build/z100_top_timing_summary_routed.rpt`：WNS +0.505 ns，WHS +0.051 ns | Vivado 2022.2 已约束路径的 STA 结果；外设时序及稳定性另行评估 |
| 独立 Eyes 重建 | `build/full_rebuild_20261027/`：LUT 24,986、FF 28,246、BRAM 303、DSP 49；WNS +0.608 ns、WHS +0.007 ns | 本机独立源码副本完成实现；12 项输入/33 项输出延迟缺失，CDC 和方法学 Critical 及外部接口时序仍待签核 |
~~~~

#### 原修改 121

原行号：147–149；当时修改后行号：147–149。

位置：EagleEye 仿生视觉转播系统——设计报告 / 3. 验证、资源与时序

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| Attention 应用上板日志 | `report/board_app_evidence_20261028_zh.md`：179 条 `ATTENTION`、FPGA 预处理和 `TIME` | 应用日志含 VDMA 错误位警告、帧新鲜度未验证；没有同输入板端张量与 Golden 对比，也不能代替独立注意力候选日志 |
| PicoDet 注意力候选上板反馈 | `src/picodet_c_candidate/readme.md` | 现场已确认使用该候选上板功能正常；现有日志属于 AttentionPolicy，未将两者日志混用 |
| 历史 CPU 预处理 | `board/attach_100.log` 和 `report/board_measurements.md` | 100 帧统计；不能与其他版本直接相除作为 FPGA 加速比 |
~~~~

**After**

~~~~text
| Attention 应用上板日志 | `report/board_app_evidence_20261028_zh.md`：179 条 `ATTENTION`、FPGA 预处理和 `TIME` | 应用日志含 VDMA 错误位警告、帧新鲜度未验证；该日志用于应用行为和计时分析；固定输入张量对比及独立注意力候选按各自记录管理 |
| PicoDet 注意力候选上板反馈 | `src/picodet_c_candidate/readme.md` | 候选功能已有现场确认；AttentionPolicy 原始日志与候选反馈分别绑定对应版本 |
| 历史 CPU 预处理 | `board/attach_100.log` 和 `report/board_measurements.md` | 历史 100 帧基线，按自身版本与条件统计 |
~~~~

#### 原修改 122

原行号：151–151；当时修改后行号：151–151。

位置：EagleEye 仿生视觉转播系统——设计报告 / 3. 验证、资源与时序

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
工具报告与板端日志中的时钟、I/O、CDC 和 VDMA 告警逐项说明见 [告警说明](warning_review_zh.md)。这些记录与现场已确认的六项功能正常并列保留，不互相替代。
~~~~

**After**

~~~~text
工具报告与板端日志中的时钟、I/O、CDC 和 VDMA 状态集中见 [工程告警说明](warning_review_zh.md)。现场六项功能确认与这些专项检查共同描述系统状态。
~~~~

#### 原修改 123

原行号：153–153；当时修改后行号：153–153。

位置：EagleEye 仿生视觉转播系统——设计报告 / 3. 验证、资源与时序

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
文件完整性检查见 `tools/verify_submission.py`。主机测试只覆盖工具自己的行为，不代表当前板端计算或完整系统 PASS。
~~~~

**After**

~~~~text
文件完整性由 `tools/verify_submission.py` 核验，主机工具、RTL、QEMU 及板端验证按上表分层记录。
~~~~

#### 原修改 124

原行号：155–155；当时修改后行号：155–163。

位置：3. 验证、资源与时序 / 3.1 测试条件与适用范围

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
各测试的输入、比较判据、执行记录和复现入口见 [仿真与验证说明](../sim/verification_zh.md)。独立实验的 ARM98 和 V2/R3 实板记录可作开发背景，不能称为本报告 Eyes 系统的同条件性能数据，详见 [历史实板证据](board_evidence_20261027.md)。
~~~~

**After**

~~~~text
### 3.1 测试条件与适用范围

各测试的输入、比较判据、执行记录和复现入口见 [仿真与验证说明](../sim/verification_zh.md)。统一按以下范围引用结果：

- **当前系统**：PicoDet 推理在 ARM 端，PL 执行视频和图像预处理；独立 V2/V3 卷积研究、ARM98 和 V2/R3 实测作为[历史实验](board_evidence_20261027.md)保存。
- **版本绑定**：独立板端 Golden 的 BOOT 已记录，独立 BIT 哈希及与视频测试版本的关系待核对；AttentionPolicy 与独立注意力候选分别绑定日志或现场反馈。
- **场景能力**：临时 ID 用于相邻结果关联，跨遮挡稳定跟踪另需验证；PWM 采用开环控制，机械误差及响应延迟暂无量化记录；长期稳定性和用户观察效率也留待专项测试。
- **历史 Lanczos**：通过与失败日志均保留原文，测试源码及环境的版本对应尚待核对；67% DSP 为团队记录，独立重算需优化前后报告。
- **工程条件**：板级 I/O、CDC/方法学告警及 VDMA 错误位和帧新鲜度问题，按[告警说明](warning_review_zh.md)记录。
~~~~

#### 原修改 125

原行号：169–169；当时修改后行号：177–177。

位置：EagleEye 仿生视觉转播系统——设计报告 / 4. 优化过程与性能、资源记录

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
这两种实际配置的预处理均值比为 **23.776 倍**，`1000 / 平均 total_ms` 比值为 **1.797 倍**，平均 `total_ms` 缩短约 44.3%。**两组采用不同缩放算法和不同摄像头帧**，不能当作同算法硬件加速比、固定输入逐值比较或精度相同的证明。`total_ms` 不包含之后的 ROI 控制和结果打印；VDMA 状态提示及 `freshness=unverified` 限制了对帧新鲜度和完整检测更新率的判断，不能将该 FPS 当作 HDMI 刷新率。
~~~~

**After**

~~~~text
两种配置的预处理均值比为 **23.776 倍**，`1000 / 平均 total_ms` 比值为 **1.797 倍**，平均 `total_ms` 缩短约 44.3%。该结果描述 CPU 双三次与 FPGA 双线性两种实际配置的计时收益。
~~~~

#### 原修改 126

原行号：171–171；当时修改后行号：179–181。

位置：EagleEye 仿生视觉转播系统——设计报告 / 4. 优化过程与性能、资源记录

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件；A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
历史 `board/attach_100.log` 的 CPU 记录和另一份 179 条 Attention 应用日志保留作独立背景，条件不完全相同，不与该 A/B 测试混算；见 [记录说明](board_measurements.md)。后续[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)已通过，但不能作为这两组不同实时帧的逐帧配对或检测精度比较。
~~~~

**After**

~~~~text
**计时条件：**两组采用不同缩放算法和先后采集的实时帧；固定输入数值对照由独立测试记录，同算法性能评价待后续开展。`total_ms` 截止检测结果解析，后续 ROI 控制和结果打印在该区间之外。VDMA 提示与 `freshness=unverified` 保留为帧完整性及检测更新率的待验证项，HDMI 刷新按独立指标统计。

历史 `board/attach_100.log` 的 CPU 记录和另一份 179 条 Attention 应用日志，按各自条件保留，见 [计时记录说明](board_measurements.md)。后续[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)提供完整张量的一致性结果；实时 A/B 输入的配对检测框精度仍需专项比较。
~~~~

#### 原修改 127

原行号：178–178；当时修改后行号：188–188。

位置：EagleEye 仿生视觉转播系统——设计报告 / 4. 优化过程与性能、资源记录

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
这两行是**不同构建记录**，不是“优化前后”资源收益结论；未约束 I/O、CDC 与方法学告警仍见 [告警说明](warning_review_zh.md)。
~~~~

**After**

~~~~text
两行分别展示历史构建与干净复现的资源和已约束时序结果，按版本保留。外部 I/O、CDC 与方法学问题见 [工程告警说明](warning_review_zh.md)。
~~~~

#### 原修改 128

原行号：180–180；当时修改后行号：190–190。

位置：EagleEye 仿生视觉转播系统——设计报告 / 4. 优化过程与性能、资源记录

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
提供的技术文档还记录了阶段联调观察：检测更新约 2 FPS、视频显示约 30 FPS。两者是不同指标；现有应用日志可重算检测路径耗时，但没有对应的独立显示帧率计时记录，不以显示帧率代表检测速度。文档中早期方案约 0.5 FPS 的观察与当前数据并非同条件配对测试，不据此计算严格的加速比。
~~~~

**After**

~~~~text
系统采用显示与 AI 检测异步运行的架构。技术文档中的阶段观察为检测更新约 2 FPS、视频显示约 30 FPS；前者有应用日志供计时复核，后者为观察值，尚待独立显示计时。早期约 0.5 FPS 也为不同条件的阶段观察，保留作开发背景。
~~~~

#### 原修改 129

原行号：184–184；当时修改后行号：194–194。

位置：EagleEye 仿生视觉转播系统——设计报告 / 5. 大模型协作与技能包

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
团队目前没有 PYNQ 实板和跨项目复用验证资料；PYNQ 示例按离线模拟候选展示，不作为需要队友补交的既有材料，也不声称已获得加分。
~~~~

**After**

~~~~text
项目提炼了视频处理、软硬件接口、模型部署和数据验证的可复用 Skill。PYNQ Overlay、寄存器、DMA 和缓存示例已完成离线模拟验证，为平台迁移提供参考，实板及真实跨项目复用留待后续验证。
~~~~

#### 原修改 130

原行号：186–186；当时修改后行号：196–196。

位置：EagleEye 仿生视觉转播系统——设计报告 / 5. 大模型协作与技能包

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
`report/ai_collaboration_record.md` 为已有的协作整理稿，不是完整原始会话。[技能索引](../skill/index.md)将视频、检测、ROI、构建及 PYNQ 相关技能按用途统一列出，记录各自适用范围、使用方式、验证及失效条件；足球检测和 AX7020 资料是独立参考案例。Lanczos 验证反馈与历史记录见 [验证说明](../sim/lanczos_legacy_readme.md)。完整提示词、回复、纠错轨迹和团队确认需另行补充。
~~~~

**After**

~~~~text
[AI 协作记录](ai_collaboration_record.md)整理六个工程案例，结合真实提示词、模型回答、纠错轨迹及工具结果；[会话证据索引](ai_collaboration_evidence_20261030.md)保存相关原文节选与来源信息。[技能索引](../skill/index.md)按用途提供工作流、最小示例、适用条件和验证方法，足球检测与 AX7020 经验按独立参考案例收录。Lanczos 历史记录见[验证说明](../sim/lanczos_legacy_readme.md)。
~~~~

#### 原修改 131

原行号：188–188；当时修改后行号：198–198。

位置：EagleEye 仿生视觉转播系统——设计报告 / 5. 大模型协作与技能包

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
可复核的协作案例包括：针对板端 BusyBox 差异修订只读采集；从固定输入的 V2/R3 实测发现卸载比 ARM 慢，保留真实日志而不将局部 Golden 等同整网提速；依据团队要求仅移除 Lanczos 的独立 Skill 展示，保留旧测试及失败记录；现场回传启动串口后，将 U-Boot 至 Linux 登录与登录后的现场功能反馈分开。各案例的来源、纠错和局限见 `report/ai_collaboration_record.md` 和相关证据页面，不应把整理稿冒充完整聊天逐字稿。
~~~~

**After**

~~~~text
协作案例展示了 AI 在环境排障、接口适配、统计复核及资料整理中的效率价值：依据真实 BusyBox 功能修订采集工具，将固定输入 Golden 和配置性能分别评估，并以版本化记录连接构建产物与现场结果。各案例提供来源、修正过程及测试结果，读者可按证据索引追溯相关会话。
~~~~

#### 原修改 132

原行号：194–194；当时修改后行号：204–204。

位置：EagleEye 仿生视觉转播系统——设计报告 / 6. 复现

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
3. 按 `report/reproduction_zh.md` 及原快照 `sim/README.md`、`src/ps/build_arm.sh` 准备工具链、Paddle-Lite 依赖及测试输入。独立构建已覆盖 Eyes FPGA、稳定 ARM 应用和固定源码 Linux 内核；现场记录提供的板测 BIT 与 2026-10-27 复现 BIT 哈希一致；2026-10-28 干净复现 BIT 的配置数据一致但整文件哈希不同。实验卡读回清单和现场运行文件独立哈希仍待核对。
~~~~

**After**

~~~~text
3. 按 `report/reproduction_zh.md` 及原快照 `sim/README.md`、`src/ps/build_arm.sh` 准备工具链、Paddle-Lite 依赖及测试输入。独立构建已覆盖 Eyes FPGA、稳定 ARM 应用和固定源码 Linux 内核；现场记录提供的板测 BIT 与 2026-10-27 复现 BIT 哈希一致；2026-10-28 干净复现 BIT 的配置数据一致但整文件哈希不同。实验卡读回清单和现场运行文件独立哈希列于[版本说明](release_identity_zh.md)的待核对项。
~~~~

### report/field_evidence_needed_zh.md

#### 原修改 133

原行号：3–3；当时修改后行号：3–3。

位置：现场证据与待确认事项

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
只列出当前仓库和既有材料不能独立证明的内容。已经提供的源码、模型、BOOT 和只读日志不要求重复发送。
~~~~

**After**

~~~~text
本页集中记录已有结果的验证范围及后续完善事项。源码、模型、BOOT、启动和应用日志按仓库索引直接查阅。
~~~~

#### 原修改 134

原行号：5–5；当时修改后行号：5–5。

位置：现场证据与待确认事项

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
历史 ARM98 逐层计时和 V2/R3 Golden、性能记录已整理，见 [本地实板证据补充](board_evidence_20261027.md)；不再索要这些已有数据。它们不等同于当前 Eyes/PicoDet 注意力的实板记录。
~~~~

**After**

~~~~text
历史 ARM98 逐层计时及 V2/R3 Golden、性能记录见 [独立实板实验](board_evidence_20261027.md)。当前 Eyes/PicoDet 注意力按相应版本的现场记录管理。
~~~~

#### 原修改 135

原行号：9–9；当时修改后行号：9–9。

位置：现场证据与待确认事项 / 板级系统与当前候选

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 现场已提供 `b286f480...` BIT、实验 BOOT、U-Boot 至 Linux 登录的串口原文和现场功能正常反馈，详见 [复现 BIT 的上板记录](board_reproduced_20261028_zh.md)，无需重复提供。若已有，补充实验卡 BOOT 读回清单和板上 `paddle_person_live`、`model.nb` 的身份记录；若没有则注明暂无。旧 `attach_100.log` 不能冒充本次日志。
~~~~

**After**

~~~~text
- 现场已提供 `b286f480...` BIT、实验 BOOT、U-Boot 至 Linux 登录的串口原文和现场功能正常反馈，详见 [复现 BIT 的上板记录](board_reproduced_20261028_zh.md)，无需重复提供。若已有，补充实验卡 BOOT 读回清单和板上 `paddle_person_live`、`model.nb` 的身份记录；未保存的读回信息标记“暂无”；旧 `attach_100.log` 按历史 CPU 运行记录单独列示。
~~~~

#### 原修改 136

原行号：11–11；当时修改后行号：11–11。

位置：现场证据与待确认事项 / 板级系统与当前候选

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- **输出对比：**后续已提供[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)，307,200 个值逐位一致；无需再索要该次输出或比较数据。此前现场团队的“未测试”反馈指更早状态；本次测试的独立 BIT 哈希未核实。已有 CPU/FPGA 预处理 A/B 计时，无需重复提供；两组缩放算法及实时帧不同，见 [A/B 证据](preproc_ab_20261028_zh.md)。经帧新鲜度验证的更新率仍无结果，不拿历史 V2/R3 或 ARM98 结果替代。
~~~~

**After**

~~~~text
- **输出对比：**后续已提供[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)，307,200 个值逐位一致；无需再索要该次输出或比较数据。此前现场团队的“未测试”反馈指更早状态；本次测试的独立 BIT 哈希未核实。已有 CPU/FPGA 预处理 A/B 计时，无需重复提供；两组缩放算法及实时帧不同，见 [A/B 证据](preproc_ab_20261028_zh.md)。经帧新鲜度验证的更新率为后续测试项，历史 V2/R3 和 ARM98 则提供固定输入实验背景。
~~~~

#### 原修改 137

原行号：18–18；当时修改后行号：18–18。

位置：现场证据与待确认事项 / 检测模型与接口

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 若比赛明确采用足球 PLAYER/BALL，则需指出已使用的模型/程序和已存在的证据位置；如尚无可用板端版本，标记未完成。不要把足球参考分支或 QEMU 结果当作 Z035 板测。
~~~~

**After**

~~~~text
- 若比赛明确采用足球 PLAYER/BALL，则需指出已使用的模型/程序和已存在的证据位置；如尚无可用板端版本，标记未完成。足球参考分支、QEMU 和 Z035 板测分别标注执行环境。
~~~~

### report/hardware_interfaces_zh.md

#### 原修改 139

原行号：3–3；当时修改后行号：3–3。

位置：Eyes 实现的硬件与软件接口表

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本表对应现场回传并确认功能正常的 `b286f480...` BIT 所配套的 [`system.hwh`](../board/reproduced_20261028/system.hwh) 与 `src/ps/src/` AttentionPolicy 源码；用于报告和版本核对，不是操作寄存器的指导。HWH 所列区间为地址空间分配，应用实际映射通常为 4 KiB。独立 PicoDet 注意力候选的执行文件不同，需以其对应构建资料为准。
~~~~

**After**

~~~~text
本表对应现场回传并确认功能正常的 `b286f480...` BIT 所配套的 [`system.hwh`](../board/reproduced_20261028/system.hwh) 与 `src/ps/src/` AttentionPolicy 源码；用于设计说明和版本核对；实际寄存器操作遵循匹配程序与目标设备的控制协议。HWH 所列区间为地址空间分配，应用实际映射通常为 4 KiB。独立 PicoDet 注意力候选的执行文件不同，需以其对应构建资料为准。
~~~~

#### 原修改 140

原行号：14–14；当时修改后行号：14–14。

位置：Eyes 实现的硬件与软件接口表

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
上述 HWH 地址可对照 [`paddle_person.cpp`](../src/ps/src/paddle_person.cpp)、[`frame_layout.h`](../src/ps/src/frame_layout.h) 与 [`eagleeye_hw.cpp`](../src/ps/src/eagleeye_hw.cpp)；不应将这些地址移植到其他版本而不复核 HWH、设备树与实际所有权。
~~~~

**After**

~~~~text
上述 HWH 地址可对照 [`paddle_person.cpp`](../src/ps/src/paddle_person.cpp)、[`frame_layout.h`](../src/ps/src/frame_layout.h) 与 [`eagleeye_hw.cpp`](../src/ps/src/eagleeye_hw.cpp)；移植地址前需复核目标 HWH、设备树及资源所有权。
~~~~

#### 原修改 141

原行号：18–18；当时修改后行号：18–18。

位置：Eyes 实现的硬件与软件接口表 / 缓冲、数据类型和控制

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 视频：源码配置 640×360、RGB 三字节、stride 1920，三个大小各 `0xb0000` 的帧槽，从 `0x1e000000` 起共 `0x210000` 字节。源码中的长度和设备树预留区应一起核对；不能把地址表当作现场 DMA 已停止或可与其他程序共享的证明。
~~~~

**After**

~~~~text
- 视频：源码配置 640×360、RGB 三字节、stride 1920，三个大小各 `0xb0000` 的帧槽，从 `0x1e000000` 起共 `0x210000` 字节。源码中的长度和设备树预留区应一起核对；现场 DMA 停止及共享条件由资源所有权和硬件状态核验。
~~~~

#### 原修改 142

原行号：20–21；当时修改后行号：20–21。

位置：Eyes 实现的硬件与软件接口表 / 缓冲、数据类型和控制

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- ROI：应用通过 AXI-Lite 传入请求序号、区域坐标、尺寸、模式等并检查状态；`0x64` 的值是该模块接口标识，不等同于完整 BIT 现场身份认证。本地匹配 ID 不保证跨遮挡稳定。
- 实际程序使用 `/dev/mem` 映射指定内存及寄存器，并在源码中插入内存屏障；屏障不单独证明所有缓存或不同设备/系统的内存一致性，移植时需遵循目标平台的 DMA 契约。
~~~~

**After**

~~~~text
- ROI：应用通过 AXI-Lite 传入请求序号、区域坐标、尺寸、模式等并检查状态；`0x64` 的值是该模块接口标识，用于模块接口识别，完整 BIT 身份另行核验。本地匹配 ID 不保证跨遮挡稳定。
- 实际程序使用 `/dev/mem` 映射指定内存及寄存器，并在源码中插入内存屏障；缓存维护与可见性依赖目标平台的 DMA 契约，移植时结合映射属性和驱动核验。
~~~~

### report/preproc_ab_20261028_zh.md

#### 原修改 143

原行号：9–9；当时修改后行号：9–9。

位置：Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28） / 资料来源与条件

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
**重要区别：CPU 路径采用双三次缩放和归一化，FPGA 路径采用双线性缩放和归一化。**两组先后使用实时摄像头，帧尺寸相同但不是同一张输入图像；因此下表是两种实际配置的耗时对比，不是同算法、逐帧配对的纯硬件加速比。PicoDet 模型推理在两组均由 ARM 执行，不代表 FPGA 卷积加速。
~~~~

**After**

~~~~text
**测试配置：CPU 采用双三次缩放与归一化，FPGA 采用双线性缩放与归一化。**两组使用相同帧尺寸、先后采集的实时画面，下表评价这两种实际配置的耗时；同算法、逐帧配对性能另需测试。两组 PicoDet 模型推理均由 ARM 执行。
~~~~

#### 原修改 144

原行号：24–24；当时修改后行号：24–24。

位置：Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28） / 独立重算的结果

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
上述两种配置的预处理均值比为 **23.776 倍**；`1000 / 平均 total_ms` 的比值为 **1.797 倍**，对应总耗时均值减少 400.485 ms（约 44.3%）。这些数值只描述本次两种配置；不能推断同样算法的硬件加速比、20 FPS，或两种检测结果精度相同。
~~~~

**After**

~~~~text
上述两种配置的预处理均值比为 **23.776 倍**；`1000 / 平均 total_ms` 的比值为 **1.797 倍**，对应总耗时均值减少 400.485 ms（约 44.3%）。以上为本次两种配置的计时收益；同算法硬件收益及检测框精度一致性分别验证，20 FPS 仍为独立性能目标。
~~~~

#### 原修改 145

原行号：26–26；当时修改后行号：26–26。

位置：Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28） / 独立重算的结果

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
根据应用源码，`total_ms` 从采集开始到检测结果解析结束，包含该时间区间内的快照记录等开销；不包含之后的 ROI 控制和结果打印。帧新鲜度未确认，因此以上 FPS 是日志计时区间的换算值，不能当成 HDMI 刷新率或已经验证的完整摄像头至 ROI 控制更新率。FPGA 预处理计时包含当前源码中的输入/输出复制、寄存器与 DMA 调用和写入模型输入；没有独立拆出 cache 维护或内核驱动时间。
~~~~

**After**

~~~~text
根据应用源码，`total_ms` 从采集开始到检测结果解析结束，包含该时间区间内的快照记录等开销；不包含之后的 ROI 控制和结果打印。以上 FPS 按日志计时区间换算；经帧新鲜度确认的摄像头至 ROI 更新率及 HDMI 刷新率分别测量。FPGA 预处理计时包含当前源码中的输入/输出复制、寄存器与 DMA 调用和写入模型输入；没有独立拆出 cache 维护或内核驱动时间。
~~~~

#### 原修改 146

原行号：28–28；当时修改后行号：28–28。

位置：Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28） / 正确性与计时适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 正确性及日志边界
~~~~

**After**

~~~~text
## 正确性与计时适用范围
~~~~

#### 原修改 147

原行号：30–33；当时修改后行号：30–33。

位置：Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28） / 正确性与计时适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 两组各 35 条 `PREPROC backend` 与 `TIME` 一一对应，无 `PREPROC_FALLBACK`。两份日志的 35 条 `SNAPSHOT` 均为 `content_changed=1`，但也均为 `freshness=unverified`，不能证明每帧硬件完成和新鲜度。
- 三份 VDMA 状态记录相同，均提示 `latched VDMA errors`；原记录保留未清除。不由该提示否定现场团队的功能正常确认，也不能据此宣布问题已闭环。
- 这两组没有同一固定输入的板端张量与 Golden 对比，也没有配对检测框精度对照。后续[独立板端固定输入测试](preproc_board_golden_20261029_zh.md)逐位通过，但不能代替该 A/B 记录的逐帧配对比较，且独立 BIT 哈希未核实。
- 如需量化这两套配置之间的精度差异，还需要相同输入的检测框比较；当前记录可以如实报告两套实际配置的**计时**，不能替代该精度验证。
~~~~

**After**

~~~~text
- 两组各 35 条 `PREPROC backend` 与 `TIME` 一一对应，无 `PREPROC_FALLBACK`。两份日志的 35 条 `SNAPSHOT` 均为 `content_changed=1`，但也均为 `freshness=unverified`，帧完成与新鲜度仍需硬件完成事件核验。
- 三份 VDMA 状态记录相同，均提示 `latched VDMA errors`；原记录保留未清除。现场功能确认与该状态一并归档；错误来源及帧新鲜度仍待专项核验。
- 这两组没有同一固定输入的板端张量与 Golden 对比，也没有配对检测框精度对照。后续[独立板端固定输入测试](preproc_board_golden_20261029_zh.md)逐位通过，其独立 BIT 哈希待核对，实时 A/B 记录的逐帧配对比较仍需单独开展。
- 如需量化这两套配置之间的精度差异，还需要相同输入的检测框比较；当前记录提供两套配置的**计时**依据，检测框精度按同输入配对流程评估。
~~~~

#### 原修改 148

原行号：44–44；当时修改后行号：44–44。

位置：Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28） / 复核方式

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
重算输出已保存为 [`recomputed.json`](../board/measurements/preproc_ab_20261028/recomputed.json)。脚本检查后端标记、35 次记录与回退标记，计算丢弃前 5 次后的均值和样本标准差；它不验证 BIT 身份、摄像头输入一致或 FPGA 硬件事务。原始板端脚本作为 `source_board_script.txt` 保留供审查；它会停止运行中的推理进程，不应在稳定系统上直接运行。
~~~~

**After**

~~~~text
重算输出已保存为 [`recomputed.json`](../board/measurements/preproc_ab_20261028/recomputed.json)。脚本检查后端标记、35 次记录与回退标记，计算丢弃前 5 次后的均值和样本标准差；其检查范围为日志字段及统计；BIT 身份、摄像头输入和硬件事务由对应测试核验。原始板端脚本作为 `source_board_script.txt` 保留供审查；执行该脚本会停止推理进程，应限定于独立实验环境。
~~~~

### report/preproc_board_golden_20261029_zh.md

#### 原修改 149

原行号：5–5；当时修改后行号：5–5。

位置：固定输入的 FPGA 预处理板端 Golden 对比（2026-10-29） / 结果与范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
现场回传的[原始归档](../board/measurements/preproc_golden_20261029/source_board_return.zip)记录了一次独立 Z035 板端 FPGA 预处理测试。输入为固定的 640×360 RGB888，输出为小端 FP32、NCHW、`1×3×320×320`：**307,200/307,200 个值与对应双线性缩放及归一化 Golden 的 32 位表示完全一致，差异 0 个，最大和平均绝对误差均为 0**。这是这一次固定输入、独立程序的实板结果，不是 307,200 次独立板测。
~~~~

**After**

~~~~text
现场回传的[原始归档](../board/measurements/preproc_golden_20261029/source_board_return.zip)记录了一次独立 Z035 板端 FPGA 预处理测试。输入为固定的 640×360 RGB888，输出为小端 FP32、NCHW、`1×3×320×320`：**307,200/307,200 个值与对应双线性缩放及归一化 Golden 的 32 位表示完全一致，差异 0 个，最大和平均绝对误差均为 0**。测试覆盖一次固定输入、独立程序运行，比较单位为张量中的 FP32 元素。
~~~~

#### 原修改 150

原行号：15–15；当时修改后行号：15–15。

位置：固定输入的 FPGA 预处理板端 Golden 对比（2026-10-29） / 测试身份与适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 身份、记录及限制
~~~~

**After**

~~~~text
## 测试身份与适用范围
~~~~

#### 原修改 151

原行号：17–18；当时修改后行号：17–18。

位置：固定输入的 FPGA 预处理板端 Golden 对比（2026-10-29） / 测试身份与适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 现场记录板上 BOOT SHA256 为 `a13ae41c8a17f97a4cfdfab61cf462fa3e72eb976f786794755881fe51309e0b`；本次未提取和核实独立 BIT 哈希，不能将它与先前 `b286f480...` BIT 或 `816ebd03...` BOOT 等同。
- 内核版本记录为 `5.15.36-xilinx-v2022.2`；IP 版本寄存器为 `0x00010000`。测试使用独立 `preproc_board_golden_arm`，SHA256 `9043743a6ca54f8417af9e861b62eb29f776b93dbd652d96586336bfe182fa19`；本次未加载 `paddle_person_live` 和 `model.nb`，不证明其现场版本。
~~~~

**After**

~~~~text
- 现场记录板上 BOOT SHA256 为 `a13ae41c8a17f97a4cfdfab61cf462fa3e72eb976f786794755881fe51309e0b`；独立 BIT 哈希及与先前 `b286f480...` BIT、`816ebd03...` BOOT 的关系待核对，版本按本次 BOOT 单独记录。
- 内核版本记录为 `5.15.36-xilinx-v2022.2`；IP 版本寄存器为 `0x00010000`。测试使用独立 `preproc_board_golden_arm`，SHA256 `9043743a6ca54f8417af9e861b62eb29f776b93dbd652d96586336bfe182fa19`；本次运行独立测试程序，`paddle_person_live` 和 `model.nb` 的版本由应用测试记录管理。
~~~~

#### 原修改 152

原行号：20–21；当时修改后行号：20–21。

位置：固定输入的 FPGA 预处理板端 Golden 对比（2026-10-29） / 测试身份与适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 测试说明指出执行前自动推理进程已退出。独立测试程序及源码保留在原始归档；该源码直接使用 `/dev/mem` 和固定地址控制 DMA，不应在正在运行的稳定视频系统中直接复用。
- 本次对比不证明摄像头采集帧的新鲜度、完整视频共存、完整模型检测精度、性能加速比或不同版本 BIT 身份。既有应用日志的 VDMA 错误位、外部 I/O 和 CDC 告警继续保留，不能被本次对比替代。
~~~~

**After**

~~~~text
- 测试说明指出执行前自动推理进程已退出。独立测试程序及源码保留在原始归档；该源码通过 `/dev/mem` 和固定地址控制 DMA，执行环境要求停止自动推理并独占相关资源，稳定视频运行期间保持隔离。
- 本次结论限定为固定输入的预处理张量一致性。摄像头帧新鲜度、完整视频共存、模型检测精度、性能及跨版本 BIT 身份分别评估；既有 VDMA、外部 I/O 和 CDC 状态见[告警说明](warning_review_zh.md)。
~~~~

#### 原修改 153

原行号：31–31；当时修改后行号：31–31。

位置：固定输入的 FPGA 预处理板端 Golden 对比（2026-10-29） / 离线复核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
复核结果：`exact=307200`、`max_abs=0.0`、`mean_abs=0.0`、`over_tolerance=0`、`passed=true`，退出码 0。另以独立 Python 浮点读取和逐字节比较得到一致结果；这些复核读取回传文件，不代表再次上板。
~~~~

**After**

~~~~text
复核结果：`exact=307200`、`max_abs=0.0`、`mean_abs=0.0`、`over_tolerance=0`、`passed=true`，退出码 0。另以独立 Python 浮点读取和逐字节比较得到一致结果；复核对象为归档的板端输出文件，执行位置为主机。
~~~~

### report/readme.md

#### 原修改 154

原行号：13–13；当时修改后行号：13–13。

位置：EagleEye 比赛报告与资料导航 / 报告正文与主要入口

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| [技能包](../skill/index.md) | 可复用工具、使用方法、验证范围及限制。 |
~~~~

**After**

~~~~text
| [技能包](../skill/index.md) | 可复用工具、最小示例、使用方法及适用条件。 |
~~~~

#### 原修改 155

原行号：22–22；当时修改后行号：22–24。

位置：EagleEye 比赛报告与资料导航 / 主要验证和版本资料

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
板端 Golden 是一次独立固定输入测试；CPU/FPGA 计时使用不同缩放算法和实时帧。两类结果的条件和范围分别写在对应记录中，不应混作同一次测试。
~~~~

**After**

~~~~text
项目分别完成了 FPGA 预处理的独立板端 Golden 一致性验证，以及 CPU/FPGA 两种预处理配置的性能测试。各项测试的输入、算法、版本和结果详见对应验证报告。

本次文档表述调整的逐项前后对照见[文档语言修订记录](document_language_revision.md)。
~~~~

#### 原修改 156

原行号：37–37；当时修改后行号：39–39。

位置：EagleEye 比赛报告与资料导航 / 历史记录和补充材料

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
原工程归档和验证记录用于追溯来源。当前设计报告仍是提交整理稿；未核实的授权、旧仿真记录及告警按各自证据范围说明。
~~~~

**After**

~~~~text
项目建立了覆盖源码、构建、仿真、板端测试及版本管理的资料体系，通过工程归档与原始记录支持成果追溯。授权核对、历史仿真版本及工程告警集中在各专题页说明。
~~~~

### report/release_identity_zh.md

#### 原修改 159

原行号：19–19；当时修改后行号：19–19。

位置：当前 PicoDet 演示版本与实测身份 / 已有实测组合

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
上述应用日志和 A/B 测试证明的范围见 [应用证据](board_app_evidence_20261028_zh.md)和 [A/B 计时](preproc_ab_20261028_zh.md)。后续[独立固定输入板端 Golden](preproc_board_golden_20261029_zh.md)逐位通过，但本次 BOOT 哈希与上表不同，独立 BIT 哈希未核实，不能自动认定为上表组合的测试。
~~~~

**After**

~~~~text
上述应用日志和 A/B 测试证明的范围见 [应用证据](board_app_evidence_20261028_zh.md)和 [A/B 计时](preproc_ab_20261028_zh.md)。后续[独立固定输入板端 Golden](preproc_board_golden_20261029_zh.md)逐位通过，但本次 BOOT 哈希与上表不同，独立 BIT 哈希与上表组合的对应关系待核对，按独立测试版本记录。
~~~~

#### 原修改 160

原行号：21–21；当时修改后行号：21–21。

位置：当前 PicoDet 演示版本与实测身份 / 历史与独立候选版本

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
## 其他已有版本，不能混用证据
~~~~

**After**

~~~~text
## 历史与独立候选版本
~~~~

#### 原修改 161

原行号：23–25；当时修改后行号：23–25。

位置：当前 PicoDet 演示版本与实测身份 / 历史与独立候选版本

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 原 GridPolicy 稳定应用归档的可执行文件 SHA256 为 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，源码重编身份见 [`src/ps_as_built/readme_provenance_zh.md`](../src/ps_as_built/readme_provenance_zh.md)。原资料是该历史版本记录，不代表它是本次 A/B 测试程序。
- 团队反馈 AX7020 早期交付为 JPEG 测试图程序，较新的交接版本使用 OV5640 摄像头与 VDMA；现有相关日志为 QEMU 离线记录，不能作为 AX7020 实板演示证据。这与上表现场已确认的 Z035 板端功能是不同平台和记录，不能互相替代。
- 独立 PicoDet 注意力候选的本地交叉编译 ELF SHA256 为 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；现场已确认该候选上板功能正常。AttentionPolicy 的原始日志对应上表的另一个 ELF，不能混作注意力候选原始日志，见 [`src/picodet_c_candidate/readme.md`](../src/picodet_c_candidate/readme.md)。
~~~~

**After**

~~~~text
- 原 GridPolicy 稳定应用归档的可执行文件 SHA256 为 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，源码重编身份见 [`src/ps_as_built/readme_provenance_zh.md`](../src/ps_as_built/readme_provenance_zh.md)。原资料记录该历史版本，本次 A/B 使用上表 AttentionPolicy 应用。
- 团队反馈 AX7020 早期交付为 JPEG 测试图程序，较新的交接版本使用 OV5640 摄像头与 VDMA；现有相关日志覆盖 QEMU 离线推理；AX7020 实板结果待验证。平台与证据按 AX7020、Z035 分别管理。
- 独立 PicoDet 注意力候选的本地交叉编译 ELF SHA256 为 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；现场已确认该候选上板功能正常。AttentionPolicy 的原始日志对应上表的另一个 ELF，按 AttentionPolicy 单独归档，见 [`src/picodet_c_candidate/readme.md`](../src/picodet_c_candidate/readme.md)。
~~~~

#### 原修改 162

原行号：28–28；当时修改后行号：28–28。

位置：当前 PicoDet 演示版本与实测身份 / 历史与独立候选版本

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
**报告口径：**主实测表引用 AttentionPolicy A/B 组合；PicoDet 注意力候选引用现场功能确认并独立标识。若最终提交演示选择的是其他应用或镜像，更新版本表和相应真实证据，不将不同应用的日志拼成一次运行。
~~~~

**After**

~~~~text
**版本使用方法：**主实测表引用 AttentionPolicy A/B 组合，独立 PicoDet 注意力候选引用对应现场功能确认。演示版本变化时，同步更新应用、镜像和测试记录，以同一版本关系引用结果。
~~~~

### report/reproduction_zh.md

#### 原修改 163

原行号：1–1；当时修改后行号：1–1。

位置：干净目录复现指南

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
# 干净目录复现与证据边界
~~~~

**After**

~~~~text
# 干净目录复现指南
~~~~

#### 原修改 164

原行号：20–20；当时修改后行号：20–20。

位置：干净目录复现指南 / 1. 获取与验证

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
源码包约 240 MiB，若没有 Git LFS 或未拉取对象，源码路径可能只有 LFS 指针；核验脚本会拒绝。`verify_submission.py` 还会核对原始 Eyes 快照、各模块与 Linux 参考快照及清单。哈希不是签名或板端运行证明。
~~~~

**After**

~~~~text
源码包约 240 MiB，若没有 Git LFS 或未拉取对象，源码路径可能只有 LFS 指针；核验脚本会拒绝。`verify_submission.py` 还会核对原始 Eyes 快照、各模块与 Linux 参考快照及清单。文件完整性、来源认证及板端运行分别使用对应验证方式。
~~~~

#### 原修改 165

原行号：32–32；当时修改后行号：32–32。

位置：干净目录复现指南 / 2. 恢复大小写并构建 Vivado 工程

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
先执行 `git lfs pull`。恢复工具将原始快照与补充材料合并并核验 275 项哈希；`build_full_vivado.tcl` 在独立输出目录进行 BD 校验、综合、实现、BIT/XSA 和资源/时序/CDC/DRC 报告。最后的校验脚本确认 XSA 内 BIT 与独立 BIT 一致，并输出 BIT/XSA/HWH SHA256。实际运行以本机 Vivado 2022.2 与许可证为准。上述完整入口已在本机从还原的工程快照实际完成 BD、综合、实现和 BIT/XSA；报告与哈希见 `build/full_rebuild_20261027/`。此前独立构建报告保存在 `build/independent_rebuild_20261027/`。BIT 生成不等于板端验证；缺失 I/O 延迟和 CDC Critical 项仍需检查。
~~~~

**After**

~~~~text
先执行 `git lfs pull`。恢复工具将原始快照与补充材料合并并核验 275 项哈希；`build_full_vivado.tcl` 在独立输出目录进行 BD 校验、综合、实现、BIT/XSA 和资源/时序/CDC/DRC 报告。最后的校验脚本确认 XSA 内 BIT 与独立 BIT 一致，并输出 BIT/XSA/HWH SHA256。实际运行以本机 Vivado 2022.2 与许可证为准。上述完整入口已在本机从还原的工程快照实际完成 BD、综合、实现和 BIT/XSA；报告与哈希见 `build/full_rebuild_20261027/`。此前独立构建报告保存在 `build/independent_rebuild_20261027/`。板端功能与 BIT 身份见[上板记录](board_reproduced_20261028_zh.md)，I/O 延迟和 CDC Critical 的处理状态见[告警说明](warning_review_zh.md)。
~~~~

#### 原修改 166

原行号：46–46；当时修改后行号：46–46。

位置：干净目录复现指南 / 3. 预处理 IP RTL 仿真

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本次实际使用 Windows Python 3.13/NumPy 2.5.2 生成与比较，WSL Ubuntu 20.04 中 Icarus 10.3 执行 RTL，命令和输出在 `sim/preproc_iverilog_20261027.log`。五帧各 307200 个值逐位一致；采集帧的输入适配器加核心亦一致。此为仿真，不是板端 DMA/整机结果。
~~~~

**After**

~~~~text
本次实际使用 Windows Python 3.13/NumPy 2.5.2 生成与比较，WSL Ubuntu 20.04 中 Icarus 10.3 执行 RTL，命令和输出在 `sim/preproc_iverilog_20261027.log`。五帧各 307200 个值逐位一致；采集帧的输入适配器加核心亦一致。该项覆盖 RTL 数值与流接口，板端 DMA 及系统结果见对应实板记录。
~~~~

#### 原修改 167

原行号：55–55；当时修改后行号：55–55。

位置：干净目录复现指南 / 3. 预处理 IP RTL 仿真

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
实际输出及工具路径见 `sim/servo_iverilog_20261027.log`；仿真通过不证明实物舵机限位、供电或机械安全。
~~~~

**After**

~~~~text
实际输出及工具路径见 `sim/servo_iverilog_20261027.log`；实物限位、供电与机械安全按现场操作条件核验。
~~~~

#### 原修改 168

原行号：67–67；当时修改后行号：67–67。

位置：干净目录复现指南 / 4. Linux 源码和构建输入

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
该补丁仅用于 `eeiso22` 实验内核，不能施用于声称不变的稳定内核。实际构建还需匹配的 PetaLinux 2022.2 交叉工具链、内核配置、设备树/XSA、rootfs 和外部依赖；生成并核对 `kernel.release`、`.config`、`Module.symvers` 与镜像哈希。稳定配置启用 `CONFIG_LOCALVERSION_AUTO`，单纯解压源码并不保证得到原构建版本字符串；必须核验，不能靠版本名推断二进制一致。
~~~~

**After**

~~~~text
该补丁仅用于 `eeiso22` 实验内核，不能施用于声称不变的稳定内核。实际构建还需匹配的 PetaLinux 2022.2 交叉工具链、内核配置、设备树/XSA、rootfs 和外部依赖；生成并核对 `kernel.release`、`.config`、`Module.symvers` 与镜像哈希。稳定配置启用 `CONFIG_LOCALVERSION_AUTO`，单纯解压源码并不保证得到原构建版本字符串；必须核验，二进制对应关系以实际哈希核对。
~~~~

#### 原修改 169

原行号：69–69；当时修改后行号：69–69。

位置：干净目录复现指南 / 4. Linux 源码和构建输入

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
实验配置强制模块签名，签名私钥不在仓库；新生成的密钥不证明与已发布模块或板端信任链匹配。已在本机独立目录用稳定配置完成上游内核 zImage 和 172 个模块构建；新 zImage 与历史镜像哈希不同，原因尚未确定。实验补丁只进行了源码核验；没有证明实验模块签名或板端启动。
~~~~

**After**

~~~~text
实验配置强制模块签名，签名私钥不在仓库；签名密钥及信任链需与目标内核匹配。已在本机独立目录用稳定配置完成上游内核 zImage 和 172 个模块构建；新 zImage 与历史镜像哈希不同，原因尚未确定。实验补丁只进行了源码核验；没有证明实验模块签名或板端启动。
~~~~

### report/submission_audit.md

#### 原修改 170

原行号：3–3；当时修改后行号：3–3。

位置：竞赛提交材料核查与缺口（2026-10-27）

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
依据已提供的比赛 3.3.5 截图检查；不代替组委会对完整规则的最终审核。
~~~~

**After**

~~~~text
本页保存比赛 3.3.5 材料的阶段核查表。当前提交导航见[比赛材料与证据索引](contest_evidence_index_zh.md)，以下各项按对应工程和测试日期引用。
~~~~

#### 原修改 172

原行号：9–9；当时修改后行号：9–9。

位置：竞赛提交材料核查与缺口（2026-10-27）

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
| 硬件实现 | 工程快照使用完整构建入口完成综合、实现、BIT 和 XSA，报告见 `build/full_rebuild_20261027/` | 12 项输入和 33 项输出延迟缺失；8 项 Methodology Critical Warning 和 CDC Critical 分类尚需核查。已约束时序通过不等于板端安全或功能通过 |
~~~~

**After**

~~~~text
| 硬件实现 | 工程快照使用完整构建入口完成综合、实现、BIT 和 XSA，报告见 `build/full_rebuild_20261027/` | 12 项输入和 33 项输出延迟缺失；8 项 Methodology Critical Warning 和 CDC Critical 分类尚需核查。已约束 STA、板端功能和专项时序签核分别记录 |
~~~~

### report/submission_check_20261028_zh.md

#### 原修改 173

原行号：3–3；当时修改后行号：3–3。

位置：比赛提交材料检查（2026-10-28）

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
按已提供的 3.3.5 截图核对，最终格式和评分以赛事组织方完整规则为准。本文件索引已有结果，不构造新的上板结论。
~~~~

**After**

~~~~text
按已提供的 3.3.5 截图核对，最终格式和评分以赛事组织方完整规则为准。本文件将各项要求与已有实现、验证记录对应。
~~~~

#### 原修改 174

原行号：14–15；当时修改后行号：14–15。

位置：比赛提交材料检查（2026-10-28）

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| Skill | [Skill 索引](../skill/index.md)：FP32 比较、ROI 接口检查、PYNQ 示例 | PYNQ 示例仅模拟对象验证；团队确认无 PYNQ 实板及真实跨项目复用资料，按模拟候选提交，不列为需要队友补交的既有材料，也不宣称获得加分。 |
| 设计报告和 AI 协作 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)及图示、参考资料 | 团队主要分工已按团队确认记录；仍需核对具体贡献与引用，AI 记录为整理稿而非完整逐字对话。 |
~~~~

**After**

~~~~text
| Skill | [Skill 索引](../skill/index.md)：FP32 比较、ROI 接口检查、PYNQ 示例 | PYNQ 示例已完成模拟对象验证，以离线参考方案提交，实板与真实跨项目复用留待后续验证。 |
| 设计报告和 AI 协作 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)及图示、参考资料 | 团队主要职责及引用来源已整理；AI 记录提供工程案例、真实会话节选和验证路径。 |
~~~~

#### 原修改 175

原行号：30–30；当时修改后行号：30–30。

位置：比赛提交材料检查（2026-10-28） / 本地可重复检查

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
链接检查只检查报告、仿真和 Skill 的 Markdown 行内本地文件路径，不检查网页可用性、段落锚点、代码块或所有脚本内部路径。文件哈希与主机测试不等于实板验证。
~~~~

**After**

~~~~text
链接检查只检查报告、仿真和 Skill 的 Markdown 行内本地文件路径，不检查网页可用性、段落锚点、代码块或所有脚本内部路径。文件完整性、工具行为和板端功能分别对应各自验证记录。
~~~~

#### 原修改 176

原行号：32–32；当时修改后行号：32–32。

位置：比赛提交材料检查（2026-10-28） / 本地可重复检查

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
现场已确认六项功能正常，且 BIT、BOOT、启动串口、应用日志及独立固定输入板端 Golden 已收到，不再要求重复提供。该 Golden 不能证明 A/B 计时两组使用同算法或相同帧，也不能代替视频帧新鲜度验证。
~~~~

**After**

~~~~text
现场六项功能确认、BIT、BOOT、启动串口、应用日志和独立固定输入 Golden 均已归档。Golden 用于固定输入张量一致性，A/B 日志用于两种实际配置计时；同算法配对及帧新鲜度按专项评估。
~~~~

### report/warning_review_zh.md

#### 原修改 181

原行号：3–3；当时修改后行号：3–3。

位置：本次复现工程告警说明

原因：C：将告警或授权状态集中为专题说明，保留未闭环事实和影响

**Before**

~~~~text
依据 2026-10-28 干净克隆并在布线中断后从检查点续跑生成的 [Vivado 2022.2 报告](../build/clean_rebuild_20261028/reports/)，不改写 RTL 或约束。现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能上板正常；以下是工具和日志的独立记录，不否定这一确认，也不能用现场正常直接替代时序签核。
~~~~

**After**

~~~~text
本页汇总 2026-10-28 干净克隆并从布线检查点续跑得到的 [Vivado 2022.2 报告](../build/clean_rebuild_20261028/reports/)及应用日志中的工程注意事项。现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能正常；板级时序与帧完整性签核仍按以下专项状态管理。
~~~~

#### 原修改 182

原行号：7–11；当时修改后行号：7–11。

位置：本次复现工程告警说明

原因：C：将告警或授权状态集中为专题说明，保留未闭环事实和影响

**Before**

~~~~text
| Methodology 8 项 Critical | [`methodology.rpt`](../build/clean_rebuild_20261028/reports/methodology.rpt)：`TIMING-4` 1 项、`TIMING-6` 3 项、`TIMING-7` 3 项、`TIMING-27` 1 项。报告称 `aclk` 在 `clk_fpga_0` 时钟树下被重新定义，并指出 `aclk`/`clk_fpga_0`、`ov5640_pclk`/`clk_fpga_1` 的关联时钟问题。 | 时钟约束和时序关系尚需根据完整时钟树、时钟域设计和工具路径报告复核。此处未修改 XDC，也不能由 WNS 为正推断问题消失。 |
| CDC Critical | [`cdc.rpt`](../build/clean_rebuild_20261028/reports/cdc.rpt) 中 `clk_fpga_0→aclk`、`aclk→clk_fpga_0` 和 `ov5640_pclk→clk_fpga_1` 列为 Critical；报告各自还包含 safe、unsafe、unknown 细分。 | 需要分别审查约束建模和跨域实现；不能把汇总行数直接当作实际失效次数，也不能因有同步器就认定所有 crossing 均安全。 |
| 外部 I/O 时序 | `timing_summary.rpt` 列出 12 项输入和 33 项输出缺少延迟约束；Methodology 的 `TIMING-18` 明细包含摄像头数据/同步、按键、复位及舵机输出等。 | 需区分同步外设接口与异步控制信号，并以传感器/显示器件时序、板级走线与实际时钟为依据；不猜填 delay，也不把这 45 项全部自动归为同一问题。 |
| 其他 DRC/方法学 | [`drc.rpt`](../build/clean_rebuild_20261028/reports/drc.rpt) 有 65 Warning、4 Advisory；`methodology.rpt` 另有复位、同步链和约束查询等 Warning。 | 类别和数量是对应构建的工具输出，尚未逐项完成设计审查；不能仅按告警数目判断板端功能是否正常。 |
| VDMA 应用日志提示 | [应用日志](../board/reproduced_20261028/attention_app_20261028.log) 记录 `s2mm=0x15810`、`mm2s=0x11000`、`cleared=0`，并打印 `WARNING VDMA error bits present`；179 次快照标为 `freshness=unverified`。 | 应用据源码掩码 `0x00000ff0` 检查错误位；日志不能单独确定产生原因、发生时间或对已确认功能的实际影响。它不是板端张量 Golden 或帧完成证明。 |
~~~~

**After**

~~~~text
| Methodology 8 项 Critical | [`methodology.rpt`](../build/clean_rebuild_20261028/reports/methodology.rpt)：`TIMING-4` 1 项、`TIMING-6` 3 项、`TIMING-7` 3 项、`TIMING-27` 1 项。报告称 `aclk` 在 `clk_fpga_0` 时钟树下被重新定义，并指出 `aclk`/`clk_fpga_0`、`ov5640_pclk`/`clk_fpga_1` 的关联时钟问题。 | 时钟约束和时序关系尚需根据完整时钟树、时钟域设计和工具路径报告复核。当前 XDC 保持原样，专项签核待完成；WNS 的适用范围为已约束路径。 |
| CDC Critical | [`cdc.rpt`](../build/clean_rebuild_20261028/reports/cdc.rpt) 中 `clk_fpga_0→aclk`、`aclk→clk_fpga_0` 和 `ov5640_pclk→clk_fpga_1` 列为 Critical；报告各自还包含 safe、unsafe、unknown 细分。 | 需要分别审查约束建模和跨域实现；汇总分类用于定位 crossing，实际影响需逐路径核对同步结构和约束。 |
| 外部 I/O 时序 | `timing_summary.rpt` 列出 12 项输入和 33 项输出缺少延迟约束；Methodology 的 `TIMING-18` 明细包含摄像头数据/同步、按键、复位及舵机输出等。 | 需区分同步外设接口与异步控制信号，并以传感器/显示器件时序、板级走线与实际时钟为依据；这 45 项按接口语义分别核对，延迟值以器件和板级资料为依据。 |
| 其他 DRC/方法学 | [`drc.rpt`](../build/clean_rebuild_20261028/reports/drc.rpt) 有 65 Warning、4 Advisory；`methodology.rpt` 另有复位、同步链和约束查询等 Warning。 | 类别和数量是对应构建的工具输出，尚未逐项完成设计审查；板端功能按现场测试记录，告警影响按具体类型及路径评估。 |
| VDMA 应用日志提示 | [应用日志](../board/reproduced_20261028/attention_app_20261028.log) 记录 `s2mm=0x15810`、`mm2s=0x11000`、`cleared=0`，并打印 `WARNING VDMA error bits present`；179 次快照标为 `freshness=unverified`。 | 应用据源码掩码 `0x00000ff0` 检查错误位；产生原因、时间及对功能的实际影响仍待定位；帧完成与张量 Golden 由专项测试核验。 |
~~~~

#### 原修改 183

原行号：13–13；当时修改后行号：13–13。

位置：本次复现工程告警说明

原因：C：将告警或授权状态集中为专题说明，保留未闭环事实和影响

**Before**

~~~~text
上述问题是当前资料中的技术边界。若后续需要修改或签核，需使用匹配版本工程、板级资料及受控实测；本次仅整理已有证据，不声称问题已修复。
~~~~

**After**

~~~~text
上述项目作为已知工程问题保留，修复或签核需匹配版本源码、板级时序资料及受控实测。本次文档整理保持现有 RTL/XDC 和告警处理状态。
~~~~

#### 原修改 184

原行号：15–15；当时修改后行号：15–15。

位置：本次复现工程告警说明

原因：C：将告警或授权状态集中为专题说明，保留未闭环事实和影响

**Before**

~~~~text
团队后续确认：已有外部 I/O 延迟约束缺口记录，但没有板级 I/O 时序签核；CDC 和其他时钟告警也没有完成处理与签核的记录。VDMA 错误位在既有板端记录及 A/B 测速中有提示，测速时未清除；其原因和每次读取均为最新完整帧尚未证明。这是现场反馈及已有记录的范围，不改变 现场功能正常的确认。
~~~~

**After**

~~~~text
团队确认外部 I/O、CDC 和其他时钟告警仍待处理与签核；VDMA 错误位在测速时保持锁存，错误来源及每次读取的最新完整帧属性待验证。现场六项功能正常与这些专项状态分别记录。
~~~~

### sim/lanczos_legacy_readme.md

#### 原修改 185

原行号：7–7；当时修改后行号：7–7。

位置：Lanczos 旧仿真记录（非独立 Skill）

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
当前工程仍使用 Lanczos，原有 RTL 保存在 `src/vivado/`。`python tools/check_lanczos.py` 可核对七份工程 RTL 与原始快照中的来源，但它不验证像素计算。
~~~~

**After**

~~~~text
当前工程仍使用 Lanczos，原有 RTL 保存在 `src/vivado/`。`python tools/check_lanczos.py` 可核对七份工程 RTL 与原始快照中的来源，用于来源一致性核验，像素计算按下列仿真记录评估。
~~~~

#### 原修改 186

原行号：16–16；当时修改后行号：16–16。

位置：Lanczos 旧仿真记录（非独立 Skill） / 历史独立工程的通过记录

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
两份文件保留来件原始字节；测试工程源码及完整环境未随本次材料提供，因此这里引用的是日志记录，未重新运行仿真。来件说明这些测试工程与当前 Eyes 中部分 RTL 的哈希不同，但本次缺少其完整源码，无法独立核对两套工程的全部版本关系；**不能将这两份 PASS 直接归为当前 Eyes RTL 的逐像素测试结果**。
~~~~

**After**

~~~~text
两份日志保留原始字节，覆盖历史独立测试工程。对应完整源码和环境未随材料归档，因此本页引用其日志结果。来件记录部分 RTL 与当前 Eyes 的哈希不同，两套工程的完整版本关系仍待核对，当前 Eyes 的同版本逐像素验证按专项测试处理。
~~~~

#### 原修改 187

原行号：18–18；当时修改后行号：18–18。

位置：Lanczos 旧仿真记录（非独立 Skill） / 历史独立工程的通过记录

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
原有 [XSim 失败日志](lanczos_legacy_xsim.txt)仍然保留：其中 1,536 个输出像素有 768 处错误。该记录与新增 PASS 日志的测试版本及参考条件未建立完整对应，不能把不同日志当成同一次测试。
~~~~

**After**

~~~~text
原有 [XSim 失败日志](lanczos_legacy_xsim.txt)仍然保留：其中 1,536 个输出像素有 768 处错误。该记录与新增 PASS 日志的测试版本及参考条件未建立完整对应，各份日志按独立测试版本保存。
~~~~

### sim/preproc_captured_dma_20261027/readme.md

#### 原修改 188

原行号：3–3；当时修改后行号：3–3。

位置：采集帧预处理仿真输出

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
`captured_dma_r.hex`、`captured_dma_g.hex`、`captured_dma_b.hex` 是 2026-10-27 Icarus 适配器加预处理核心仿真的原始三路输出，每路 102,400 个 32 位字；来源和原始运行命令见 [仿真日志](../preproc_iverilog_20261027.log)。本地使用的 `captured.rgb` 和 `captured.golden.f32` 哈希与仓库 `data/` 下文件一致。该数据不是板端采集输出。
~~~~

**After**

~~~~text
`captured_dma_r.hex`、`captured_dma_g.hex`、`captured_dma_b.hex` 是 2026-10-27 Icarus 适配器加预处理核心仿真的原始三路输出，每路 102,400 个 32 位字；来源和原始运行命令见 [仿真日志](../preproc_iverilog_20261027.log)。本地使用的 `captured.rgb` 和 `captured.golden.f32` 哈希与仓库 `data/` 下文件一致。该目录记录 Icarus 仿真输出，板端输出另见[固定输入 Golden](../../report/preproc_board_golden_20261029_zh.md)。
~~~~

### sim/verification_zh.md

#### 原修改 189

原行号：3–3；当时修改后行号：3–3。

位置：仿真与验证说明

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
本说明索引已保存的结果，不把主机、RTL 或 QEMU 测试写成开发板实测。原工程自带的 [sim/readme.md](readme.md) 是原始快照，描述的是当时的材料状态；后续的验证证据列于本页。
~~~~

**After**

~~~~text
本页按主机工具、RTL、QEMU 与板端四类整理验证结果，连接测试输入、原始输出和复现方法。原工程 [sim/readme.md](readme.md)保留历史快照状态，后续结果以本页索引查阅。
~~~~

#### 原修改 190

原行号：11–11；当时修改后行号：11–11。

位置：仿真与验证说明 / 测试输入和身份

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
| `data/model.nb` | `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4` | PicoDet 模型；哈希不证明它在现场被加载 |
~~~~

**After**

~~~~text
| `data/model.nb` | `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4` | PicoDet 模型文件身份；现场加载版本见应用测试记录 |
~~~~

#### 原修改 191

原行号：17–17；当时修改后行号：17–17。

位置：仿真与验证说明 / 预处理 RTL：Icarus 离线仿真

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
现补充本地保留的[采集帧三路仿真输出](preproc_captured_dma_20261027/)，分别为 R/G/B 32 位十六进制字，共 307,200 个值。输入和 Golden 与上述仓库文件哈希一致。运行 `python -B tools/verify_preproc_sim_capture.py` 可将这三路记录逐位对照仓库 Golden；此数据出自 Icarus 仿真，**不是板端 DMA 输出**。
~~~~

**After**

~~~~text
现补充本地保留的[采集帧三路仿真输出](preproc_captured_dma_20261027/)，分别为 R/G/B 32 位十六进制字，共 307,200 个值。输入和 Golden 与上述仓库文件哈希一致。运行 `python -B tools/verify_preproc_sim_capture.py` 可将这三路记录逐位对照仓库 Golden；该数据来源为 Icarus 适配器与核心仿真，板端张量输出见独立 Golden 记录。
~~~~

#### 原修改 192

原行号：19–19；当时修改后行号：19–19。

位置：仿真与验证说明 / 预处理 RTL：Icarus 离线仿真

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
日志中核心 `673804 cycles` 和适配器加核心 `788884 cycles` 是测试平台的仿真周期；按 100 MHz 换算的时长不是板端 DMA、缓存和完整预处理延迟。
~~~~

**After**

~~~~text
日志中核心 `673804 cycles` 和适配器加核心 `788884 cycles` 按测试平台的仿真周期记录，100 MHz 为换算条件；板端 DMA/cache 及完整预处理延迟由实测计时。
~~~~

#### 原修改 193

原行号：25–28；当时修改后行号：25–28。

位置：仿真与验证说明 / 舵机与 ROI 控制

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
- [舵机 RTL 仿真日志](servo_iverilog_20261027.log)：Icarus Verilog 10.3，中心保持、20 ms 周期、10 µs 步进和 ROI 反向用例通过。该仿真不证明板端机械限位、供电、舵机方向或现场操作。
- [ROI 合同检查器测试](host_roi.log)：主机 1 项通过；检查器测试不证明鼠标、按键、暂停画面或舵机在板上运行。
- [预处理比较器测试](host_preproc.log)：主机 2 项通过，覆盖相等、差异与长度错误；不是板端张量对照。2026-10-28 复核见 [测试记录](host_skill_tests_20261028.txt)。
- 后续补充的两项 Skill 独立副本正反例、空配置和损坏归档测试见 [示例验证记录](skill_examples_20261028.txt)；XSA 文件检查也不能代表板端运行。
~~~~

**After**

~~~~text
- [舵机 RTL 仿真日志](servo_iverilog_20261027.log)：Icarus Verilog 10.3，中心保持、20 ms 周期、10 µs 步进和 ROI 反向用例通过。该项覆盖 RTL 行为，板端机械限位、供电和方向按现场条件核验。
- [ROI 合同检查器测试](host_roi.log)：主机 1 项通过；测试范围为接口合同检查器，板端功能见现场确认记录。
- [预处理比较器测试](host_preproc.log)：主机 2 项通过，覆盖相等、差异与长度错误；该项覆盖主机比较器。2026-10-28 复核见 [测试记录](host_skill_tests_20261028.txt)。
- 后续补充的两项 Skill 独立副本正反例、空配置和损坏归档测试见 [示例验证记录](skill_examples_20261028.txt)；XSA 检查覆盖文件元数据，运行行为由板端测试记录。
~~~~

#### 原修改 194

原行号：39–39；当时修改后行号：39–39。

位置：仿真与验证说明 / PicoDet 与注意力独立候选

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
[主机测试摘要](picodet_c_candidate/host_results.txt)记录临时 ID、无目标、错误输入以及模拟 Hold 行为。ARM 用户态 QEMU 使用离线输入和 CPU 预处理：[人体框与注意力请求摘要](picodet_c_candidate/qemu_predictor.txt)和[无目标摘要](picodet_c_candidate/qemu_empty.txt)均标明测试范围。这里的请求不是 Fovea 硬件完成信号；临时 ID 不保证遮挡后稳定。构建入口和边界见 [候选说明](../src/picodet_c_candidate/readme.md)。
~~~~

**After**

~~~~text
[主机测试摘要](picodet_c_candidate/host_results.txt)记录临时 ID、无目标、错误输入以及模拟 Hold 行为。ARM 用户态 QEMU 使用离线输入和 CPU 预处理：[人体框与注意力请求摘要](picodet_c_candidate/qemu_predictor.txt)和[无目标摘要](picodet_c_candidate/qemu_empty.txt)均标明测试范围。这里记录软件请求；Fovea 完成状态由硬件事件核验，临时 ID 用于相邻结果关联。构建入口和边界见 [候选说明](../src/picodet_c_candidate/readme.md)。
~~~~

#### 原修改 195

原行号：41–41；当时修改后行号：41–41。

位置：仿真与验证说明 / Lanczos 历史仿真与版本范围

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
## 旧 Lanczos 仿真及仍缺的验证
~~~~

**After**

~~~~text
## Lanczos 历史仿真与版本范围
~~~~

#### 原修改 196

原行号：43–43；当时修改后行号：43–43。

位置：仿真与验证说明 / Lanczos 历史仿真与版本范围

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
[旧 XSim 原始记录](lanczos_legacy_xsim.txt)报告 1536 个输出像素中 768 处错误；旧测试台与当前 RTL 的版本、参考算法对应关系尚未确认。既不能声称当前 RTL 逐像素通过，也不能单凭旧日志断言当前实板像素错误。工程源码和记录保留，独立 Lanczos Skill 不再展示；详见 [说明](lanczos_legacy_readme.md)。
~~~~

**After**

~~~~text
[旧 XSim 原始记录](lanczos_legacy_xsim.txt)报告 1536 个输出像素中 768 处错误；旧测试台与当前 RTL 的版本、参考算法对应关系尚未确认。该日志按历史测试版本保存，当前 RTL 的逐像素状态需匹配版本后核验。工程源码和记录保留，独立 Lanczos Skill 不再展示；详见 [说明](lanczos_legacy_readme.md)。
~~~~

#### 原修改 197

原行号：45–45；当时修改后行号：45–45。

位置：仿真与验证说明 / Lanczos 历史仿真与版本范围

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
另有历史独立测试工程的 [单通道](lanczos_single_xsim_pass_legacy.txt)和 [RGB](lanczos_rgb_xsim_pass_legacy.txt) XSim 日志，分别记录 768 和 1,536 个输出像素零错误。它们的测试工程源码未随本次材料提供，不能将结果直接套用于当前 Eyes RTL，也不能与上述失败日志混为同一次测试。[测试范围与来源](lanczos_legacy_readme.md)
~~~~

**After**

~~~~text
另有历史独立测试工程的 [单通道](lanczos_single_xsim_pass_legacy.txt)和 [RGB](lanczos_rgb_xsim_pass_legacy.txt) XSim 日志，分别记录 768 和 1,536 个输出像素零错误。它们的测试工程源码未随本次材料提供，其与当前 Eyes RTL 的版本对应待核对，通过及失败日志分别保存。[测试范围与来源](lanczos_legacy_readme.md)
~~~~

#### 原修改 198

原行号：51–51；当时修改后行号：51–51。

位置：仿真与验证说明 / 2026-10-28 脱离工程复用补充

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
两项 Skill 分别复制到临时空目录，使用另一种张量形状及不同模块/地址的合成 HWH 运行正反例、长度错误和单元测试，命令及输出见 [复用测试记录](skill_portability_20261028.txt)。这些不是其他真实项目或 PYNQ 板端证据。
~~~~

**After**

~~~~text
两项 Skill 分别复制到临时空目录，使用另一种张量形状及不同模块/地址的合成 HWH 运行正反例、长度错误和单元测试，命令及输出见 [复用测试记录](skill_portability_20261028.txt)。该项验证独立目录及合成契约的可移植性，其他真实项目与 PYNQ 实板留待迁移测试。
~~~~

#### 原修改 199

原行号：55–55；当时修改后行号：55–55。

位置：仿真与验证说明 / PYNQ 示例的模拟验证

原因：A/B：以成果和方法为主线精简重复否定，保留案例事实和适用条件

**Before**

~~~~text
独立 PYNQ Skill 在空目录通过模拟对象测试；身份校验、下载许可、寄存器白名单、DMA 调用顺序、异常及长度检查的命令与输出见 [模拟记录](pynq_skill_mock_20261028.txt)。未加载 Overlay，未使用真实 DMA 或 PYNQ 板卡，不能视为板端结果。
~~~~

**After**

~~~~text
独立 PYNQ Skill 在空目录通过模拟对象测试；身份校验、下载许可、寄存器白名单、DMA 调用顺序、异常及长度检查的命令与输出见 [模拟记录](pynq_skill_mock_20261028.txt)。该项以 PC 模拟对象覆盖控制流，Overlay 加载及真实 DMA/PYNQ 硬件行为留待板端验证。
~~~~

### skill/01-axi-video-regression/skill.md

#### 原修改 200

原行号：28–28；当时修改后行号：28–28。

位置：AXI 视频流与 RGB 锁步回归 / 已验证效果与失败来源

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
[E01](../evidence.md#e01视频流的正确性不能只看静态画面)：已有单通道和 RGB 的小帧、连续帧、目标行宽回归，检查数最高 20480 像素且零错误。经验来自旧坐标总线宽度错误、RGB 各核独立前进风险和行缓存/窗口边界核对。历史数据不替代新项目回归。
~~~~

**After**

~~~~text
[E01](../evidence.md#e01视频流的正确性不能只看静态画面)：已有单通道和 RGB 的小帧、连续帧、目标行宽回归，检查数最高 20480 像素且零错误。经验来自旧坐标总线宽度错误、RGB 各核独立前进风险和行缓存/窗口边界核对。迁移后按新项目输入、时钟和接口重新执行回归。
~~~~

### skill/02-tensor-preprocess-golden/skill.md

#### 原修改 201

原行号：19–20；当时修改后行号：19–20。

位置：预处理张量的完整数值验收 / 工作流

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
6. 比较替换前后的模型结果时，用同一原始帧、权重、阈值和 NMS；在源坐标系按类别/IoU 匹配框，报告新增、消失、坐标与分数变化。预处理数值通过不代表检测行为不变。
7. 分别记录核周期、流适配器周期、包含 cache/DMA/DDR 的端到端耗时。DDR 字节量只用于带宽预算，不能直接当测得时延。
~~~~

**After**

~~~~text
6. 比较替换前后的模型结果时，用同一原始帧、权重、阈值和 NMS；在源坐标系按类别/IoU 匹配框，报告新增、消失、坐标与分数变化。预处理张量与下游检测行为分别验收。
7. 分别记录核周期、流适配器周期、包含 cache/DMA/DDR 的端到端耗时。DDR 字节量用于带宽预算，实际时延由包含传输的计时获得。
~~~~

### skill/03-armv7-inference-deploy/skill.md

#### 原修改 202

原行号：10–10；当时修改后行号：10–10。

位置：Cortex-A9 推理移植与 ABI 验收 / 适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
目标为 ARM32 Linux CPU 推理，典型平台为 Zynq-7000 Cortex-A9。不要从板名推断 aarch64、驱动或加速后端。模型训练、PL 卷积迁移和实时摄像头链路分别验收。
~~~~

**After**

~~~~text
目标为 ARM32 Linux CPU 推理，典型平台为 Zynq-7000 Cortex-A9。根据 SoC 和实际系统资料确认指令集、驱动及后端。模型训练、PL 卷积迁移和实时摄像头链路分别验收。
~~~~

#### 原修改 203

原行号：31–31；当时修改后行号：31–31。

位置：Cortex-A9 推理移植与 ABI 验收 / 使用方法

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
最后一行参数必须查实际程序帮助，不能把示意参数直接当作可执行命令。
~~~~

**After**

~~~~text
最后一行按实际程序帮助替换单图参数后执行。
~~~~

### skill/04-petalinux-xsa-rebuild/skill.md

#### 原修改 204

原行号：38–38；当时修改后行号：38–38。

位置：硬件导出到系统镜像的可复现构建 / 已验证效果与失败来源

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
[E04](../evidence.md#e04镜像重建前先识别工具代际)：XSA 为 2022.2，而原课程环境为 2017.4；本机还曾打开不同 Ubuntu 版本且无 PetaLinux。既有记录据此明确阻塞阶段，未伪造镜像产物。本技能抽取的是预检与绑定方法，不声称这些历史镜像已构建成功。
~~~~

**After**

~~~~text
[E04](../evidence.md#e04镜像重建前先识别工具代际)：XSA 为 2022.2，而原课程环境为 2017.4；本机还曾打开不同 Ubuntu 版本且无 PetaLinux。既有记录据此明确阻塞阶段，未伪造镜像产物。本技能提炼环境预检与文件绑定方法，该历史任务的镜像构建仍受上述环境条件限制。
~~~~

### skill/05-vdma-frame-snapshot/skill.md

#### 原修改 205

原行号：10–10；当时修改后行号：10–10。

位置：持续采集下的 DDR 帧快照 / 适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
适用于相机生产帧、显示或推理消费者异步读取同一 DDR 环的系统。先从实际硬件导出确认 VDMA 方向、地址、帧槽、stride、缓存属性及帧完成语义。此流程不授权改动既有硬件。
~~~~

**After**

~~~~text
适用于相机生产帧、显示或推理消费者异步读取同一 DDR 环的系统。先从实际硬件导出确认 VDMA 方向、地址、帧槽、stride、缓存属性及帧完成语义。实际硬件操作按目标系统的权限与恢复流程执行。
~~~~

#### 原修改 206

原行号：32–32；当时修改后行号：32–32。

位置：持续采集下的 DDR 帧快照 / 已验证效果与失败来源

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
[E05](../evidence.md#e05vdma-私有快照保护推理输入)：现有实现完成私有快照及 raw-frame QEMU 推理。真实 VDMA 并发覆盖、最坏复制时间和环回保护没有独立板端证据。本技能保留其设计目标，并明确指针回绕推理的不足，防止把前后检查当成无条件保证。
~~~~

**After**

~~~~text
[E05](../evidence.md#e05vdma-私有快照保护推理输入)：现有实现完成私有快照及 raw-frame QEMU 推理。真实 VDMA 并发覆盖、最坏复制时间和环回保护没有独立板端证据。本技能据此要求用所有权、单调序号或可证明的时间界限补全环回保护，作为迁移后的验收条件。
~~~~

### skill/06-detection-stream-contract/skill.md

#### 原修改 207

原行号：29–29；当时修改后行号：29–29。

位置：检测流坐标与时间契约 / 使用方法

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
校验器检查帧序、计数、字段、范围、越界、同帧重复 ID，以及已观测到的 ID 类别变化。它无法仅由坐标证明跨遮挡身份正确，也不证明接收方实现正确。保留窗口之后 ID 可重新分配时，须更换会话或扩充带 generation 的协议。
~~~~

**After**

~~~~text
校验器检查帧序、计数、字段、范围、越界、同帧重复 ID，以及已观测到的 ID 类别变化。跨遮挡身份及接收方行为还需真值评价与端到端回放。保留窗口之后 ID 可重新分配时，须更换会话或扩充带 generation 的协议。
~~~~

### skill/08-roi-ablation-evaluation/skill.md

#### 原修改 209

原行号：32–32；当时修改后行号：32–32。

位置：ROI 插值与精细识别的配对消融 / 已验证效果与失败来源

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
[E08](../evidence.md#e08负消融结果同样是可复用结论)：400 配对 ROI 上闭集准确率 Nearest 49.00%、Bilinear 53.25%、Lanczos 49.75%，不支持 Lanczos 全面优于双线性。拒识后另一组指标排序不同，小目标子集只有 23 样本。代理 VOC 数据与 OV5640 桌面素材的领域差异也已观察到。
~~~~

**After**

~~~~text
[E08](../evidence.md#e08负消融结果同样是可复用结论)：400 配对 ROI 上闭集准确率 Nearest 49.00%、Bilinear 53.25%、Lanczos 49.75%，该测试中双线性闭集准确率较高，按指标分别评价各插值方案。拒识后另一组指标排序不同，小目标子集只有 23 样本。代理 VOC 数据与 OV5640 桌面素材的领域差异也已观察到。
~~~~

### skill/09-roi-transaction-integration/skill.md

#### 原修改 210

原行号：21–21；当时修改后行号：21–21。

位置：跨模块语义适配与 ROI 事务闭环 / 工作流

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
8. 交接当前版本、适配表、示例消息、运行入口和原始报告，使用 [assets/handoff.md](assets/handoff.md)。消息草稿由用户自行发送，技能本身不授权外发。
~~~~

**After**

~~~~text
8. 交接当前版本、适配表、示例消息、运行入口和原始报告，使用 [assets/handoff.md](assets/handoff.md)。消息由使用者审核后发送。
~~~~

### skill/10-hls-cross-platform-port/skill.md

#### 原修改 211

原行号：33–33；当时修改后行号：33–33。

位置：HLS 算子到 Zynq 系统的迁移审计 / 可直接使用的提示词

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
> 审计这些仓库到目标 Zynq 的迁移工作。先按文件/commit 画清模型、运行时、HLS、驱动和 BD 的职责，再提取算子限制与内存/寄存器契约。运行器件最小预检，建立 CPU/C/RTL/板端数值证据，最后评估并行优化；报告精确阻塞阶段，不将 IP 综合等同于网络部署。
~~~~

**After**

~~~~text
> 审计这些仓库到目标 Zynq 的迁移工作。先按文件/commit 画清模型、运行时、HLS、驱动和 BD 的职责，再提取算子限制与内存/寄存器契约。运行器件最小预检，建立 CPU/C/RTL/板端数值证据，最后评估并行优化；报告器件预检、IP 综合与网络部署各阶段的实际结果。
~~~~

### skill/11-reproducible-evidence-release/skill.md

#### 原修改 212

原行号：10–10；当时修改后行号：10–10。

位置：模型、程序和演示的可复现发布 / 适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
适用于比赛资料整理、队友交接、报告绑定版本和公开仓库准备。此技能完成审计与本地打包；不自动发布、推送、改许可证或向队友发消息。
~~~~

**After**

~~~~text
适用于比赛资料整理、队友交接、报告绑定版本和公开仓库准备。此技能提供版本核对与本地打包流程，发布、推送及对外沟通由使用者另行执行，第三方许可沿用原声明。
~~~~

### skill/evidence.md

#### 原修改 213

原行号：3–3；当时修改后行号：3–3。

位置：经验来源与证据覆盖

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本文件让评审能追溯经验来源。历史聊天中的结论是线索，现存文件能支持的部分优先采用文件证据。原始文件不随本包重复分发；下方路径以原工作区 `E:/Lanczos` 为基准，移交后可用发布清单绑定其实际位置和 SHA-256。
~~~~

**After**

~~~~text
本文件为技能方法提供经验来源和文件索引。历史会话用于定位问题与修正过程，工程文件记录对应输入和结果；原工作区路径以 `E:/Lanczos` 为基准，迁移时通过发布清单绑定位置及 SHA-256。
~~~~

#### 原修改 214

原行号：7–7；当时修改后行号：7–7。

位置：经验来源与证据覆盖 / 对话覆盖

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
已逐页读取以下 10 个聊天，所有分页均到 `hasMore=false`，合计 93 回合。标题按工具返回原文记录。当前聊天补充了 model.nb 来源、训练路线与授权核对。此覆盖不等于读取了账户中不可访问或已删除的对话。
~~~~

**After**

~~~~text
已逐页读取以下 10 个聊天，所有分页均到 `hasMore=false`，合计 93 回合。标题按工具返回原文记录。当前聊天补充了 model.nb 来源、训练路线与授权核对。本次提炼范围限定为上述可访问聊天。
~~~~

#### 原修改 215

原行号：28–28；当时修改后行号：28–28。

位置：证据案例 / E01：视频流的正确性不能只看静态画面

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
现存说明记录：单通道 16×12 连续两帧 1536 个像素、640×8 目标行宽测试 20480 个像素零错误；RGB 连续两帧 1536 个像素零错误。检查覆盖 clamp、Q10 数值、SOF/EOL、反压保持、通道锁步。旧版本坐标总线宽度错误，后改为 `$clog2(2*IMG_WIDTH)` 等按数值范围计算。全分辨率长时上板稳定性不由这些测试证明。
~~~~

**After**

~~~~text
现存说明记录：单通道 16×12 连续两帧 1536 个像素、640×8 目标行宽测试 20480 个像素零错误；RGB 连续两帧 1536 个像素零错误。检查覆盖 clamp、Q10 数值、SOF/EOL、反压保持、通道锁步。旧版本坐标总线宽度错误，后改为 `$clog2(2*IMG_WIDTH)` 等按数值范围计算。全分辨率长时稳定性另设板端测试。
~~~~

#### 原修改 216

原行号：34–34；当时修改后行号：34–34。

位置：证据案例 / E02：预处理必须验证整个张量

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
历史记录：五类 640×360 RGB888 输入，每帧输出 307200 个 FP32，与 CPU 双线性 Golden 逐位一致。Vivado 2024.2 独立 IP 后布线报告：709 LUT、333 FF、25.5 BRAM36 等效 tile、1 DSP、100 MHz WNS +3.519 ns。流接口仿真 7.88884 ms；真实 DDR 端到端延迟、模型框变化和 2022.2 重跑未验证。该固定尺寸的插值结果落在 1/32 像素值网格，归一化 ROM 按 n/32 编址，Golden 直接以双精度插值后归一化再转 FP32。改变尺寸后不能直接沿用这张 ROM 和相位表。
~~~~

**After**

~~~~text
历史记录：五类 640×360 RGB888 输入，每帧输出 307200 个 FP32，与 CPU 双线性 Golden 逐位一致。Vivado 2024.2 独立 IP 后布线报告：709 LUT、333 FF、25.5 BRAM36 等效 tile、1 DSP、100 MHz WNS +3.519 ns。流接口仿真 7.88884 ms；真实 DDR 端到端延迟、模型框变化和 2022.2 重跑未验证。该固定尺寸的插值结果落在 1/32 像素值网格，归一化 ROM 按 n/32 编址，Golden 直接以双精度插值后归一化再转 FP32。改变尺寸时需重新生成与坐标契约匹配的 ROM 和相位表。
~~~~

#### 原修改 217

原行号：40–40；当时修改后行号：40–40。

位置：证据案例 / E03：ARM 指令集正确仍可能因系统库失败

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
历史失败：早期动态链接二进制要求 `GLIBC_2.27`，旧板端系统兼容性未满足，后改为完全静态链接。Cortex-A9 QEMU 运行 JPEG 检出 8 个 PERSON，最高置信度 0.8298。实时 raw-frame 程序的另一份 QEMU 日志检出 6 个 PERSON，两份输入/程序不同，结果不能混用。QEMU 不能提供板卡 FPS。
~~~~

**After**

~~~~text
历史失败：早期动态链接二进制要求 `GLIBC_2.27`，旧板端系统兼容性未满足，后改为完全静态链接。Cortex-A9 QEMU 运行 JPEG 检出 8 个 PERSON，最高置信度 0.8298。实时 raw-frame 程序的另一份 QEMU 日志检出 6 个 PERSON，两份结果按输入和程序分别记录，板卡 FPS 使用真实板端计时。
~~~~

#### 原修改 218

原行号：46–46；当时修改后行号：46–46。

位置：证据案例 / E04：镜像重建前先识别工具代际

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
观察：XSA 标示 2022.2，旧 2017.4 流程期待 HDF；现有 Ubuntu 24.04/26.04 环境与本次推荐宿主版本不匹配，且未安装所需 PetaLinux。历史请求的 BOOT.BIN/image.ub 未因脚本已写好而视为完成。宿主支持表和工具命令随版本变化，复用时应查目标版本官方文档。
~~~~

**After**

~~~~text
观察：XSA 标示 2022.2，旧 2017.4 流程期待 HDF；现有 Ubuntu 24.04/26.04 环境与本次推荐宿主版本不匹配，且未安装所需 PetaLinux。该次工作完成环境预检与脚本准备，BOOT.BIN/image.ub 构建留待依赖满足后执行。宿主支持表和工具命令随版本变化，复用时应查目标版本官方文档。
~~~~

#### 原修改 219

原行号：52–52；当时修改后行号：52–52。

位置：证据案例 / E05：VDMA 私有快照保护推理输入

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
代码选择最近完成槽、复制到私有内存并复查写指针。该策略依赖复制期间写者未回绕覆盖所选槽。仅前后两次环形指针相同不能排除多次回绕；复用必须增加可证明的时间界限、单调帧序号或显式缓冲所有权。本包将此作为验收缺口说明，不把离线运行称为真实 DMA 并发验证。
~~~~

**After**

~~~~text
代码选择最近完成槽、复制到私有内存并复查写指针。该策略依赖复制期间写者未回绕覆盖所选槽。仅前后两次环形指针相同不能排除多次回绕；复用必须增加可证明的时间界限、单调帧序号或显式缓冲所有权。本包将上述条件纳入验收清单，真实 DMA 并发按目标板证据评估。
~~~~

#### 原修改 220

原行号：64–64；当时修改后行号：64–64。

位置：证据案例 / E07：小球回归需要保留失败案例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
历史记录：两类数据合并 6111 张图片，训练集 12228 个 PLAYER 框、1011 个 BALL 框；CPU 1 epoch、1% 数据冒烟 mAP50=0.00063，只证明链路能运行。队友正式训练报告指标为 P=96.1%、R=91.4%、mAP50=95.8%、mAP50-95=69.9%，不能外推真实视频/板端性能。真实视频分块检测仍有低置信假球；目标数上限内增加 BALL 保留策略，遮挡时保留 valid=0 的短时轨迹。上述数值是历史记录，本次未重训。
~~~~

**After**

~~~~text
历史记录：两类数据合并 6111 张图片，训练集 12228 个 PLAYER 框、1011 个 BALL 框；CPU 1 epoch、1% 数据冒烟 mAP50=0.00063，只证明链路能运行。队友正式训练报告指标为 P=96.1%、R=91.4%、mAP50=95.8%、mAP50-95=69.9%，该训练集指标与真实视频/板端性能按各自测试条件记录。真实视频分块检测仍有低置信假球；目标数上限内增加 BALL 保留策略，遮挡时保留 valid=0 的短时轨迹。上述数值引用历史训练记录。
~~~~

#### 原修改 221

原行号：70–70；当时修改后行号：70–70。

位置：证据案例 / E08：负消融结果同样是可复用结论

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
VOC2007 代理数据，训练 1200 ROI、验证 400 ROI、同一 TinyFoveaCNN。Closed-set Accuracy：Nearest 49.00%、Bilinear 53.25%、Lanczos 49.75%。Operational Accuracy：19.50%、14.25%、20.50%。后者包含 UNKNOWN 拒识条件，不能与闭集准确率混写。小目标仅 23 个样本，不足以支撑强结论。Pillow Lanczos 也不等于 FPGA 定点实现。
~~~~

**After**

~~~~text
VOC2007 代理数据，训练 1200 ROI、验证 400 ROI、同一 TinyFoveaCNN。Closed-set Accuracy：Nearest 49.00%、Bilinear 53.25%、Lanczos 49.75%。Operational Accuracy：19.50%、14.25%、20.50%。后者包含 UNKNOWN 拒识条件，与闭集准确率分列；小目标 23 个样本按小样本条件解释。Pillow Lanczos 与 FPGA 定点实现需分别验证。
~~~~

#### 原修改 222

原行号：76–76；当时修改后行号：76–76。

位置：证据案例 / E09：字段同名不代表语义兼容

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
旧检测流置信度为 0..255，注意力输入为 0..100；速度 Q8.8 与整数不同；OBSTACLE 不可直接转为 BICYCLE；区域枚举语义不同。现存 12 帧软件闭环报告为 PASS，正常结果接受、过期结果拒绝、request ID 唯一。输入为合成视频、软件 Lanczos 和 VOC 模型，未证明 FPGA ROI/板端闭环。
~~~~

**After**

~~~~text
旧检测流置信度为 0..255，注意力输入为 0..100；速度 Q8.8 与整数不同；OBSTACLE 不可直接转为 BICYCLE；区域枚举语义不同。现存 12 帧软件闭环报告为 PASS，正常结果接受、过期结果拒绝、request ID 唯一。输入为合成视频、软件 Lanczos 和 VOC 模型，FPGA ROI 及板端闭环留待实际硬件接入后验证。
~~~~

### skill/experience_notes.md

#### 原修改 224

原行号：5–5；当时修改后行号：5–5。

位置：嵌赛项目可复用技能包

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
本包从“嵌赛”项目的 10 个可访问历史聊天（93 个回合）及当前聊天提炼，结合本地源码、测试、训练报告和工程材料编写。整理日期：2026-10-29。覆盖清单及证据见 [EVIDENCE.md](evidence.md)。无法读取的原始附件、未保存的串口记录不作为已验证事实。
~~~~

**After**

~~~~text
本包从“嵌赛”项目的 10 个可访问历史聊天（93 个回合）及当前聊天提炼，结合本地源码、测试、训练报告和工程材料编写。整理日期：2026-10-29。覆盖清单及证据见 [EVIDENCE.md](evidence.md)。各案例的验证层级和原始材料可获取情况列于来源说明。
~~~~

#### 原修改 225

原行号：15–15；当时修改后行号：15–15。

位置：嵌赛项目可复用技能包 / 技能索引

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| [01-axi-video-regression](01-axi-video-regression/skill.md) | AXI 视频缩放错行、RGB 错位、反压后丢像素 | 握手不变量、独立 Golden、连续帧回归提示词 | 单通道及 RGB 小帧、目标行宽测试；不等于全系统上板 |
~~~~

**After**

~~~~text
| [01-axi-video-regression](01-axi-video-regression/skill.md) | AXI 视频缩放错行、RGB 错位、反压后丢像素 | 握手不变量、独立 Golden、连续帧回归提示词 | 单通道及 RGB 小帧、目标行宽的离线回归 |
~~~~

#### 原修改 226

原行号：18–18；当时修改后行号：18–18。

位置：嵌赛项目可复用技能包 / 技能索引

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| [04-petalinux-xsa-rebuild](04-petalinux-xsa-rebuild/skill.md) | XSA、bitstream、系统镜像版本混用 | XSA/外部 bitstream 校验脚本、环境预检清单 | 硬件匹配与环境阻塞有记录；不声称镜像已构建 |
~~~~

**After**

~~~~text
| [04-petalinux-xsa-rebuild](04-petalinux-xsa-rebuild/skill.md) | XSA、bitstream、系统镜像版本混用 | XSA/外部 bitstream 校验脚本、环境预检清单 | 硬件身份检查与环境预检已记录，镜像构建按目标环境执行 |
~~~~

#### 原修改 227

原行号：24–24；当时修改后行号：24–24。

位置：嵌赛项目可复用技能包 / 技能索引

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| [10-hls-cross-platform-port](10-hls-cross-platform-port/skill.md) | 将上游 FPGA 后端误当可直接部署的 Zynq 工程 | 仓库职责图、寄存器/内存迁移表、器件支持预检 | Z100 在 set_part 阻塞；后续报告不虚填 |
~~~~

**After**

~~~~text
| [10-hls-cross-platform-port](10-hls-cross-platform-port/skill.md) | 将上游 FPGA 后端误当可直接部署的 Zynq 工程 | 仓库职责图、寄存器/内存迁移表、器件支持预检 | Z100 的 set_part 器件预检定位了环境依赖 |
~~~~

#### 原修改 228

原行号：32–32；当时修改后行号：32–32。

位置：嵌赛项目可复用技能包 / 使用方法

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
4. 输出包含“已证实、仅历史记录、未验证”的状态，不能把本包案例结果当作自己工程的验收结果。
~~~~

**After**

~~~~text
4. 输出按“已证实、历史记录、待验证”标注，新工程以自身输入和测试结果验收。
~~~~

#### 原修改 229

原行号：56–56；当时修改后行号：56–56。

位置：嵌赛项目可复用技能包 / 验证与复用边界

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本包没有 PYNQ 框架实测记录，因此不申报“已验证的通用 PYNQ Skill”。AXI 握手、DMA/cache、张量校验和发布检查可迁移到 PYNQ，但换成 `Overlay`/`allocate`/DMA Python API 后，需重新验证缓冲生命周期、flush/invalidate、TLAST 和完成条件，才能形成 PYNQ 专项提交。
~~~~

**After**

~~~~text
PYNQ 迁移可复用 AXI 握手、DMA/cache、张量校验和发布检查方法。当前 [Overlay 与 DMA 示例](pynq-overlay-dma/readme.md)已完成离线模拟验证；转接 `Overlay`、`allocate` 和 DMA Python API 后，还需在目标平台验证缓冲生命周期、flush/invalidate、TLAST 和完成条件，实板及跨项目复用为后续验证项。
~~~~

#### 原修改 230

原行号：58–58；当时修改后行号：58–58。

位置：嵌赛项目可复用技能包 / 验证与复用边界

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
本包整理自本队对话与工程观察；不为引用的第三方代码、权重、图片、数据集或上游库重新授予许可。公开提交前，按第 11 项逐项记录适用授权；不明处填“待确认”。
~~~~

**After**

~~~~text
本包整理自团队对话与工程观察。第三方代码、权重、图片、数据集和上游库沿用各自许可，公开提交时按第 11 项记录授权；待确认项明确标注。
~~~~

### skill/preproc-fp32-golden/readme.md

#### 原修改 231

原行号：5–5；当时修改后行号：5–5。

位置：PicoDet FP32 预处理 Golden 验证 / 问题来源与提炼

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
在摄像头、四路 DMA 和 FPGA 预处理对接时，仅看到 IP 完成或画面正常，并不能确认模型实际收到的张量正确：输入帧可能被覆盖，通道顺序、缩放或缓存状态也可能不一致。因此将“固定同一输入、比较完整参考张量、检查错误和退出码”整理成可复用流程；项目背景与局限见 [AI 协作记录](../../report/ai_collaboration_record.md)的预处理案例。
~~~~

**After**

~~~~text
在摄像头、四路 DMA 和 FPGA 预处理对接时，需要同时检查 IP 完成状态和模型实际收到的完整张量：输入帧可能被覆盖，通道顺序、缩放或缓存状态也可能不一致。因此将“固定同一输入、比较完整参考张量、检查错误和退出码”整理成可复用流程；项目背景与局限见 [AI 协作记录](../../report/ai_collaboration_record.md)的预处理案例。
~~~~

#### 原修改 232

原行号：9–9；当时修改后行号：9–9。

位置：PicoDet FP32 预处理 Golden 验证 / 适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
验证 RGB888 摄像头帧经 FPGA 预处理后生成的 FP32 NCHW 张量。默认示例契约为 640x360 RGB888 输入、`1x3x320x320` 输出；换模型或输入尺寸时，应先修改参考程序和 DMA 长度，再使用本 Skill。它是验证流程，不包含队友 IP 的所有权或许可证。
~~~~

**After**

~~~~text
验证 RGB888 摄像头帧经 FPGA 预处理后生成的 FP32 NCHW 张量。默认示例契约为 640x360 RGB888 输入、`1x3x320x320` 输出；换模型或输入尺寸时，应先修改参考程序和 DMA 长度，再使用本 Skill。IP 的来源和使用许可由对应工程资料另行记录。
~~~~

#### 原修改 233

原行号：19–19；当时修改后行号：19–19。

位置：PicoDet FP32 预处理 Golden 验证 / 不依赖开发板的最小示例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
将本目录完整复制到任意位置，在该目录下用 Python 3.10 或更新版本运行；脚本仅使用标准库。`examples/` 中是人工构造的 2×2 小端 FP32 测试数据，**不是真实摄像头或板端输出**：
~~~~

**After**

~~~~text
将本目录完整复制到任意位置，在该目录下用 Python 3.10 或更新版本运行；脚本仅使用标准库。`examples/` 提供人工构造的 2×2 小端 FP32 正反例，适合在主机验证比较器：
~~~~

#### 原修改 234

原行号：31–31；当时修改后行号：31–31。

位置：PicoDet FP32 预处理 Golden 验证 / 非 EagleEye 形状的复用示例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
以下是合成的 2×3 小端 FP32 数据，展示比较工具不依赖摄像头或具体模型；不是板端数据：
~~~~

**After**

~~~~text
以下是合成的 2×3 小端 FP32 数据，展示比较工具在不同张量形状下的复用，数据来源为主机构造：
~~~~

#### 原修改 235

原行号：38–38；当时修改后行号：38–38。

位置：PicoDet FP32 预处理 Golden 验证 / 非 EagleEye 形状的复用示例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
正例退出码 0；第二个例子改动了一个值，退出码 1。其他项目先确认输入来源一致、元素类型、布局、字节序和预先确定的容差；本工具不执行格式转换。用于 PYNQ 项目时，仅比较导出的数据，不负责 Overlay、缓存同步或 DMA。PYNQ 板端适配尚未验证。
~~~~

**After**

~~~~text
正例退出码 0；第二个例子改动了一个值，退出码 1。其他项目先确认输入来源一致、元素类型、布局、字节序和预先确定的容差；比较器接收符合契约的原始文件；格式转换、Overlay、缓存与 DMA 由目标工程负责。PYNQ 文件比较可复用本工具，板端适配待实际迁移验证。
~~~~

#### 原修改 236

原行号：51–51；当时修改后行号：51–51。

位置：PicoDet FP32 预处理 Golden 验证 / 用法

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
测速要分别记录 IP `cycles`、DMA/DDR/cache 的端到端 `preprocess_ms`、纯 PS 参考 `preprocess_ms` 和检测结果。**IP 流接口仿真时延不是板端实测。**已收到当前版本的 [CPU/FPGA A/B 原始日志](../../report/preproc_ab_20261028_zh.md)：两组使用相同模型与线程，但 CPU 为双三次、FPGA 为双线性且实时帧不同；只能报告两种配置的计时比，不能称为同算法加速比或数值一致。
~~~~

**After**

~~~~text
测速分别记录 IP `cycles`、包含 DMA/DDR/cache 的端到端 `preprocess_ms`、PS 参考 `preprocess_ms` 和检测结果。流接口仿真周期按仿真条件报告，板端时间按现场计时报告。已有 [CPU/FPGA A/B 日志](../../report/preproc_ab_20261028_zh.md)保持相同模型与线程，比较 CPU 双三次和 FPGA 双线性两种配置，使用先后采集的实时帧；同算法收益及同输入数值一致性分别验证。
~~~~

#### 原修改 237

原行号：55–55；当时修改后行号：55–55。

位置：PicoDet FP32 预处理 Golden 验证 / 已验证效果与边界

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
仓库内 [Icarus 仿真记录](../../sim/preproc_iverilog_20261027.log)显示五类帧各 307200 个值与参考逐位一致；采集帧的三路仿真输出和独立复核方法见 [仿真数据说明](../../sim/preproc_captured_dma_20261027/readme.md)。这证明指定测试台和输入下的离线结果，不证明实板 DMA 与缓存行为。
~~~~

**After**

~~~~text
仓库内 [Icarus 仿真记录](../../sim/preproc_iverilog_20261027.log)显示五类帧各 307200 个值与参考逐位一致；采集帧的三路仿真输出和独立复核方法见 [仿真数据说明](../../sim/preproc_captured_dma_20261027/readme.md)。该记录覆盖指定测试台和输入下的离线结果，实板 DMA 与缓存行为由独立板端测试核验。
~~~~

### skill/pynq-overlay-dma/readme.md

#### 原修改 238

原行号：7–7；当时修改后行号：7–7。

位置：PYNQ Overlay、寄存器与 DMA 接入示例 / 问题来源和适用范围

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
**团队目前没有 PYNQ 实板和真实外部项目复用验证资料；本项仅作为 Python 离线模拟验证的 Skill 候选提交，不再作为需要队友提供的既有材料。** 示例不是完整设备驱动、隔离机制、硬件安全准入或可直接部署的 Overlay；不提供实际 BIT/HWH。哈希核对只验证文件与给定值一致，不能证明来源可信、BIT 与 HWH 真实配套或适用于某块板。
~~~~

**After**

~~~~text
**验证状态：已完成 Python 离线模拟。**示例覆盖 Overlay 文件检查、寄存器白名单与简单 DMA 调用顺序；PYNQ 实板和真实跨项目复用为后续验证项。BIT/HWH 由目标工程提供，设备驱动、隔离、硬件准入和故障恢复由目标平台负责。SHA256 用于文件完整性核对，来源认证及 BIT/HWH 与板卡的实际配套关系单独核验。
~~~~

#### 原修改 239

原行号：18–18；当时修改后行号：18–18。

位置：PYNQ Overlay、寄存器与 DMA 接入示例 / 脱离工程运行模拟示例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
第一条只打印带 `MOCK` 的缓存和传输顺序，没有产生 FPGA 事务或数值结果；第二条使用 Python 假对象检查成功、身份不符、未授权寄存器写、DMA 异常、长度错误及中断。验证记录见 [本仓库记录](../../sim/pynq_skill_mock_20261028.txt)。复制到别的项目时只复制本目录即可，仓库记录链接不属于运行依赖。
~~~~

**After**

~~~~text
第一条以 `MOCK` 标记演示缓存和传输顺序；第二条使用 Python 假对象检查成功、身份不符、未授权寄存器写、DMA 异常、长度错误及中断。验证记录见 [本仓库记录](../../sim/pynq_skill_mock_20261028.txt)。复制到别的项目时只复制本目录即可，仓库记录链接不属于运行依赖。
~~~~

#### 原修改 240

原行号：83–83；当时修改后行号：83–83。

位置：PYNQ Overlay、寄存器与 DMA 接入示例 / 官方资料与验证边界

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
参考 [PYNQ allocate 与缓存 API](https://pynq.readthedocs.io/en/latest/pynq_libraries/allocate.html)、[PYNQ 2.7 DMA 实现](https://pynq.readthedocs.io/en/v2.7.0/_modules/pynq/lib/dma.html) 和 [PYNQ Overlay 接口](https://pynq.readthedocs.io/en/v3.0.0/_modules/pynq/overlay.html)。实际使用须对照所装版本。当前仅在 PC 的模拟对象上测试控制流，未验证 Overlay 重配置、寄存器、DMA、缓存一致性、目标板资源隔离或跨板复用；不能以此申请“实板已验证”结论。
~~~~

**After**

~~~~text
参考 [PYNQ allocate 与缓存 API](https://pynq.readthedocs.io/en/latest/pynq_libraries/allocate.html)、[PYNQ 2.7 DMA 实现](https://pynq.readthedocs.io/en/v2.7.0/_modules/pynq/lib/dma.html) 和 [PYNQ Overlay 接口](https://pynq.readthedocs.io/en/v3.0.0/_modules/pynq/overlay.html)，实际使用对照安装版本。本项已用 PC 模拟对象验证控制流；Overlay 重配置、寄存器、真实 DMA/cache、资源隔离及跨板复用待目标平台实测。
~~~~

### skill/readme.md

#### 原修改 241

原行号：36–36；当时修改后行号：36–36。

位置：EagleEye 可复用技能包 / 使用与验证

原因：B/C：合并重复边界说明，在成果之后集中交代测试条件

**Before**

~~~~text
每项都有适用范围和验证边界。部分工具功能相近，但输入契约和输出有所不同，按任务选用即可。足球检测、AX7020 和历史工程案例仅作为可复用经验，不代表当前 Z035 系统已完成相同板端功能。
~~~~

**After**

~~~~text
每项 Skill 提供适用范围、输入输出契约和验证方法。可根据任务选择工具；足球检测、AX7020 和历史工程案例用于展示可迁移的方法，当前 Z035 的板端功能见各自测试记录。
~~~~

#### 原修改 242

原行号：47–47；当时修改后行号：47–47。

位置：EagleEye 可复用技能包 / 使用与验证

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
已有测试和来源见 [验证记录](validation.md)、[整理与复核记录](validation_local_20261030.txt)、[经验来源](evidence.md) 和 [Skill 复用测试](../sim/skill_portability_20261028.txt)。PYNQ 示例尚无实板或真实跨项目验证，不能把模拟结果写成实板结果。
~~~~

**After**

~~~~text
已有测试和来源见 [验证记录](validation.md)、[整理与复核记录](validation_local_20261030.txt)、[经验来源](evidence.md) 和 [Skill 复用测试](../sim/skill_portability_20261028.txt)。PYNQ 示例已完成模拟对象验证，实板和真实跨项目复用留待迁移时验证。
~~~~

### skill/roi-review-control/readme.md

#### 原修改 243

原行号：5–5；当时修改后行号：5–5。

位置：可暂停的 ROI 自动/手动复判 / 问题来源与提炼

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
项目曾出现暂停后切换模式导致 HDMI 黑屏的反馈；复盘将暂停状态、帧边界、模式切换和人工接管拆成需要核对的条件，而不是仅凭检测进程仍在运行判断视频正常。相关历史过程见 [AI 协作记录](../../report/ai_collaboration_record.md)的黑屏排查案例；本 Skill 将可检查的静态接口与必须上板观察的行为分开。
~~~~

**After**

~~~~text
项目曾出现暂停后切换模式导致 HDMI 黑屏的反馈；复盘将暂停状态、帧边界、模式切换和人工接管拆成需要核对的条件，并将检测进程、显示链路与硬件状态分别检查。相关历史过程见 [AI 协作记录](../../report/ai_collaboration_record.md)的黑屏排查案例；本 Skill 将可检查的静态接口与必须上板观察的行为分开。
~~~~

#### 原修改 244

原行号：9–9；当时修改后行号：9–9。

位置：可暂停的 ROI 自动/手动复判 / 适用范围

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
适合已有“检测器给出 ROI + 视频放大窗口”的 Zynq/FPGA 工程。实时播放时自动跟踪；暂停后由 USB 相对鼠标接管 ROI，KEY0 切换放大/边缘模式；可选两路位置舵机跟随最终 ROI。此 Skill 提供控制契约和验收方法，**不是**可以直接接到任意板卡的引脚文件。
~~~~

**After**

~~~~text
适合已有“检测器给出 ROI + 视频放大窗口”的 Zynq/FPGA 工程。实时播放时自动跟踪；暂停后由 USB 相对鼠标接管 ROI，KEY0 切换放大/边缘模式；可选两路位置舵机跟随最终 ROI。此 Skill 提供可迁移的控制契约和验收方法，目标板引脚与电气配置按各自工程定义。
~~~~

#### 原修改 245

原行号：26–26；当时修改后行号：26–26。

位置：快速检查 / 不依赖开发板的最小示例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
将本目录完整复制到任意位置，在该目录下用 Python 3.10 或更新版本运行；脚本仅使用标准库。`examples/sample.xsa` 是只包含演示用 `system.hwh` 的合成 ZIP，**不是 Vivado 生成的可配置硬件**：
~~~~

**After**

~~~~text
将本目录完整复制到任意位置，在该目录下用 Python 3.10 或更新版本运行；脚本仅使用标准库。`examples/sample.xsa` 为仅含演示 `system.hwh` 的合成 ZIP，用于主机检查器正反例：
~~~~

#### 原修改 246

原行号：45–45；当时修改后行号：45–45。

位置：快速检查 / 第二种工程与独立 HWH 示例

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
依次应返回 0（匹配）和 1（故意缺少模块）。XSA 和 HWH 参数互斥；文件损坏、配置类型错误返回 2。迁移时用目标设计的 HWH 和逐项确认的模块名、顶层端口、地址替换示例；不能将示例地址用于设备访问。若目标是 PYNQ，此检查可用于预先审查与 Overlay 配套的 HWH，但**不加载 Overlay、不访问寄存器或 DMA**，也不能证明 BIT 与 HWH、板卡或运行时状态匹配。尚未在 PYNQ 环境和其他真实工程验证。
~~~~

**After**

~~~~text
依次应返回 0（匹配）和 1（故意缺少模块）。XSA 和 HWH 参数互斥；文件损坏、配置类型错误返回 2。迁移时用目标设计的 HWH 和逐项确认的模块名、顶层端口、地址替换示例；不能将示例地址用于设备访问。若目标是 PYNQ，可用本工具预查 Overlay 的 HWH 元数据；加载、寄存器与 DMA 操作由目标平台负责，BIT/HWH 配套及运行时身份分别核验。当前跨工程示例为主机合成数据，PYNQ 与其他真实工程复用留待验证。
~~~~

#### 原修改 247

原行号：56–56；当时修改后行号：56–56。

位置：快速检查 / 检查自己的工程

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
该脚本检查 XSA 中的 HWH 或独立 HWH 里配置列出的模块、外部端口和寄存器基地址，**不能证明**鼠标事件、暂停后帧保持或舵机方向正确。配置应列出实际需要检查的项目；空配置会被拒绝。脚本要求 ZIP 根目录包含名为 `system.hwh` 的文件；其他导出结构或 HWH 格式需先核对，不能忽略异常。随后至少做：KEY1 暂停 10 秒且 HDMI 不黑屏；两种起始模式各循环多次 KEY0；鼠标四个动作及拔插恢复；ROI 在四边界保持合法；KEY1 恢复后自动跟踪；断电冷启动和 30 分钟连续演示。这里列的是建议验收步骤，不代表这些步骤已有本次原始记录。
~~~~

**After**

~~~~text
该脚本检查 XSA 中的 HWH 或独立 HWH 中列出的模块、外部端口和寄存器基地址。鼠标事件、暂停后的帧保持和舵机方向由后续现场功能测试检查。配置应列出实际需要检查的项目；空配置会被拒绝。脚本要求 ZIP 根目录包含名为 `system.hwh` 的文件；其他导出结构或 HWH 格式需先核对，不能忽略异常。随后至少做：KEY1 暂停 10 秒且 HDMI 不黑屏；两种起始模式各循环多次 KEY0；鼠标四个动作及拔插恢复；ROI 在四边界保持合法；KEY1 恢复后自动跟踪；断电冷启动和 30 分钟连续演示。以上为迁移后的建议验收步骤，实际执行结果按现场记录填写。
~~~~

#### 原修改 248

原行号：62–62；当时修改后行号：62–62。

位置：可暂停的 ROI 自动/手动复判 / 已验证与失效条件

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
现场已确认复现 BIT 上的摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能均正常。此为现场确认；未记录的数值及跨板复用效果不作推测。现场证据范围见 [上板记录](../../report/board_reproduced_20261028_zh.md)。
~~~~

**After**

~~~~text
现场已确认复现 BIT 上的摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能均正常。该项验证来源为现场功能确认，量化指标和跨板复用按各自实际测试记录。现场证据范围见 [上板记录](../../report/board_reproduced_20261028_zh.md)。
~~~~

#### 原修改 249

原行号：64–64；当时修改后行号：64–64。

位置：可暂停的 ROI 自动/手动复判 / 已验证与失效条件

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
当前仓库的主机测试及合成示例只能验证检查器能够识别匹配和缺失项目，不证明帧边界握手或物理行为。若 XSA 契约不匹配、帧边界切换不受控、USB 鼠标未枚举、DDR 画面黑屏或舵机持续旋转，应停止并按现场已验证的恢复方案处理。不同分辨率、板卡、XSA/HWH 结构、引脚和外围设备均须重新配置并独立验证。
~~~~

**After**

~~~~text
主机测试及合成示例覆盖契约匹配、缺失项目和异常输入；帧边界握手与物理行为需在目标硬件检查。若 XSA 契约不匹配、帧边界切换不受控、USB 鼠标未枚举、DDR 画面黑屏或舵机持续旋转，应停止并按现场已验证的恢复方案处理。不同分辨率、板卡、XSA/HWH 结构、引脚和外围设备均须重新配置并独立验证。
~~~~

### skill/validation.md

#### 原修改 250

原行号：3–3；当时修改后行号：3–3。

位置：本次技能包验证

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
验证日期：2026-10-29。本次验证对象是新整理的技能格式、校验工具和已有保存产物；没有重新训练、运行 Vivado 构建或操作开发板。
~~~~

**After**

~~~~text
验证日期：2026-10-29。本次验证对象是新整理的技能格式、校验工具和已有保存产物；验证方式为主机工具测试和保存产物复核，训练、Vivado 及板端测试按来源记录列示。
~~~~

#### 原修改 251

原行号：19–19；当时修改后行号：19–19。

位置：本次技能包验证 / 已有本地产物复核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| JPEG 版 paddle_person ELF 头 | ARM32、小端、EABI5、hard-float 标记通过 | 不能由此证明 NEON、动态库或实际运行 |
~~~~

**After**

~~~~text
| JPEG 版 paddle_person ELF 头 | ARM32、小端、EABI5、hard-float 标记通过 | ELF 头部验证；NEON、动态依赖和实际运行分别检查 |
~~~~

#### 原修改 252

原行号：21–22；当时修改后行号：21–22。

位置：本次技能包验证 / 已有本地产物复核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
| 真实视频流，声明 1920×1080 | 272 帧、1616 条记录通过；507 条 held、134 次观测到 invalid→valid | 跨类别总体统计；不等于球的恢复真值或 ID 准确率 |
| XSA 内嵌/外部 system_wrapper.bit | 相同，2037636 bytes | 文件身份一致，不证明板级配置或启动成功 |
~~~~

**After**

~~~~text
| 真实视频流，声明 1920×1080 | 272 帧、1616 条记录通过；507 条 held、134 次观测到 invalid→valid | 跨类别总体统计，球的恢复真值和 ID 准确率另需标注评价 |
| XSA 内嵌/外部 system_wrapper.bit | 相同，2037636 bytes | 内嵌与外部文件身份核验，板级配置及启动分别验证 |
~~~~

#### 原修改 253

原行号：37–37；当时修改后行号：37–37。

位置：本次技能包验证 / 接收方复核

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
技能材料已按用途统一整理在当前目录；格式校验不证明工作流在新板卡上成功，保存的 Golden 不证明本次重新运行过 RTL。历史资源/时序、QEMU、训练和上板反馈的证据范围见 [经验来源](evidence.md)。迁移到新模型、工具版本、器件、坐标系或 PYNQ API 后，需按相应技能重新验证。
~~~~

**After**

~~~~text
技能材料已按用途统一整理在当前目录；格式、工具行为及保存张量按本次检查结果记录，目标板运行及 RTL 重跑分别列示。历史资源/时序、QEMU、训练和上板反馈的证据范围见 [经验来源](evidence.md)。迁移到新模型、工具版本、器件、坐标系或 PYNQ API 后，需按相应技能重新验证。
~~~~

### src/attention_reference/readme.md

#### 原修改 254

原行号：3–3；当时修改后行号：3–3。

位置：注意力参考子集

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
此目录仅包含 `attention.c`、`attention_scheduler.c` 和对应的三个头文件，用于解释后续注意力 ROI 方案；不是完整注意力仿真仓库，也不是已上板 PicoDet 程序。来源参考仓库 `EagleEye_C_Attention_Sim` 的 `c-attention` 分支，当前核对提交为 `25bd7a35ab1a072f004e75cad20331a3d128c166`。所用子集与当前分支的全部文件/构建目标并不等价。
~~~~

**After**

~~~~text
此目录仅包含 `attention.c`、`attention_scheduler.c` 和对应的三个头文件，用于说明注意力 ROI 算法；完整 PC 仿真及板端 PicoDet 程序分别位于对应工程。来源参考仓库 `EagleEye_C_Attention_Sim` 的 `c-attention` 分支，当前核对提交为 `25bd7a35ab1a072f004e75cad20331a3d128c166`。所用子集与当前分支的全部文件/构建目标并不等价。
~~~~

#### 原修改 255

原行号：5–5；当时修改后行号：5–5。

位置：注意力参考子集

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
在 Linux/WSL 下运行 `make` 可编译该目录实际包含的两个 `.c` 为静态库 `build/libeagleeye_attention_reference.a`；2026-10-27 已在独立副本中通过 GCC 编译。完整 PC 仿真应另取该 Git 分支并以其自身的测试、Makefile 和提交号核验。不要把该 PC 仿真或足球 PLAYER/BALL 检测描述为 Z035 已上板功能。
~~~~

**After**

~~~~text
在 Linux/WSL 下运行 `make` 可编译该目录实际包含的两个 `.c` 为静态库 `build/libeagleeye_attention_reference.a`；2026-10-27 已在独立副本中通过 GCC 编译。完整 PC 仿真应另取该 Git 分支并以其自身的测试、Makefile 和提交号核验。PC 仿真、足球 PLAYER/BALL 及 Z035 板端功能按各自测试环境记录。
~~~~

### src/picodet_c_candidate/readme.md

#### 原修改 256

原行号：6–6；当时修改后行号：6–6。

位置：PicoDet 注意力独立候选

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 推理 sequence 不是可证明的摄像头帧号；日志中 `c_request` 是软件生成的请求，不代表硬件完成，`hardware_commit=pending` 也不是实板成功。
~~~~

**After**

~~~~text
- 推理 sequence 表示推理序号；`c_request` 表示软件请求，`hardware_commit=pending` 表示提交待完成。摄像头帧号及硬件完成状态需各自事件记录。
~~~~

#### 原修改 257

原行号：8–8；当时修改后行号：8–8。

位置：PicoDet 注意力独立候选

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
- 本候选不包含足球检测或已验证的完整足球关注/控球链；只适配 PERSON 人体框。
~~~~

**After**

~~~~text
- 本候选面向 PERSON 人体框，足球检测及控球链由独立参考路线提供。
~~~~

#### 原修改 258

原行号：20–20；当时修改后行号：20–20。

位置：PicoDet 注意力独立候选 / 构建和离线测试

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
交叉编译所生成的 ELF 在本机 SHA256 为 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；这不是板端哈希读回。测试使用 mock 设备，不能证明硬件操作。QEMU 的 `--c-attention-dry-run` 仅允许离线 RGB、CPU 预处理且不启用硬件 ROI；一次输入 7 人框与原程序一致，并产生注意力 ROI。`sim/picodet_c_candidate/` 保留摘要。QEMU 不代表开发板 FPS。
~~~~

**After**

~~~~text
交叉编译所生成的 ELF 在本机 SHA256 为 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；该哈希为本地编译产物身份；离线设备交互使用 mock。QEMU 的 `--c-attention-dry-run` 仅允许离线 RGB、CPU 预处理且不启用硬件 ROI；一次输入 7 人框与原程序一致，并产生注意力 ROI。`sim/picodet_c_candidate/` 保留摘要。开发板 FPS 按实板计时报告。
~~~~

#### 原修改 259

原行号：24–24；当时修改后行号：24–24。

位置：PicoDet 注意力独立候选 / 板端反馈与证据边界

原因：A：以中性工程语言介绍成果，保留事实与适用范围

**Before**

~~~~text
2026-10-27，现场已确认上板使用这次新 PicoDet 注意力候选且功能正常，按现场确认记录。该次原始串口及量化性能数据未随包提供；不能将后来 AttentionPolicy 版本的日志当作该候选的运行日志，也不额外推断量化指标。现有资料确认暂无独立稳定 target_id。
~~~~

**After**

~~~~text
2026-10-27，现场已确认上板使用这次新 PicoDet 注意力候选且功能正常，按现场确认记录。该次原始串口及量化性能数据未随包提供；后续 AttentionPolicy 日志按对应 ELF 单独归档，本候选量化指标待实测记录。现有资料确认暂无独立稳定 target_id。
~~~~

## 关键技术事实的呈现

- PicoDet 模型推理在 ARM 端执行，FPGA 负责视频和模型输入预处理。
- CPU 双三次与 FPGA 双线性按两种实际配置统计，固定输入 Golden 按独立测试记录。
- 历史独立仿真、当前完整工程及现场功能反馈分别绑定版本。
- 已知 I/O、CDC 和 VDMA 状态保留在专题说明中。
- PYNQ 示例按离线模拟状态展示，原始对话和测试输出保留来源。

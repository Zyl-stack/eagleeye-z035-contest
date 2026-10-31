# EagleEye：仿生视觉转播系统

EagleEye 是一套基于 **Xilinx Zynq XC7Z035** 的仿生视觉辅助观察原型，以足球转播中的快速运动场景为主要演示方向。系统借鉴人眼周边视觉、选择性注意和中央凹视觉的分工：保留全局画面以观察场景，同时选择值得关注的区域，进行局部放大与展示。

摄像头提供全局视频，PicoDet 检测人体候选目标；注意力策略综合目标的区域、运动、接近趋势、尺寸和检测不确定性，选择关注对象并生成动态 ROI。视频画面与局部窗口共同显示，使用者也可以暂停画面，通过鼠标调整观察区域；双轴机械眼随关注位置联动。

本项目使用 **Vivado 2022.2** 构建 FPGA 工程，采用 **PicoDet-XS 与 Paddle-Lite** 在 ARM 端完成人体检测，并在 PL 侧实现视频处理与模型输入预处理。

## 系统功能

- **全局视频与局部观察**：OV5640 摄像头采集的视频经 FPGA 视频链路和 VDMA 处理，在 HDMI 界面中呈现全局画面及局部窗口。
- **五因素注意力**：ARM 端运行 PicoDet，并依据人体候选的区域、运动、接近趋势、尺寸和检测不确定性评分；结合目标保持和切换机制减少频繁跳变。
- **动态中央凹**：在 640×360 全局画面中选择固定 320×180 ROI，通过 FPGA 横纵各 2 倍重采样形成 640×360 局部窗口，支持双线性、Lanczos 等处理模式。
- **FPGA 图像预处理**：将视频输入转换为 PicoDet 所需的 `1×3×320×320` FP32 NCHW 张量。
- **交互与反馈**：支持画面暂停、鼠标调整 ROI、按键操作和双轴舵机联动；舵机不具备位置传感器反馈。

```text
OV5640 摄像头 → FPGA 视频通路 / VDMA → HDMI 全景与局部显示
                         ↓                     ↑
                 FPGA 图像预处理               │
                         ↓                     │
                ARM PicoDet 人体检测           │
                         ↓                     │
              五因素注意力 → 动态 ROI ──────────┘
                                 ↕
                        鼠标 / 按键 / 舵机
```

系统原理、软硬件接口及团队分工见 [设计报告](report/design_report_zh.md) 和 [硬件接口表](report/hardware_interfaces_zh.md)。当前演示采用 PicoDet 人体检测；足球 YOLO 参考路线单独保存在参考资料中。

系统将视频显示、检测推理和 ROI 更新异步组织，使画面展示与智能感知按各自节奏运行。

## 验证与实现结果

| 项目 | 已有结果 |
|---|---|
| 上板功能 | 现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能正常；[上板记录](report/board_reproduced_20261028_zh.md)与[应用日志](report/board_app_evidence_20261028_zh.md)已保存。 |
| 预处理板端 Golden | 一次独立固定输入测试中，**307,200 / 307,200 个 FP32 值**与参考逐位一致；[结果与复核方式](report/preproc_board_golden_20261029_zh.md)。 |
| RTL 仿真 | 预处理五类输入、AXI-Lite 和舵机等测试记录见 [仿真与验证说明](sim/verification_zh.md)。 |
| Vivado 构建 | 干净克隆后的独立构建生成 BIT/XSA：LUT 24,986、FF 28,246、BRAM36 303/500、DSP 49；已约束路径 WNS +0.608 ns、WHS +0.007 ns。[构建记录和报告](report/clean_reproduction_20261028_zh.md) |
| 预处理计时 | CPU 双三次与 FPGA 双线性两种预处理配置均已完成板端计时，输入条件、算法及统计口径见对应报告。[测试条件与结果](report/preproc_ab_20261028_zh.md) |

阶段联调观察中，检测更新约 **2 FPS**、视频显示约 **30 FPS**，分别对应智能感知和视频展示路径。[应用日志](report/board_app_evidence_20261028_zh.md)提供检测路径的计时记录；30 FPS 为阶段观察值，独立显示帧率计时留待后续测量。

### 测试条件与工程说明

固定输入 Golden 使用独立测试程序，其 BOOT 身份已记录，独立 BIT 哈希及与视频测试版本的对应关系待核对。CPU/FPGA 计时比较双三次与双线性两种配置，使用先后采集的实时帧。PicoDet 推理保持在 ARM 端，独立 V2/R3 卷积实验作为历史研究资料保存。I/O 时序、CDC 和 VDMA 的处理状态集中见 [工程告警说明](report/warning_review_zh.md)。

## 仓库目录

| 目录 | 内容 |
|---|---|
| [`src/`](src/) | RTL、IP、FPGA 工程及 PS 应用源码 |
| [`sim/`](sim/) | 仿真脚本、日志和验证说明 |
| [`build/`](build/) | 构建脚本、综合与实现报告 |
| [`board/`](board/) | 上板文件、实测日志和 Golden 对比数据 |
| [`data/`](data/) | 模型、测试输入和参考结果 |
| [`skill/`](skill/) | 可复用技能、示例和验证说明 |
| [`report/`](report/) | 设计报告、AI 协作记录与证据索引 |
| [`tools/`](tools/) | 完整性、复现产物与日志核验脚本 |
| [`reference/`](reference/) | 参考源码及 Linux 固定版本源码 |
| [`original/`](original/) | 工程恢复所需归档及参考源码快照，保留原文件大小写 |

按比赛要求查找具体文件，请从 [比赛材料与证据索引](report/contest_evidence_index_zh.md) 开始。

## 获取与复现

需要 Git LFS、Python 3.10+ 和 **Vivado 2022.2**；ARM/Linux 构建还需要匹配的 PetaLinux 交叉工具链、sysroot 和相应依赖。部分大文件通过 Git LFS 保存：

```sh
git lfs install
git clone https://github.com/Zyl-stack/eagleeye-z035-contest.git
cd eagleeye-z035-contest
git lfs pull
python -B tools/verify_submission.py
```

仓库中展开的文件名采用小写。为保留构建所需的原始大小写，请按 [复现指南](report/reproduction_zh.md) 使用 `tools/restore_a_repro.py` 将快照恢复到**新的独立目录**，再运行 `build/build_full_vivado.tcl`。构建和上板试验均使用独立工作区及实验介质，保留稳定 SD。

使用 Vivado 2025.2 可参考 [实验性迁移脚本及说明](build/vivado_2025_2_zh.md)。已记录的完整构建基于 2022.2；2025.2 脚本的目标版本运行验证为后续迁移步骤。

已完成从干净克隆恢复工程、综合与实现；布线中断后从检查点续跑生成 BIT/XSA。具体命令、原始日志及验证范围见 [干净复现记录](report/clean_reproduction_20261028_zh.md)。

## 报告与技能包

- [报告目录导航](report/readme.md) · [设计报告](report/design_report_zh.md) · [比赛材料与证据索引](report/contest_evidence_index_zh.md) · [版本身份](report/release_identity_zh.md)
- [AI 协作记录](report/ai_collaboration_record.md) · [Skill 索引](skill/index.md) · [第三方来源与授权](report/third_party_sources_zh.md)
- [预处理板端 Golden](report/preproc_board_golden_20261029_zh.md) · [CPU/FPGA 计时](report/preproc_ab_20261028_zh.md) · [复现指南](report/reproduction_zh.md)

PYNQ Skill 提供 Overlay、寄存器、DMA 和缓存一致性的可复用示例，已完成离线模拟验证，实板及跨项目复用留待平台迁移时验证。模型权重及部分第三方 IP 的授权核对状态详见 [来源与授权说明](report/third_party_sources_zh.md)。
# EagleEye：仿生视觉转播系统

EagleEye 是一套基于 **Xilinx Zynq XC7Z035** 的仿生视觉辅助观察原型，以足球转播中的快速运动场景为主要演示方向。系统借鉴人眼周边视觉、选择性注意和中央凹视觉的分工：保留全局画面以观察场景，同时选择值得关注的区域，进行局部放大与展示。

摄像头提供全局视频，PicoDet 检测人体候选目标；注意力策略综合目标的区域、运动、接近趋势、尺寸和检测不确定性，选择关注对象并生成动态 ROI。视频画面与局部窗口共同显示，使用者也可以暂停画面，通过鼠标调整观察区域；双轴机械眼随关注位置联动。

本项目使用 **Vivado 2022.2** 构建 FPGA 工程；人体检测使用 **PicoDet-XS 与 Paddle-Lite**，模型推理运行在 ARM 端。局部放大是对已有像素的重采样，不代表恢复了原图中不存在的细节；项目用于辅助观察，不是自动裁判系统。

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

系统原理、软硬件接口及团队分工见 [设计报告](report/design_report_zh.md) 和 [硬件接口表](report/hardware_interfaces_zh.md)。当前系统以 PicoDet 人体检测为准；仓库中的足球 YOLO 是独立参考路线，不代表已在当前 Z035 系统部署。

视频显示、检测推理和 ROI 更新具有不同节奏：显示持续更新，并不意味着每一帧都产生了新的检测结果。现有临时目标编号用于相邻检测结果关联，不代表跨遮挡的稳定跟踪。

## 验证与实现结果

| 项目 | 已有结果 |
|---|---|
| 上板功能 | 现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能正常；[上板记录](report/board_reproduced_20261028_zh.md)与[应用日志](report/board_app_evidence_20261028_zh.md)已保存。 |
| 预处理板端 Golden | 一次独立固定输入测试中，**307,200 / 307,200 个 FP32 值**与参考逐位一致；[结果与复核方式](report/preproc_board_golden_20261029_zh.md)。 |
| RTL 仿真 | 预处理五类输入、AXI-Lite 和舵机等测试记录见 [仿真与验证说明](sim/verification_zh.md)。 |
| Vivado 构建 | 干净克隆后的独立构建生成 BIT/XSA：LUT 24,986、FF 28,246、BRAM36 303/500、DSP 49；已约束路径 WNS +0.608 ns、WHS +0.007 ns。[构建记录和报告](report/clean_reproduction_20261028_zh.md) |
| 预处理计时 | 已有 CPU 与 FPGA 两组板端记录；两组使用不同缩放算法和不同实时帧，不能直接作为同算法加速比或精度对比。[测试条件与结果](report/preproc_ab_20261028_zh.md) |

技术文档记录的阶段联调观察为检测更新约 **2 FPS**、视频显示约 **30 FPS**；两者是不同指标。现有[应用日志](report/board_app_evidence_20261028_zh.md)可重算检测路径耗时，视频显示帧率未见独立计时记录，不将显示帧率当作检测速度。

固定输入 Golden 使用独立测试程序，该次测试的独立 BIT 哈希未核实；它不能直接证明其他版本或整网精度。I/O 时序、CDC 和 VDMA 告警的现有记录及限制见 [告警说明](report/warning_review_zh.md)。独立 V2/R3 卷积实验不等同于当前 Eyes 系统已实现 FPGA 卷积卸载。

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
| [`original/`](original/) | 原工程和补充材料快照，保留原文件大小写 |

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

仓库中展开的文件名采用小写；原工程脚本和约束可能依赖原始大小写。请先按 [复现指南](report/reproduction_zh.md) 使用 `tools/restore_a_repro.py` 将原快照恢复到**新的独立目录**，再运行 `build/build_full_vivado.tcl`。不要直接使用小写副本重建或覆盖稳定 SD。

使用 Vivado 2025.2 的人员可参考 [实验性迁移脚本及说明](build/vivado_2025_2_zh.md)；该脚本尚未在 2025.2 实际验证，不替代已记录的 2022.2 构建结果。

已记录的干净克隆构建曾在布线阶段中断，随后从检查点续跑生成 BIT/XSA；具体命令、日志和验证范围见 [干净复现记录](report/clean_reproduction_20261028_zh.md)。文件哈希核验、仿真、上板记录分别证明不同内容。

## 报告与技能包

- [报告目录导航](report/readme.md) · [设计报告](report/design_report_zh.md) · [比赛材料与证据索引](report/contest_evidence_index_zh.md) · [版本身份](report/release_identity_zh.md)
- [AI 协作记录](report/ai_collaboration_record.md) · [Skill 索引](skill/index.md) · [第三方来源与授权](report/third_party_sources_zh.md)
- [预处理板端 Golden](report/preproc_board_golden_20261029_zh.md) · [CPU/FPGA 计时](report/preproc_ab_20261028_zh.md) · [复现指南](report/reproduction_zh.md)

PYNQ Skill 提供可复用示例并完成离线模拟测试，尚无 PYNQ 实板或真实跨项目验证。模型权重及部分第三方 IP 的使用与再分发授权仍需按 [来源与授权说明](report/third_party_sources_zh.md) 核对。

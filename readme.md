# EagleEye｜让快速运动的画面看得更清楚

在体育运动中，有时候重要的细节只出现一瞬间：全景里的人物太小，想仔细看看某个区域，又不希望丢掉整幅画面的上下文。

EagleEye 是我们基于 **Xilinx Zynq XC7Z035** 做的一套视频辅助观察原型。它接入摄像头画面，在保留全景的同时展示感兴趣的局部区域；使用者也可以暂停画面，用鼠标调整观察位置。系统结合 PicoDet 人体检测和注意力选择，让关注区域能够随目标变化，并通过舵机提供机械动作反馈。

这个项目面向辅助观察和复判，不代替裁判作出判断。

## 它是怎么工作的？

摄像头采集的视频经过 FPGA 视频链路，一路用于 HDMI 显示，一路经过图像预处理后送入 ARM 上的 PicoDet。程序根据检测到的人体框选择关注目标，再更新 ROI 和相关控制。

```text
                       ┌→ HDMI 全景与局部画面
摄像头 → FPGA 视频链路 ┤
                       └→ FPGA 图像预处理 → ARM 人体检测
                                                 ↓
                               注意力选择 → ROI / 舵机控制
```

画面暂停后，可以通过鼠标调整 ROI，并通过按键控制显示。图像预处理在 FPGA 上完成；**PicoDet 模型推理运行在 ARM 端**。仓库中还保留了独立的卷积加速实验，但它不等同于当前视频系统已启用 FPGA 卷积推理。

想了解接口和设计细节，可以直接看[设计报告](report/design_report_zh.md)和[硬件接口表](report/hardware_interfaces_zh.md)。

## 我们已经做到哪里？

- **整机功能**：现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能正常。[上板记录](report/board_reproduced_20261028_zh.md)和[应用日志](report/board_app_evidence_20261028_zh.md)已保存。
- **预处理正确性**：一次独立固定输入的板端测试中，输出的 **307,200 个 FP32 值**与 Golden 逐位一致。[查看对比结果](report/preproc_board_golden_20261029_zh.md)。
- **工程构建**：使用 Vivado 2022.2 完成独立工程构建并生成 BIT/XSA。资源使用为 LUT 24,986、FF 28,246、BRAM36 303/500、DSP 49；已约束路径 WNS 为 +0.608 ns、WHS 为 +0.007 ns。[查看构建记录](report/clean_reproduction_20261028_zh.md)。
- **运行耗时**：已有 CPU 和 FPGA 预处理的板端计时记录。两组采用不同的缩放算法和实时画面，因此应作为两种配置的测试结果阅读，不能直接当作同算法加速比。[查看测试条件与数据](report/preproc_ab_20261028_zh.md)。

这些结果各有对应的测试条件：固定输入 Golden 来自独立测试程序，不能代替整条视频链的所有验证；构建时的外部 I/O、CDC 和 VDMA 告警也如实保留在[告警说明](report/warning_review_zh.md)中。

## 仓库里有什么？

如果是第一次看这个项目，建议从[设计报告](report/design_report_zh.md)开始；需要找比赛材料或具体证据时，可以查看[材料索引](report/contest_evidence_index_zh.md)。

| 目录 | 内容 |
| --- | --- |
| [`src/`](src/) | FPGA 工程、RTL、IP 和 ARM 应用源码 |
| [`sim/`](sim/) | 仿真脚本、测试记录与说明 |
| [`build/`](build/) | 构建脚本、综合与实现报告 |
| [`board/`](board/) | 上板文件、运行日志和板端对比数据 |
| [`data/`](data/) | 模型、测试输入和参考结果 |
| [`skill/`](skill/) | 开发过程中整理的可复用工具与方法 |
| [`report/`](report/) | 设计报告、复现说明与 AI 协作记录 |
| [`tools/`](tools/) | 文件、构建产物和日志核验脚本 |
| [`reference/`](reference/) | 参考源码和固定版本的 Linux 源码 |
| [`original/`](original/) | 保留原始大小写的工程及材料快照 |

## 想自己复现？

FPGA 工程使用 **Vivado 2022.2**。获取仓库需要 Git LFS；检查脚本需要 Python 3.10 或更新版本。

```sh
git lfs install
git clone https://github.com/Zyl-stack/eagleeye-z035-contest.git
cd eagleeye-z035-contest
git lfs pull
python -B tools/verify_submission.py
```

仓库中的展开文件使用小写命名，而原工程部分脚本和约束依赖原始大小写。请按照[复现指南](report/reproduction_zh.md)，先使用 `tools/restore_a_repro.py` 将原始快照恢复到**新的独立目录**，再运行 Vivado 构建脚本。ARM 和 Linux 构建还需要匹配的交叉工具链、sysroot 与依赖。

已有的[干净克隆复现记录](report/clean_reproduction_20261028_zh.md)保存了命令、日志和产物：该次构建在布线期间中断，随后从检查点续跑生成了 BIT/XSA。请使用独立实验环境，不覆盖稳定启动卡。

使用 Vivado 2025.2 的话，可以参考[实验性迁移说明](build/vivado_2025_2_zh.md)；目前尚未在该版本上完成实际验证。

## 还有一些可以复用的东西

开发过程中，我们把接口检查、Golden 对比、视频流验证和工程复现等经验整理到了 [Skill 目录](skill/index.md)。其中 PYNQ 示例已完成离线模拟测试，暂时没有 PYNQ 实板或真实跨项目验证记录。

项目中的模型及第三方 IP 来源列在[来源与授权说明](report/third_party_sources_zh.md)中；部分权重和 IP 的比赛使用及公开再分发权限仍需核对。

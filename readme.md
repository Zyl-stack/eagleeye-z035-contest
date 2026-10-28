# EagleEye Z035 竞赛工程

本仓库整理自 `eagleeye_z035` 本地交付目录，包含 Z035 摄像头视频、FPGA 预处理、PicoDet ARM 推理、ROI 和舵机相关代码及现有证据。**本次整理和上传不等于重新完成综合或板端验收。**

| 目录 | 内容 |
|---|---|
| `src/` | RTL、IP、FPGA 工程源、PS 应用源码 |
| `sim/` | 仿真与验证材料 |
| `build/` | 构建脚本与原实现报告 |
| `board/` | 现有上板资料及历史日志 |
| `data/` | 模型、测试输入与参考结果 |
| `skill/` | 可复用技能与使用说明 |
| `report/` | 设计报告材料、协作记录和待补证据 |
| `original/eagleeye_z035.zip` | 原文件夹 226 个文件的完整快照，保留原大小写 |
| `tools/` | 本仓库完整性与来源核验脚本 |
| `reference/` | 独立的 A/B/C 和 Linux 分支参考源码及版本边界，不代表与主工程已联调 |

## 复现与大小写

仓库中展开的文件名已转成小写以符合目录命名要求；工程脚本、约束和板端启动文件可能依赖原大小写，**不要直接使用小写副本重建或写卡**。请先按 [复现指南](report/reproduction_zh.md) 用 `tools/restore_a_repro.py` 恢复保留原始大小写的 A 快照，再使用 `build/build_full_vivado.tcl` 在新的独立输出目录构建。原始快照未被覆盖。

```sh
python tools/verify_submission.py
```

工具检查仓库文件哈希、原快照 CRC、源文件数量及展开副本与原件的对应关系，不代替 Vivado 构建、仿真或板端验证。工程使用 Vivado 2022.2；匹配的 Paddle-Lite 库在 A 增量快照中，PetaLinux 交叉编译工具链和 sysroot 仍需外部准备。

## 实测和缺失材料

`board/` 同时包含历史资料，`build/independent_rebuild_20261027/` 包含独立重建报告。当前版本同条件性能数据、完整设计报告及部分验证证据仍需补充，见 `report/submission_audit.md`。历史日志不能直接称为本次构建结果；HDMI 刷新率也不等于检测更新 FPS。

已找到的历史 ARM98 逐层计时及 V2/R3 Golden 和模型计时记录见 [实板证据补充](report/board_evidence_20261027.md)，可使用 `python tools/verify_field_evidence.py` 复核；它们不是当前视频版本的同条件实测。

A 于 2026-10-28 提供了复现 BIT、实验 BOOT 和 U-Boot 至 Linux 登录的串口记录；BIT 哈希与独立构建结果一致。现场功能正常为 A 的反馈，证据边界见 [复现 BIT 的上板记录](report/board_reproduced_20261028_zh.md)。

原材料含模型、第三方 IP、启动镜像及历史日志；当前仓库为私有。公开或比赛提交前需确认版权授权、路径中的个人信息和内容完整性。不要覆盖稳定 SD 卡。

本次补充的材料及仍缺证据见 [提交核查](report/submission_audit.md)、[中文设计报告材料稿](report/design_report_zh.md)、[复现指南](report/reproduction_zh.md)、[技能索引](skill/index.md) 和 [A/B/C/Linux 参考说明](reference/readme.md)。完整 Linux 源码归档使用 Git LFS；克隆后需运行 `git lfs pull`。

## 2026-10-27 A 复现补充

A 提供的稳定版源码、Paddle-Lite 构建依赖和修订 Vivado Tcl 可从保留大小写的增量快照恢复，步骤与验证边界见 [A 复现补充](report/a_supplement_results_20261027.md)。原工程快照保持不变。

## 足球 B→C 联调

当前使用 PicoDet；足球 YOLO 与 ONNX 移植是独立参考路线。接口和 PC 样例验证见 [B 检测联调说明](report/b_detection_integration_20261027.md)。

## PicoDet + C 候选

独立候选源码、构建及现场反馈边界见 [PicoDet + C 候选说明](src/picodet_c_candidate/readme.md)。

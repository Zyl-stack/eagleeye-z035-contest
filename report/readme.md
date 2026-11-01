# EagleEye 比赛报告与资料导航

本目录汇集系统设计、AI 协作、工程验证和复现资料，可按下列顺序阅读。

## 1. 正式设计报告

- [设计报告](design_report_zh.md)：选题背景、设计贡献、系统架构、软硬件分工、性能与资源结果。
- [比赛材料与证据索引](contest_evidence_index_zh.md)：按比赛要求查找源码、报告和验证资料。

## 2. AI 协作记录

- [AI 协作主报告](ai_collaboration_record.md)：提示词、模型回答、自我纠错与工程工作流。
- [补充案例](evidence/ai_collaboration_supplement_20261031.md)、[原始会话摘录](evidence/ai_collaboration_excerpts_20261031.md)和[证据索引](evidence/ai_collaboration_evidence_20261030.md)：历史实践和可追溯来源。
- [技能包](../skill/index.md)：可复用工具、示例、使用条件与验证方法。

## 3. 工程验证材料

- [仿真与验证说明](../sim/verification_zh.md)；[独立板端预处理 Golden](evidence/preproc_board_golden_20261029_zh.md)。
- [复现 BIT 上板记录](evidence/board_reproduced_20261028_zh.md)；[应用日志与功能确认](evidence/board_app_evidence_20261028_zh.md)。
- [CPU/FPGA 预处理计时](evidence/preproc_ab_20261028_zh.md)；[Vivado 干净复现、资源与时序](evidence/clean_reproduction_20261028_zh.md)；[工程告警说明](evidence/warning_review_zh.md)。

板端固定输入 Golden 与实时 CPU/FPGA 计时分别使用各自记录的输入和算法条件。

## 4. 工程复现与接口

- [复现指南](reproduction_zh.md)：恢复工程快照、Vivado 2022.2、仿真和 Linux 构建输入。
- [硬件与软件接口表](evidence/hardware_interfaces_zh.md)；[实测版本身份](evidence/release_identity_zh.md)；[现场证据范围](evidence/field_evidence_needed_zh.md)。
- [原始项目说明](source_readme.md)和[原始文件哈希](source_sha256sums.txt)保留源码来源。

## 5. 补充技术资料

- [Lanczos 技术说明（Word）](lanczos_fpga_brief_20261030.docx)；[历史独立仿真及适用范围](../sim/lanczos_legacy_readme.md)。
- [历史独立实验](evidence/board_evidence_20261027.md)；[历史计时记录](evidence/board_measurements.md)。独立实验与当前视频系统分别记录。
- [图示](figures/)；[第三方来源与许可](third_party/)。

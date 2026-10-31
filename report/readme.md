# EagleEye 比赛报告与资料导航

本目录包含设计报告、复现说明和验证资料。**阅读比赛材料时，先看下方的报告正文与证据索引**；其他文件主要用于查看原始结果、版本边界和历史过程。

## 报告正文与主要入口

| 材料 | 内容 |
|---|---|
| [设计报告](design_report_zh.md) | 选题背景、设计贡献、系统原理、软硬件分工、验证、性能与复现。 |
| [比赛材料与证据索引](contest_evidence_index_zh.md) | 按比赛要求查找源码、仿真、实现报告、板端资料和 Skill。 |
| [复现指南](reproduction_zh.md) | 获取源码、恢复原始工程、Vivado 2022.2 构建及仿真步骤。 |
| [AI 协作主报告](ai_collaboration_record.md) | 提示词、模型回答、纠错轨迹与工作流；[补充案例](ai_collaboration_supplement_20261031.md)和[来源摘录](ai_collaboration_excerpts_20261031.md)提供历史实践和证据。 |
| [技能包](../skill/index.md) | 可复用工具、最小示例、使用方法及适用条件。 |

## 主要验证和版本资料

- **板端与正确性**：[上板记录](board_reproduced_20261028_zh.md) · [应用日志与功能确认](board_app_evidence_20261028_zh.md) · [预处理板端 Golden 对比](preproc_board_golden_20261029_zh.md) · [仿真与验证说明](../sim/verification_zh.md)。
- **性能与硬件实现**：[CPU/FPGA 预处理计时](preproc_ab_20261028_zh.md) · [Vivado 干净复现](clean_reproduction_20261028_zh.md) · [时序与告警说明](warning_review_zh.md)。
- **版本与来源**：[实测版本身份](release_identity_zh.md) · [硬件与软件接口表](hardware_interfaces_zh.md) · [第三方来源与授权](third_party/)。
- **Lanczos 补充材料**：[技术说明（Word）](lanczos_fpga_brief_20261030.docx) · [历史独立仿真记录及适用范围](../sim/lanczos_legacy_readme.md)。

项目分别完成了 FPGA 预处理的独立板端 Golden 一致性验证，以及 CPU/FPGA 两种预处理配置的性能测试。各项测试的输入、算法、版本和结果详见对应验证报告。



项目建立了覆盖源码、构建、仿真、板端测试及版本管理的资料体系。

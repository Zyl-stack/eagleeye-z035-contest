# EagleEye 比赛报告与资料导航

本目录包含设计报告、复现说明和验证资料。**阅读比赛材料时，先看下方的报告正文与证据索引**；其他文件主要用于查看原始结果、版本边界和历史过程。

## 报告正文与主要入口

| 材料 | 内容 |
|---|---|
| [设计报告](design_report_zh.md) | 选题背景、设计贡献、系统原理、软硬件分工、验证、性能与复现。 |
| [比赛材料与证据索引](contest_evidence_index_zh.md) | 按比赛要求查找源码、仿真、实现报告、板端资料和 Skill。 |
| [复现指南](reproduction_zh.md) | 获取源码、恢复原始工程、Vivado 2022.2 构建及仿真步骤。 |
| [AI 协作记录](ai_collaboration_record.md) | 工程问题、真实会话摘录和验证案例；[新增案例](ai_collaboration_supplement_20261031.md)及[来源摘录](ai_collaboration_excerpts_20261031.md)补充显示、交互与机械眼实践。 |
| [技能包](../skill/index.md) | 可复用工具、最小示例、使用方法及适用条件。 |

## 主要验证和版本资料

- **板端与正确性**：[上板记录](board_reproduced_20261028_zh.md) · [应用日志与功能确认](board_app_evidence_20261028_zh.md) · [预处理板端 Golden 对比](preproc_board_golden_20261029_zh.md) · [仿真与验证说明](../sim/verification_zh.md)。
- **性能与硬件实现**：[CPU/FPGA 预处理计时](preproc_ab_20261028_zh.md) · [Vivado 干净复现](clean_reproduction_20261028_zh.md) · [时序与告警说明](warning_review_zh.md)。
- **版本与来源**：[实测版本身份](release_identity_zh.md) · [硬件与软件接口表](hardware_interfaces_zh.md) · [第三方来源与授权](third_party/)。
- **Lanczos 补充材料**：[技术说明（Word）](lanczos_fpga_brief_20261030.docx) · [历史独立仿真记录及适用范围](../sim/lanczos_legacy_readme.md)。

项目分别完成了 FPGA 预处理的独立板端 Golden 一致性验证，以及 CPU/FPGA 两种预处理配置的性能测试。各项测试的输入、算法、版本和结果详见对应验证报告。

本次文档表述调整的逐项前后对照见[文档语言修订记录](document_language_revision.md)。

## 历史记录和补充材料

以下文件保留作过程记录，可能描述当时状态；了解当前情况请优先使用上面的主要入口：

| 类别 | 文件 |
|---|---|
| 早期工程复现 | [复现检查表](a_repro_checklist_20261027.md) · [复现状态](a_repro_status_20261027.md) · [补充结果](a_supplement_results_20261027.md) |
| 早期板端记录 | [历史板端证据](board_evidence_20261027.md) · [历史计时说明](board_measurements.md) |
| 提交核查 | [检查表](submission_check_20261028_zh.md) · [早期核查](submission_audit.md) · [现场证据范围](field_evidence_needed_zh.md) |
| 原始说明和草稿 | [英文报告草稿](design_report_draft.md) · [原始项目说明](source_readme.md) · [原始文件哈希](source_sha256sums.txt)；早期说明保留在原工程归档中 |
| 图和许可文件 | [`figures/`](figures/) · [`third_party/`](third_party/) |

项目建立了覆盖源码、构建、仿真、板端测试及版本管理的资料体系，通过工程归档与原始记录支持成果追溯。授权核对、历史仿真版本及工程告警集中在各专题页说明。

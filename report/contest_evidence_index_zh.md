# 比赛材料与证据索引（2026-10-29）

本索引按比赛 3.3.5 的工程包、Skill 和设计报告要求组织材料，连接源码、构建产物、验证结果及版本记录。提交格式与评分以赛事完整规则为准。

| 要求 | 可直接查阅的材料 | 结果及边界 |
|---|---|---|
| 工程与源码 | [仓库说明](../readme.md)、[源码复现步骤](reproduction_zh.md)、[`src/`](../src/)、[Linux 源码](../reference/linux_z035/readme.md) | 含 Eyes RTL/BD/XDC、预处理、PicoDet 应用、注意力候选及 Linux 固定源码；原始快照保留大小写。已有第三方许可见[许可文本](third_party/)，具体授权范围按各组件核对。 |
| 仿真或验证 | [仿真说明](../sim/verification_zh.md)、[原始预处理仿真日志](../sim/preproc_iverilog_20261027.log)、[舵机仿真日志](../sim/servo_iverilog_20261027.log)、[固定输入板端 Golden](evidence/preproc_board_golden_20261029_zh.md) | 五类帧仿真逐位一致；一次独立板端固定输入比较 307,200 值逐位一致。Lanczos 历史独立工程通过日志和旧失败日志分别保留，各自绑定历史测试版本，见[记录范围](../sim/lanczos_legacy_readme.md)。 |
| 综合与实现 | [干净复现报告](evidence/clean_reproduction_20261028_zh.md)、[资源报告](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序报告](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[告警说明](evidence/warning_review_zh.md) | Vivado 2022.2 实现记录含资源和 WNS/WHS；WNS/WHS 只覆盖已约束路径。外部 I/O、CDC 和方法学专项检查仍待闭环，当前 STA 结果覆盖已约束路径。 |
| 上板、运行及实测 | [现场团队的 BIT 和启动记录](evidence/board_reproduced_20261028_zh.md)、[应用日志与功能确认](evidence/board_app_evidence_20261028_zh.md)、[CPU/FPGA 预处理 A/B 计时](evidence/preproc_ab_20261028_zh.md)、[板端 Golden](evidence/preproc_board_golden_20261029_zh.md) | 现场确认六项功能正常；应用日志含 VDMA 错误位提示、帧新鲜度未验证。独立板端固定输入 Golden 通过；A/B 两组缩放算法及实时帧不同，按两种系统配置的计时结果报告。 |
| 可复现构建 | [复现说明](reproduction_zh.md)、[Vivado 构建 Tcl](../build/build_full_vivado.tcl)、[干净复现记录](evidence/clean_reproduction_20261028_zh.md) | 干净克隆构建在布线中断后从检查点续跑完成，生成 BIT/XSA；工具链、许可证和部分运行依赖需另行准备。独立重建内核与现场镜像一致性尚未证明。 |
| Skill | [技能索引](../skill/index.md)、[独立复用测试](../sim/skill_portability_20261028.txt)、[工具验证记录](../skill/validation.md)、[PYNQ 模拟测试](../sim/pynq_skill_mock_20261028.txt) | 技能按用途统一整理，已提供运行示例、适用范围和验证边界。PYNQ 已完成离线模拟，实板与真实跨项目复用为后续验证项。 |
| 设计报告 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)、[版本说明](evidence/release_identity_zh.md) | 有背景、相关工作、原理、分工、性能、资源和复现说明；AI 协作主报告整合了八个典型工程案例，涵盖提示词、模型回答、自我纠错轨迹和智能体工作流，并提供可追溯的原始会话及工程验证依据；性能按配置条件比较，资源按各构建版本列示。 |

## 已保存结果的复核方法

2026-10-29 在仓库目录运行以下只读核验，均退出码 0；没有运行开发板或重新综合：

```text
python -B tools/verify_preproc_sim_capture.py
Icarus captured DMA: compared=307200 mismatches=0; NOT BOARD DATA

python -B tools/verify_clean_rebuild_20261028.py
PASS: manifest, BIT configuration, XSA BIT and HWH timestamp comparison
NOTE: configuration equality is not board validation of the new BIT

python -B tools/summarize_preproc_ab.py board/measurements/preproc_ab_20261028
```

第三条从已保存日志重算：两组各记录 35 帧、去除前 5 帧；CPU 双三次预处理平均 416.960 ms，FPGA 双线性预处理平均 17.537 ms，`total_ms` 分别为 903.150 ms 和 502.665 ms。完整输入和输出见[A/B 记录](evidence/preproc_ab_20261028_zh.md)；该核验重算已保存日志，输入配对、张量一致性及硬件帧完成按专项记录核验。

## 测试条件与版本范围

1. 独立预处理板端 Golden 覆盖一次固定输入；独立 BIT 哈希及跨版本关联待核对，完整视频链按对应版本的记录评估。
2. 按实际需要处理或说明外部 I/O、CDC/时钟及 VDMA 告警；尚无已完成签核或帧新鲜度验证记录。
3. 演示程序、现场功能确认和原始日志按[版本说明](evidence/release_identity_zh.md)绑定，独立注意力候选与 AttentionPolicy 分别列示。
4. 模型权重及部分厂商 IP 的比赛使用和再分发授权仍待核对；模型来源说明按[第三方资料](third_party/)记录的来源和证据范围引用。
5. PYNQ 以离线模拟示例提交；实板与真实跨项目复用记录作为可选扩展。

BIT、BOOT、启动日志、Attention 应用日志、功能确认、模型及 A/B 计时均可从上表直接查阅。
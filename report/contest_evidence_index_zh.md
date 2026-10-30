# 比赛材料与证据索引（2026-10-29）

本索引按已收到的比赛 3.3.5 截图整理。它是材料导航和验证范围说明，不代替赛事组织方对完整规则、格式与评分的审核。原始文件、失败记录和 A 的功能确认均保留。

| 要求 | 可直接查阅的材料 | 结果及边界 |
|---|---|---|
| 工程与源码 | [仓库说明](../readme.md)、[源码复现步骤](reproduction_zh.md)、[`src/`](../src/)、[Linux 源码](../reference/linux_z035/readme.md) | 含 Eyes RTL/BD/XDC、预处理、PicoDet 应用、C 候选及 Linux 固定源码；原始快照保留大小写。第三方组件的权利状态见[来源与授权](third_party_sources_zh.md)。 |
| 仿真或验证 | [仿真说明](../sim/verification_zh.md)、[原始预处理仿真日志](../sim/preproc_iverilog_20261027.log)、[舵机仿真日志](../sim/servo_iverilog_20261027.log)、[固定输入板端 Golden](preproc_board_golden_20261029_zh.md) | 五类帧仿真逐位一致；新增一次独立板端固定输入比较，307,200 值逐位一致，测试 BOOT 与 BIT 绑定限制见原记录。旧 [Lanczos 日志](../sim/lanczos_legacy_xsim.txt)有失败，版本对应关系未确认。 |
| 综合与实现 | [干净复现报告](clean_reproduction_20261028_zh.md)、[资源报告](../build/clean_rebuild_20261028/reports/impl_utilization.rpt)、[时序报告](../build/clean_rebuild_20261028/reports/timing_summary.rpt)、[告警说明](warning_review_zh.md) | Vivado 2022.2 实现记录含资源和 WNS/WHS；WNS/WHS 只覆盖已约束路径。外部 I/O、CDC 和方法学告警尚未闭环，不能称为完整时序签核。 |
| 上板、运行及实测 | [A 的 BIT 和启动记录](board_reproduced_20261028_zh.md)、[应用日志与功能确认](board_app_evidence_20261028_zh.md)、[CPU/FPGA 预处理 A/B 计时](preproc_ab_20261028_zh.md)、[板端 Golden](preproc_board_golden_20261029_zh.md) | A 确认六项功能正常；应用日志含 VDMA 错误位提示、帧新鲜度未验证。独立板端固定输入 Golden 通过；A/B 两组缩放算法及实时帧不同，不是同算法精度或加速比结论。 |
| 可复现构建 | [复现说明](reproduction_zh.md)、[Vivado 构建 Tcl](../build/build_full_vivado.tcl)、[干净复现记录](clean_reproduction_20261028_zh.md) | 干净克隆构建在布线中断后从检查点续跑完成，生成 BIT/XSA；工具链、许可证和部分运行依赖需另行准备。独立重建内核与现场镜像一致性尚未证明。 |
| Skill | [技能索引](../skill/index.md)、[独立复用测试](../sim/skill_portability_20261028.txt)、[PYNQ 模拟测试](../sim/pynq_skill_mock_20261028.txt) | 已提供运行示例、适用范围和边界；PYNQ 没有实板及真实跨项目验证，不宣称加分已取得。 |
| 设计报告 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)、[版本说明](release_identity_zh.md) | 有背景、相关工作、原理、分工、性能、资源和复现说明；AI 协作材料为整理稿。现有 CPU/FPGA 计时和两份硬件构建不能分别冒充同算法加速比和资源优化前后对照。 |

## 本地已有结果复核

2026-10-29 在仓库目录运行以下只读核验，均退出码 0；没有运行开发板或重新综合：

```text
python -B tools/verify_preproc_sim_capture.py
Icarus captured DMA: compared=307200 mismatches=0; NOT BOARD DATA

python -B tools/verify_clean_rebuild_20261028.py
PASS: manifest, BIT configuration, XSA BIT and HWH timestamp comparison
NOTE: configuration equality is not board validation of the new BIT

python -B tools/summarize_preproc_ab.py board/measurements/preproc_ab_20261028
```

第三条从已保存日志重算：两组各记录 35 帧、去除前 5 帧；CPU 双三次预处理平均 416.960 ms，FPGA 双线性预处理平均 17.537 ms，`total_ms` 分别为 903.150 ms 和 502.665 ms。完整输入和输出见[A/B 记录](preproc_ab_20261028_zh.md)；该核验只重算原日志，不证明相同输入、张量一致或板端每帧完成。

## 仍需核对或取得新证据

1. 已收到一次独立预处理固定输入板端 Golden 通过记录；独立 BIT 哈希未核实，不能推断与此前上板 BIT 身份相同，也不能推广为所有输入或完整视频链通过。
2. 按实际需要处理或说明外部 I/O、CDC/时钟及 VDMA 告警；尚无已完成签核或帧新鲜度验证记录。
3. 明确最终演示选用的应用版本，按[版本说明](release_identity_zh.md)绑定证据；已有 A 的功能确认不能与其他程序的原始日志混用。
4. 模型权重及部分厂商 IP 的比赛使用和再分发授权仍待核对；当前机器无法独立读取团队反馈中另一台机器的来源文件。
5. PYNQ 实板与真实跨项目验证属于可选补充；现无记录，模拟候选如实提交。

已有 BIT、BOOT、启动日志、Attention 应用日志、功能确认、模型和 A/B 计时不应重复索要。

# 比赛提交材料检查（2026-10-28）

按已提供的 3.3.5 截图核对，最终格式和评分以赛事组织方完整规则为准。本文件索引已有结果，不构造新的上板结论。

逐项材料入口及本地复核结果见 [比赛材料与证据索引](contest_evidence_index_zh.md)。后续提供的一次[独立板端同输入 Golden](preproc_board_golden_20261029_zh.md)逐位对比通过；时序与告警尚未签核；团队关于模型来源的反馈见 [来源与授权](third_party_sources_zh.md)。

| 要求 | 仓库现有内容 | 保留事项 |
|---|---|---|
| 工程与源码 | Eyes 源码、PicoDet 与注意力应用、原始源码快照及 Linux 固定源码；根目录和 `original/` 说明来源 | 已整理 [实测版本](release_identity_zh.md)和 [第三方来源/许可核对](third_party_sources_zh.md)；最终演示应用选择及未找到的权重和部分 IP 再分发授权仍需团队确认，原始源文件保留。 |
| 仿真和正确性 | [仿真说明](../sim/verification_zh.md)与对应 Golden 和日志；另有[独立板端 Golden 对比](preproc_board_golden_20261029_zh.md) | 一次固定输入逐位通过；本次 BIT 独立哈希未核实。Lanczos 历史独立工程通过日志和旧失败日志分别保留，与当前 Eyes 的版本关系尚未完整核实。 |
| 资源与时序 | [干净复现记录](clean_reproduction_20261028_zh.md)保留 Vivado 日志、报告、BIT/XSA；[告警说明](warning_review_zh.md)按类型汇总 | WNS/WHS 为已约束路径结果；I/O、CDC 和方法学问题仍未闭环。 |
| 上板及运行 | [启动证据](board_reproduced_20261028_zh.md)、[Attention 应用日志及现场六项功能确认](board_app_evidence_20261028_zh.md)，另有独立注意力候选现场功能确认 | 应用日志含 VDMA 警告与 `freshness=unverified`，不否定现场功能确认；已有 CPU/FPGA A/B 计时，但缩放算法和实时帧不同，见 [实板结果](preproc_ab_20261028_zh.md)。 |
| 构建复现 | [复现步骤](reproduction_zh.md)、`build/build_full_vivado.tcl`、源码及归档校验脚本；干净克隆后从中断的检查点续跑生成 BIT/XSA | 所需 Vivado、交叉工具链、sysroot 和设备环境不全部随仓库提供；新内核与现场镜像二进制一致性未证明。 |
| Skill | [Skill 索引](../skill/index.md)：FP32 比较、ROI 接口检查、PYNQ 示例 | PYNQ 示例仅模拟对象验证；团队确认无 PYNQ 实板及真实跨项目复用资料，按模拟候选提交，不列为需要队友补交的既有材料，也不宣称获得加分。 |
| 设计报告和 AI 协作 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)及图示、参考资料 | 团队主要分工已按团队确认记录；仍需核对具体贡献与引用，AI 记录为整理稿而非完整逐字对话。 |

## 本地可重复检查

在完整克隆并执行 `git lfs pull` 后运行：

```sh
python -B tools/verify_submission.py
python -B tools/verify_clean_rebuild_20261028.py
python -B tools/check_submission_links.py
python -B -m unittest discover -s skill/preproc-fp32-golden/tests -v
python -B -m unittest discover -s skill/roi-review-control/tests -v
python -B -m unittest discover -s skill/pynq-overlay-dma/tests -v
```

链接检查只检查报告、仿真和 Skill 的 Markdown 行内本地文件路径，不检查网页可用性、段落锚点、代码块或所有脚本内部路径。文件哈希与主机测试不等于实板验证。

现场已确认六项功能正常，且 BIT、BOOT、启动串口、应用日志及独立固定输入板端 Golden 已收到，不再要求重复提供。该 Golden 不能证明 A/B 计时两组使用同算法或相同帧，也不能代替视频帧新鲜度验证。

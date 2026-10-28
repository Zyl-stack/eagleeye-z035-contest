# 比赛提交材料检查（2026-10-28）

按已提供的 3.3.5 截图核对，最终格式和评分以赛事组织方完整规则为准。本文件索引已有结果，不构造新的上板结论。

| 要求 | 仓库现有内容 | 保留事项 |
|---|---|---|
| 工程与源码 | Eyes 源码、PicoDet/C 应用、原始源码快照及 Linux 固定源码；根目录和 `original/` 说明来源 | 最终演示版本和第三方使用授权由团队确认；原始源文件保留。 |
| 仿真和正确性 | [仿真说明](../sim/verification_zh.md)与对应 Golden 和日志；预处理、舵机、AXI-Lite、主机/QEMU 等按各自范围记录 | 当前板端预处理输出与 Golden 对比无结果；旧 Lanczos 测试报告失败，版本关系未核实。 |
| 资源与时序 | [干净复现记录](clean_reproduction_20261028_zh.md)保留 Vivado 日志、报告、BIT/XSA；[告警说明](warning_review_zh.md)按类型汇总 | WNS/WHS 为已约束路径结果；I/O、CDC 和方法学问题仍未闭环。 |
| 上板及运行 | [启动证据](board_reproduced_20261028_zh.md)、[Attention 应用日志及 A 六项功能确认](board_app_evidence_20261028_zh.md)，另有独立 C 候选现场功能确认 | 应用日志含 VDMA 警告与 `freshness=unverified`，不否定 A 的功能确认；当前系统同条件 CPU/FPGA 性能对比仍无。 |
| 构建复现 | [复现步骤](reproduction_zh.md)、`build/build_full_vivado.tcl`、源码及归档校验脚本；干净克隆后从中断的检查点续跑生成 BIT/XSA | 所需 Vivado、交叉工具链、sysroot 和设备环境不全部随仓库提供；新内核与现场镜像二进制一致性未证明。 |
| Skill | [Skill 索引](../skill/index.md)：FP32 比较、ROI 接口检查、PYNQ 示例 | PYNQ 项目仅模拟对象验证，无 PYNQ 实板及真实跨项目复用证据；不宣称获得加分。 |
| 设计报告和 AI 协作 | [设计报告](design_report_zh.md)、[AI 协作记录](ai_collaboration_record.md)及图示、参考资料 | 团队确认分工、贡献及引用；AI 记录为整理稿而非完整逐字对话。 |

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

A 已确认六项功能正常，且 BIT、BOOT、启动串口及应用日志已经收到，不再要求重复提供。未来如要证明精确张量和加速比，需实际取得对应新测试数据；没有则保持无结果。

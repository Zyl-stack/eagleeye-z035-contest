# 本次复现工程告警说明

依据 2026-10-28 干净克隆并在布线中断后从检查点续跑生成的 [Vivado 2022.2 报告](../build/clean_rebuild_20261028/reports/)，不改写 RTL 或约束。现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能上板正常；以下是工具和日志的独立记录，不否定这一确认，也不能用现场正常直接替代时序签核。

| 记录 | 已知事实 | 影响与目前结论 |
|---|---|---|
| Methodology 8 项 Critical | [`methodology.rpt`](../build/clean_rebuild_20261028/reports/methodology.rpt)：`TIMING-4` 1 项、`TIMING-6` 3 项、`TIMING-7` 3 项、`TIMING-27` 1 项。报告称 `aclk` 在 `clk_fpga_0` 时钟树下被重新定义，并指出 `aclk`/`clk_fpga_0`、`ov5640_pclk`/`clk_fpga_1` 的关联时钟问题。 | 时钟约束和时序关系尚需根据完整时钟树、时钟域设计和工具路径报告复核。此处未修改 XDC，也不能由 WNS 为正推断问题消失。 |
| CDC Critical | [`cdc.rpt`](../build/clean_rebuild_20261028/reports/cdc.rpt) 中 `clk_fpga_0→aclk`、`aclk→clk_fpga_0` 和 `ov5640_pclk→clk_fpga_1` 列为 Critical；报告各自还包含 safe、unsafe、unknown 细分。 | 需要分别审查约束建模和跨域实现；不能把汇总行数直接当作实际失效次数，也不能因有同步器就认定所有 crossing 均安全。 |
| 外部 I/O 时序 | `timing_summary.rpt` 列出 12 项输入和 33 项输出缺少延迟约束；Methodology 的 `TIMING-18` 明细包含摄像头数据/同步、按键、复位及舵机输出等。 | 需区分同步外设接口与异步控制信号，并以传感器/显示器件时序、板级走线与实际时钟为依据；不猜填 delay，也不把这 45 项全部自动归为同一问题。 |
| 其他 DRC/方法学 | [`drc.rpt`](../build/clean_rebuild_20261028/reports/drc.rpt) 有 65 Warning、4 Advisory；`methodology.rpt` 另有复位、同步链和约束查询等 Warning。 | 类别和数量是对应构建的工具输出，尚未逐项完成设计审查；不能仅按告警数目判断板端功能是否正常。 |
| VDMA 应用日志提示 | [应用日志](../board/reproduced_20261028/attention_app_20261028.log) 记录 `s2mm=0x15810`、`mm2s=0x11000`、`cleared=0`，并打印 `WARNING VDMA error bits present`；179 次快照标为 `freshness=unverified`。 | 应用据源码掩码 `0x00000ff0` 检查错误位；日志不能单独确定产生原因、发生时间或对已确认功能的实际影响。它不是板端张量 Golden 或帧完成证明。 |

上述问题是当前资料中的技术边界。若后续需要修改或签核，需使用匹配版本工程、板级资料及受控实测；本次仅整理已有证据，不声称问题已修复。

团队后续确认：已有外部 I/O 延迟约束缺口记录，但没有板级 I/O 时序签核；CDC 和其他时钟告警也没有完成处理与签核的记录。VDMA 错误位在既有板端记录及 A/B 测速中有提示，测速时未清除；其原因和每次读取均为最新完整帧尚未证明。这是现场反馈及已有记录的范围，不改变 现场功能正常的确认。

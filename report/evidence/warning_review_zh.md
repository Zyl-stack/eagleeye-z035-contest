# Eyes 工程告警与签核状态

本页汇总 2026-10-28 干净克隆并从布线检查点续跑得到的 [Vivado 2022.2 报告](../../build/clean_rebuild_20261028/reports/)及应用日志中的工程注意事项。现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 功能正常；板级时序与帧完整性签核仍按以下专项状态管理。

| 记录 | 已知事实 | 影响与目前结论 |
|---|---|---|
| Methodology 8 项 Critical | [`methodology.rpt`](../../build/clean_rebuild_20261028/reports/methodology.rpt)：`TIMING-4` 1 项、`TIMING-6` 3 项、`TIMING-7` 3 项、`TIMING-27` 1 项。报告称 `aclk` 在 `clk_fpga_0` 时钟树下被重新定义，并指出 `aclk`/`clk_fpga_0`、`ov5640_pclk`/`clk_fpga_1` 的关联时钟问题。 | 时钟约束和时序关系尚需根据完整时钟树、时钟域设计和工具路径报告复核。当前 XDC 保持原样，专项签核待完成；WNS 的适用范围为已约束路径。 |
| CDC Critical | [`cdc.rpt`](../../build/clean_rebuild_20261028/reports/cdc.rpt) 中 `clk_fpga_0→aclk`、`aclk→clk_fpga_0` 和 `ov5640_pclk→clk_fpga_1` 列为 Critical；报告各自还包含 safe、unsafe、unknown 细分。 | 需要分别审查约束建模和跨域实现；汇总分类用于定位 crossing，实际影响需逐路径核对同步结构和约束。 |
| 外部 I/O 时序 | `timing_summary.rpt` 列出 12 项输入和 33 项输出缺少延迟约束；Methodology 的 `TIMING-18` 明细包含摄像头数据/同步、按键、复位及舵机输出等。 | 需区分同步外设接口与异步控制信号，并以传感器/显示器件时序、板级走线与实际时钟为依据；这 45 项按接口语义分别核对，延迟值以器件和板级资料为依据。 |
| 其他 DRC/方法学 | [`drc.rpt`](../../build/clean_rebuild_20261028/reports/drc.rpt) 有 65 Warning、4 Advisory；`methodology.rpt` 另有复位、同步链和约束查询等 Warning。 | 类别和数量是对应构建的工具输出，尚未逐项完成设计审查；板端功能按现场测试记录，告警影响按具体类型及路径评估。 |
| VDMA 应用日志提示 | [应用日志](../../board/reproduced_20261028/attention_app_20261028.log) 记录 `s2mm=0x15810`、`mm2s=0x11000`、`cleared=0`，并打印 `WARNING VDMA error bits present`；179 次快照标为 `freshness=unverified`。 | 应用据源码掩码 `0x00000ff0` 检查错误位；产生原因、时间及对功能的实际影响仍待定位；帧完成与张量 Golden 由专项测试核验。 |

上述项目作为已知工程问题保留，修复或签核需匹配版本源码、板级时序资料及受控实测。

团队确认外部 I/O、CDC 和其他时钟告警仍待处理与签核；VDMA 错误位在测速时保持锁存，错误来源及每次读取的最新完整帧属性待验证。现场六项功能正常与这些专项状态分别记录。
# 现场验证范围与工程条件

本页汇总当前测试的版本边界和工程条件；完整结果及原始记录由各专题报告提供。

## 板级系统与视频链路

- **实测身份**：[版本表](release_identity_zh.md)记录 `b286f480...` BIT、配套 BOOT、Attention 应用及模型。实验卡 BOOT 读回清单、板上程序与模型的独立身份记录仍待核对；稳定启动版本与恢复介质按实际部署记录管理。
- **应用与功能**：[应用证据](board_app_evidence_20261028_zh.md)保存 `ATTENTION`、`PREPROC`、`TIME` 日志，以及摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能正常的现场确认。
- **数值与计时**：[固定输入板端 Golden](preproc_board_golden_20261029_zh.md)的 307,200 个值逐位一致，该次独立 BIT 哈希及跨版本关系待核对。[CPU/FPGA A/B](preproc_ab_20261028_zh.md)比较不同缩放算法及先后采集的实时帧；帧新鲜度确认后的检测更新率为专项测试项。
- **时序与数据完整性**：外部 I/O、CDC 和其他时钟告警尚未完成签核；VDMA 错误位在测速时未清除，日志标记 `freshness=unverified`。错误来源及最新完整帧属性仍待定位，具体记录见[告警说明](warning_review_zh.md)。旧 `attach_100.log` 作为历史 CPU 基线单独保存。

## 模型、接口与来源

- 当前检测路线为 ARM 端 PicoDet PERSON；AttentionPolicy 使用本地临时匹配 ID，跨遮挡稳定跟踪由独立测试评估。演示应用变化时同步更新[版本表](release_identity_zh.md)。
- 模型权重及部分第三方 IP 的使用和再分发授权范围见[来源与许可](../third_party/)。团队分工见[设计报告](../design_report_zh.md)。
- ARM98 与 V2/R3 属于[历史独立实板实验](board_evidence_20261027.md)，其固定输入计时与当前视频链分别统计。

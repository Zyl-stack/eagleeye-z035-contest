# 实板应用日志及功能反馈（2026-10-28）

## 回传与核验

现场回传 `eagleeye_z035_repro_with_app_log_20261028.zip`，SHA256 `6e7210373081177ac8f14c0f63833afb4799ccca874509478b20f4cd2524c070`，ZIP CRC 通过；包内 `SHA256SUMS.txt` 的 280 项逐项核验通过。本仓库收录 [应用日志原文](../../board/reproduced_20261028/attention_app_20261028.log)与 [现场结果说明原文](../../board/reproduced_20261028/board_result_app_source_zh.md)，并与已有 BIT、BOOT 和启动串口记录共同归档。

| 资料 | SHA256 | 边界 |
|---|---|---|
| 应用日志原文 | `9bfd5245ad2952fd429a61092e10841dd77bfcc7041eb31720df9891dd69a5e0` | 151456 字节，结束于第 179 次 `TIME`，没有程序结束记录。 |
| 本次结果说明原文 | `6aabb1c2ee192e7d0c9e066ebcf6de1ae9ad9b5a6fbbc7c6a8f3e0973ce4bad9` | 现场记录使用 `b286f480...` BIT、对应 BOOT 和 Attention 应用归档。 |
| Attention 应用归档 | `b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59` | 已存于 `original/a_repro_delta_20261027.zip`。包内可执行文件哈希 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`；模型哈希 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。以上为归档及交接身份，现场运行文件的独立读回哈希仍待核对。 |

[启动串口和 BIT 记录](board_reproduced_20261028_zh.md)与应用日志按现场归档说明关联到同一 BOOT、Attention 归档；两份日志的共同启动会话标识仍待核对。

## 日志实际内容

按行首统计，日志含 179 条 `ATTENTION`、179 条 `PREPROC backend=fpga`、179 条 `TIME` 和 179 条 `SNAPSHOT`；未发现 `PREPROC_FALLBACK`。其中 79 条注意力记录为 `no_player`，100 条含本地 `target_id`。这些记录呈现应用行为，ID 用于相邻推理结果临时匹配；跨遮挡稳定性另需跟踪验证。

179 条 `SNAPSHOT` 均为 `freshness=unverified`；日志明确警告 `WARNING VDMA error bits present; snapshot consistency is not frame-completion proof`。该日志用于分析应用调用与计时；DMA 帧完成、新鲜度及张量 Golden 一致性按各自专项测试记录。

运行 `python -B tools/summarize_board_log.py board/reproduced_20261028/attention_app_20261028.log` 可复核 179 条时间记录：平均捕获 16.290 ms、预处理 18.329 ms、推理 477.883 ms、后处理 0.028 ms、`total` 512.566 ms，`1000 / mean(total)` 为 **1.951 FPS**；日志中 FPS 字段的算术均值为 1.953 FPS，两种统计口径不同。以上为日志计时区间的统计；CPU/FPGA 配置对比见专项报告，HDMI 刷新及经帧新鲜度确认的检测更新率分别测量。日志 RTC 为 2018 年，现场测试日期按回传记录标注。

**应用版本：**该日志对应 AttentionPolicy，未出现 `C_ATTENTION`；归档 ELF 与独立 PicoDet 注意力候选 `1421b51b...` 分别管理。后者的板端功能已有现场确认，其当次原始日志尚待归档。

## 现场功能与后续验证

现场已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能均正常，作为本次功能验证结论。记录来源为现场确认；逐项操作时间及照片/视频未随包归档。应用日志中的 VDMA 提示保留在[告警说明](warning_review_zh.md)，与功能记录共同呈现工程状态。

### 专项验证范围

张量一致性见[独立板端固定输入 Golden](preproc_board_golden_20261029_zh.md)，配置性能见[CPU/FPGA A/B 计时](preproc_ab_20261028_zh.md)；测试条件、版本关系及告警状态集中见[验证范围](field_evidence_needed_zh.md)。独立注意力候选的原始应用日志及长期稳定性为专项验证项。
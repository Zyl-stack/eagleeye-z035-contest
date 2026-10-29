# 实板应用日志及功能反馈（2026-10-28）

## 回传与核验

A 回传 `eagleeye_z035_repro_with_app_log_20261028.zip`，SHA256 `6e7210373081177ac8f14c0f63833afb4799ccca874509478b20f4cd2524c070`，ZIP CRC 通过；包内 `SHA256SUMS.txt` 的 280 项逐项核验通过。本仓库只新增与本次证据相关的 [应用日志原文](../board/reproduced_20261028/attention_app_20261028.log)与 [A 的结果说明原文](../board/reproduced_20261028/board_result_app_source_zh.md)；不需要重复上传包中已经保存的 BIT、BOOT 和启动串口。

| 资料 | SHA256 | 边界 |
|---|---|---|
| 应用日志原文 | `9bfd5245ad2952fd429a61092e10841dd77bfcc7041eb31720df9891dd69a5e0` | 151456 字节，结束于第 179 次 `TIME`，没有程序结束记录。 |
| 本次结果说明原文 | `6aabb1c2ee192e7d0c9e066ebcf6de1ae9ad9b5a6fbbc7c6a8f3e0973ce4bad9` | A 记录使用 `b286f480...` BIT、对应 BOOT 和 Attention 应用归档。 |
| Attention 应用归档 | `b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59` | 已存于 `original/a_repro_delta_20261027.zip`。包内可执行文件哈希 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`；模型哈希 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。这些是归档身份与交接说明，不能替代板上可执行文件和模型的独立读回哈希。 |

旧版 [U-Boot 至登录串口和 BIT 记录](board_reproduced_20261028_zh.md) 已收录。A 的归档说明将本次应用日志与同一 BOOT、Attention 归档关联；两份日志没有足够的共同会话标识可以仅凭文件独立证明同一启动会话。

## 日志实际内容

按行首统计，日志含 179 条 `ATTENTION`、179 条 `PREPROC backend=fpga`、179 条 `TIME` 和 179 条 `SNAPSHOT`；未发现 `PREPROC_FALLBACK`。其中 79 条注意力记录为 `no_player`，100 条含本地 `target_id`。这些是应用记录，ID 为相邻帧临时匹配，不证明遮挡后稳定跟踪。

179 条 `SNAPSHOT` 均为 `freshness=unverified`；日志明确警告 `WARNING VDMA error bits present; snapshot consistency is not frame-completion proof`。因此不能把本次记录表述成每帧 DMA 正常、帧完成或帧新鲜度已验证，也不能将预处理后端日志直接当成张量 Golden 正确的证明。

运行 `python -B tools/summarize_board_log.py board/reproduced_20261028/attention_app_20261028.log` 可复核 179 条时间记录：平均捕获 16.290 ms、预处理 18.329 ms、推理 477.883 ms、后处理 0.028 ms、`total` 512.566 ms，`1000 / mean(total)` 为 **1.951 FPS**；日志中 FPS 字段的算术均值为 1.953 FPS，两种统计口径不同。以上仅为这份日志记录的统计，不是同条件 CPU/FPGA 配对加速比，也不是 HDMI 刷新率或已验证的连续检测更新率。日志中 RTC 为 2018 年，不能用该时钟推断实际测试日期。

日志未出现 `C_ATTENTION`；应用归档所含 ELF 的哈希不同于此前独立 PicoDet+C 候选 `1421b51b...`。因此本应用日志证明的是交接所描述的 AttentionPolicy 应用运行记录，不能单凭它证明独立 PicoDet+C 候选的调用链；用户此前对 C 候选功能正常的反馈单独保留，不与该日志混称。

## 现场功能确认与未测试项目

A 已现场确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能均正常；按用户要求，将 A 的确认作为本次功能验证结论，不再要求重复核验。该结论的来源为 A 的现场确认，本包未包含逐项操作时间、独立功能表或相应照片/视频，因此不添加未经记录的具体数值。原始应用日志中的 VDMA 提示同时保留，不据此否定 A 的功能确认。

A 后续确认：同一固定输入的板端预处理输出与 Golden 逐值对比**未测试、无记录**。此前 307,200 个值逐位一致的结果来自 RTL 仿真，不是板端对比，不能宣称板端逐值 PASS。后续已收到 [CPU/FPGA A/B 计时](preproc_ab_20261028_zh.md)，但两组算法和实时帧不同；同算法固定输入对照、独立 C 候选的原始应用记录、完整时序和长期稳定性证据仍缺。无需重复提供已回传的 BIT、BOOT、启动串口或本次应用日志。

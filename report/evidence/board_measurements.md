# 板端计时与统计口径

`board/attach_100.log` 含 100 条可解析的 `TIME` 记录，统计如下：

复现命令：`python tools/summarize_board_log.py board/attach_100.log`。脚本从日志字段重算统计，硬件身份及配对条件见各次测试说明。

| 历史 CPU 预处理运行指标 | 100 帧平均 |
|---|---:|
| capture | 13.607 ms |
| preprocess | 418.061 ms |
| inference | 481.955 ms |
| postprocess | 0.029 ms |
| total | 913.653 ms |
| FPS 字段算术均值 | 1.09475 FPS |
| 1000 / 平均 total | 1.09451 FPS |

该记录提供历史 CPU 基线；与当前 Eyes FPGA 预处理测试的版本及场景不同，按各自条件列示。`board/attach_100_result.json` 保存第 100 帧结果，均值由完整 100 条记录计算。

当前 Eyes 使用同一板、应用、模型、输入尺寸及线程的 [CPU/FPGA 预处理 A/B 实板日志](preproc_ab_20261028_zh.md)。两组使用不同缩放算法并先后采集实时帧，不是同算法、同一固定输入的精度对照；帧新鲜度和完整视频检测更新率仍未验证。视频画面刷新率与新检测结果更新率是不同指标。

另有历史 ARM98 单线程实板逐层计时和独立 V2/R3 实板 Golden 与固定输入推理记录，原始数据摘录、输出及其验证方法见 [本地实板证据补充](board_evidence_20261027.md)。这些固定输入实验与当前 Eyes 视频测试按各自条件统计。

## Attention 应用计时

2026-10-28 应用日志含 179 条 `TIME`，平均预处理 18.329 ms、平均 total 512.566 ms，`1000 / 平均 total` 为 1.951 FPS；详见 [日志证据及限制](board_app_evidence_20261028_zh.md)。日志含 VDMA 错误位警告且 `freshness=unverified`；该数据属于本次日志计时区间的结果，与历史 CPU 记录分别列示；帧新鲜度确认后的检测更新率仍待测量。

## CPU/FPGA 预处理 A/B 实测

现场团队的实板记录经独立重算，30 帧均值：CPU 双三次预处理 416.960 ms、FPGA 双线性预处理 17.537 ms；`total_ms` 分别为 903.150 ms 与 502.665 ms。两组比较不同算法和实时输入配置；同算法收益及数值一致性由相应专项测试评估。完整条件和原始日志见 [A/B 证据](preproc_ab_20261028_zh.md)。

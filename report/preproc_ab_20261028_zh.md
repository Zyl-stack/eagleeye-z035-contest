# Eyes 预处理 CPU/FPGA A/B 实板结果（2026-10-28）

## 资料来源与条件

现场提供 `Preproc_AB_Result_20261028.zip`，SHA256 `27a45173a5a6e2148cf8abb32f8f77e0ea093180486b833eacee062da7ecad42`；ZIP CRC 通过。包内板端归档 [`source_board_return.tar.gz`](../board/measurements/preproc_ab_20261028/source_board_return.tar.gz) SHA256 `0059dc49e95b127ff6d162c56b2e2f5a026909afcb98525550dbb85c9b05744a`。已保留现场团队的[原始说明](../board/measurements/preproc_ab_20261028/source_report_zh.md)、[CPU 日志](../board/measurements/preproc_ab_20261028/cpu.log)、[FPGA 日志](../board/measurements/preproc_ab_20261028/fpga.log)和板端条件、身份及 VDMA 状态文件。

根据交接说明，两组使用同一块板、同一 BIT/BOOT 与 `paddle_person_live`、`model.nb`，320×320 模型输入、2 线程、阈值 0.3、RGB 像素顺序、摄像头及既有 VDMA。板端身份文件记录的应用 SHA256 为 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`、模型 SHA256 为 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`；A 交接说明 BOOT 为 `816ebd03...`、BIT 为 `b286f480...`。模型先预热 1 次；每组记录 35 帧，统计时各丢弃前 5 帧，保留 30 帧。

**测试配置：CPU 采用双三次缩放与归一化，FPGA 采用双线性缩放与归一化。**两组使用相同帧尺寸、先后采集的实时画面，下表评价这两种实际配置的耗时；同算法、逐帧配对性能另需测试。两组 PicoDet 模型推理均由 ARM 执行。

## 独立重算的结果

| 指标 | CPU 双三次预处理 | FPGA 双线性预处理 |
|---|---:|---:|
| 统计帧数 | 30 | 30 |
| 捕获平均耗时 | 17.485 ms | 16.878 ms |
| 预处理平均耗时 | 416.960 ms | 17.537 ms |
| ARM Predictor 平均耗时 | 468.659 ms | 468.204 ms |
| 后处理平均耗时 | 0.029 ms | 0.029 ms |
| `total_ms` 平均 | 903.150 ms | 502.665 ms |
| `total_ms` 样本标准差 | 5.915 ms | 5.149 ms |
| `1000 / 平均 total_ms` | 1.107 FPS | 1.989 FPS |

上述两种配置的预处理均值比为 **23.776 倍**；`1000 / 平均 total_ms` 的比值为 **1.797 倍**，对应总耗时均值减少 400.485 ms（约 44.3%）。以上为本次两种配置的计时收益；同算法硬件收益及检测框精度一致性分别验证，20 FPS 仍为独立性能目标。

根据应用源码，`total_ms` 从采集开始到检测结果解析结束，包含该时间区间内的快照记录等开销；不包含之后的 ROI 控制和结果打印。以上 FPS 按日志计时区间换算；经帧新鲜度确认的摄像头至 ROI 更新率及 HDMI 刷新率分别测量。FPGA 预处理计时包含当前源码中的输入/输出复制、寄存器与 DMA 调用和写入模型输入；没有独立拆出 cache 维护或内核驱动时间。

## 正确性与计时适用范围

- 两组各 35 条 `PREPROC backend` 与 `TIME` 一一对应，无 `PREPROC_FALLBACK`。两份日志的 35 条 `SNAPSHOT` 均为 `content_changed=1`，但也均为 `freshness=unverified`，帧完成与新鲜度仍需硬件完成事件核验。
- 三份 VDMA 状态记录相同，均提示 `latched VDMA errors`；原记录保留未清除。现场功能确认与该状态一并归档；错误来源及帧新鲜度仍待专项核验。
- 这两组没有同一固定输入的板端张量与 Golden 对比，也没有配对检测框精度对照。后续[独立板端固定输入测试](preproc_board_golden_20261029_zh.md)逐位通过，其独立 BIT 哈希待核对，实时 A/B 记录的逐帧配对比较仍需单独开展。
- 如需量化这两套配置之间的精度差异，还需要相同输入的检测框比较；当前记录提供两套配置的**计时**依据，检测框精度按同输入配对流程评估。

## 复核方式

仓库根目录运行：

```sh
python -B tools/summarize_preproc_ab.py board/measurements/preproc_ab_20261028
python -B -m unittest discover -s tools/tests -v
```

重算输出已保存为 [`recomputed.json`](../board/measurements/preproc_ab_20261028/recomputed.json)。脚本检查后端标记、35 次记录与回退标记，计算丢弃前 5 次后的均值和样本标准差；其检查范围为日志字段及统计；BIT 身份、摄像头输入和硬件事务由对应测试核验。原始板端脚本作为 `source_board_script.txt` 保留供审查；执行该脚本会停止推理进程，应限定于独立实验环境。

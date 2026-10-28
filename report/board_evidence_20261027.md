# 本地实板证据补充（2026-10-27）

本页将两次不同的历史开发板实验单独归档；它们不证明当前 Eyes 完整视频程序的性能或本次上板 BIT 的身份。原始交接包保留在本地，仓库仅收录分析所需记录和输出，未上传启动镜像、设备序列号及现场个人信息。

## ARM98：纯 ARM 的完整 Predictor 与逐层计时

- 来源：本地 `ARM98_TEAMMATE_HANDOFF_20261027.zip`，SHA256 `3f0e47ce96b265a58c9885636a15f3352e12517ea3e1424d714adcfb2811dba5`；其中 `ARM98_BOARD_RESULT.tar.gz` 的 SHA256 为 `2605f6c0a16743dff59d0b987fa0f12f5431642b964e8390bff18753cfa5eb6b`，与交接记录及旁车一致。
- 实板记录使用固定 320×320 输入、单线程，预热 2 次，测量 20 帧；每帧有 98 个卷积记录，`hardware_submits=0`。模型 SHA256：`7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。
- 20 帧完整 Predictor 平均 **698.013 ms**；逐卷积均值之和为 **593.708 ms**。这是固定输入下的模型计时，不是摄像头到检测结果的 FPS，也不包含 HDMI 刷新性能。
- 同一输出张量在 20 次运行中逐字节一致；这证明本次重复运行的输出一致，不能代替与 Golden 的精度对照。
- 记录：[执行摘要](../board/measurements/arm98_20261027/arm98_records.txt)、[98 层汇总](../board/measurements/arm98_20261027/conv98.csv)、[逐帧算子记录](../board/measurements/arm98_20261027/layers.tsv)，以及同目录的 40 个输出张量文件。

## V2/R3：历史独立卷积 Golden 和固定输入推理

- 来源：本地 `EAGLEEYE_V2_STAGE_B_R3_RESULT_20261024.zip`，SHA256 `c1282e473c734f16d28d3f0cee8e64746a13d7a7b7e04c8f623047124e4cbf0f`；归档内 SHA256 清单 7 项核验通过。原始串口日志 SHA256 `dc1a05e0cd8277d7a04a38f3a46a458addf6a5c776d77f68e55ad7ac1195c80d`；仓库收录的是[测量相关日志摘录](../board/measurements/v2_r3_20261024/serial_measurement_excerpt.txt)，不是完整串口日志。
- 实验 BOOT 及板端读回 SHA256 均为 `f6bf4128938e67080444f300c5f9f80932f3101c3badc1bf561ce449bbf315b0`；实验 FIT SHA256 `ca6dd08854da465b0bd72dbdfd45c39982d019f3e326b0a1491bb8ca0d500e55`。PL 身份读回为 `bcb63d27093a3c122052896eb2e4982cc3ea3ce29d43ebfd1fbc6570ee36f8e5`。这些身份只属于该次实验，不能用于认定当前 Eyes BIT。
- 实板日志记录单次及随后 20 次 Golden 共 **21/21** 次完成，最终 int32 输出差异为 0，错误和锁定状态均为 0。该验证为当时 V2 `conv43/44` 独立实验，不代表 V3 或当前视频工程的 Golden 结果。
- 20 次固定输入全模型准确性记录 `pass=1`；其范围是本次输入及日志指定的检查，不能表述为 14 图全模型资格通过。实验日志同时标记 `diagnostic_only=1`、`formal_offload=0`。
- 同次 20 帧平均：ARM 完整 Predictor **815.568 ms，1.22614 FPS**；V2 **861.566 ms，1.16068 FPS**；报告加速比 **0.946611**，即该次实验 V2 较慢。此处 FPS 为固定输入完整推理 FPS，不是实时视频检测更新率。
- 20 次卸载明细的平均值：输入 pack **51.685 ms**、输出 decode **22.308 ms**、驱动计算监测区间 **16.209 ms**、完整卸载 **105.256 ms**。监测区间包含等待并已计入完整卸载时间，不能再次相加；这些数据不能代表纯 FPGA 计算时间。

## 可复核性与现有缺口

运行 `python tools/verify_field_evidence.py` 可从收录数据复核帧数、硬件提交数、逐层统计、重复输出一致性及 V2 Golden/耗时摘要。核验脚本检查原始记录内部一致性，不代替现场验证，也不认定两次实验的配置相同。

2026-10-28 后续补充已确认新复现 BIT 的哈希为 `b286f480...`，并保存 U-Boot 到 Linux 登录的串口原文；证据范围见 [复现 BIT 的上板记录](board_reproduced_20261028_zh.md)。后续已收到 Attention 应用日志、A 对六项功能正常的确认，以及不同缩放算法的 [CPU/FPGA A/B 计时](preproc_ab_20261028_zh.md)；同一固定输入的输出对照仍无结果。原有 CPU 预处理记录见 [已有板端数据](board_measurements.md)，必要的现场证据见 [待补清单](field_evidence_needed_zh.md)。

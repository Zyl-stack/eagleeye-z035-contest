# ARM98 与 V2/R3 独立实板实验

本页汇总两次历史独立开发板实验：ARM98 逐层计时与 V2/R3 卷积卸载。它们为后续架构选择提供实测依据，与当前 Eyes 视频程序的测试按独立版本归档。仓库收录分析记录和输出，启动镜像、设备序列号及现场个人信息保留在本地原始交接资料中。

## ARM98：纯 ARM 的完整 Predictor 与逐层计时

- 来源：本地 `ARM98_TEAMMATE_HANDOFF_20261027.zip`，SHA256 `3f0e47ce96b265a58c9885636a15f3352e12517ea3e1424d714adcfb2811dba5`；其中 `ARM98_BOARD_RESULT.tar.gz` 的 SHA256 为 `2605f6c0a16743dff59d0b987fa0f12f5431642b964e8390bff18753cfa5eb6b`，与交接记录及旁车一致。
- 实板记录使用固定 320×320 输入、单线程，预热 2 次，测量 20 帧；每帧有 98 个卷积记录，`hardware_submits=0`。模型 SHA256：`7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。
- 20 帧完整 Predictor 平均 **698.013 ms**；逐卷积均值之和为 **593.708 ms**。该计时覆盖固定输入的完整模型，摄像头到检测结果及 HDMI 刷新另设测量区间。
- 同一输出张量在 20 次运行中逐字节一致；该项验证重复运行一致性，Golden 精度按独立对照评估。
- 记录：[执行摘要](../board/measurements/arm98_20261027/arm98_records.txt)、[98 层汇总](../board/measurements/arm98_20261027/conv98.csv)、[逐帧算子记录](../board/measurements/arm98_20261027/layers.tsv)，以及同目录的 40 个输出张量文件。

## V2/R3：历史独立卷积 Golden 和固定输入推理

- 来源：本地 `EAGLEEYE_V2_STAGE_B_R3_RESULT_20261024.zip`，SHA256 `c1282e473c734f16d28d3f0cee8e64746a13d7a7b7e04c8f623047124e4cbf0f`；归档内 SHA256 清单 7 项核验通过。原始串口日志 SHA256 `dc1a05e0cd8277d7a04a38f3a46a458addf6a5c776d77f68e55ad7ac1195c80d`；仓库收录的是[测量相关日志摘录](../board/measurements/v2_r3_20261024/serial_measurement_excerpt.txt)，不是完整串口日志。
- 实验 BOOT 及板端读回 SHA256 均为 `f6bf4128938e67080444f300c5f9f80932f3101c3badc1bf561ce449bbf315b0`；实验 FIT SHA256 `ca6dd08854da465b0bd72dbdfd45c39982d019f3e326b0a1491bb8ca0d500e55`。PL 身份读回为 `bcb63d27093a3c122052896eb2e4982cc3ea3ce29d43ebfd1fbc6570ee36f8e5`。这些身份绑定本次独立实验，当前 Eyes BIT 按自己的版本表核验。
- 实板日志记录单次及随后 20 次 Golden 共 **21/21** 次完成，最终 int32 输出差异为 0，错误和锁定状态均为 0。该验证覆盖当时 V2 `conv43/44`，V3 与当前视频工程按各自 Golden 记录管理。
- 20 次固定输入全模型准确性记录 `pass=1`；其范围为本次固定输入和指定检查；14 图全模型资格属于另一测试集合。实验日志同时标记 `diagnostic_only=1`、`formal_offload=0`。
- 同次 20 帧平均：ARM 完整 Predictor **815.568 ms，1.22614 FPS**；V2 **861.566 ms，1.16068 FPS**；报告加速比 **0.946611**，即该次实验 V2 较慢。此处 FPS 为固定输入完整推理 FPS，不是实时视频检测更新率。
- 20 次卸载明细的平均值：输入 pack **51.685 ms**、输出 decode **22.308 ms**、驱动计算监测区间 **16.209 ms**、完整卸载 **105.256 ms**。驱动监测区间包含等待，作为完整卸载的子区间统计；纯 PL 计算需单独硬件计时。

## 复核方法与相关测试

运行 `python tools/verify_field_evidence.py` 可从收录数据复核帧数、硬件提交数、逐层统计、重复输出一致性及 V2 Golden/耗时摘要。核验脚本检查归档数据的内部一致性，两次实验的条件与版本分别列于上文。

当前 Eyes 视频系统的功能与性能见[上板记录](board_reproduced_20261028_zh.md)、[应用日志](board_app_evidence_20261028_zh.md)、[CPU/FPGA A/B](preproc_ab_20261028_zh.md)及[固定输入预处理 Golden](preproc_board_golden_20261029_zh.md)。历史 CPU 基线保留在[计时记录](board_measurements.md)，测试边界集中见[验证范围](field_evidence_needed_zh.md)。

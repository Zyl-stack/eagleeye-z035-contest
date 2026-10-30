# 仿真与验证说明

本说明索引已保存的结果，不把主机、RTL 或 QEMU 测试写成开发板实测。原工程自带的 [sim/readme.md](readme.md) 是原始快照，描述的是当时的材料状态；后续的验证证据列于本页。

## 测试输入和身份

| 文件 | SHA256 | 用途 |
|---|---|---|
| `data/captured.rgb` | `5560050490623f2292416364a2de30ea0182ead515f774c860c2ef5750a39070` | 预处理采集帧输入 |
| `data/captured.golden.f32` | `aece2b12ba154bf37dc209440bdc8fa0a0b1bc9a44936d12c4e327b03f7d447f` | 对应的 CPU 参考张量 |
| `data/model.nb` | `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4` | PicoDet 模型；哈希不证明它在现场被加载 |

## 预处理 RTL：Icarus 离线仿真

2026-10-27 的实际工具命令、退出码和逐项输出保存在 [原始仿真日志](preproc_iverilog_20261027.log)。输入包括零值、满值、通道区分、边界及采集帧；每帧 307,200 个 FP32 值与对应参考逐位一致，最大和平均绝对误差均为 0。采集帧经 RGB 字节适配器和核心联合仿真后也逐位一致。另有字节适配器与 AXI-Lite 独立地址/数据通道和 busy 状态测试通过。

现补充本地保留的[采集帧三路仿真输出](preproc_captured_dma_20261027/)，分别为 R/G/B 32 位十六进制字，共 307,200 个值。输入和 Golden 与上述仓库文件哈希一致。运行 `python -B tools/verify_preproc_sim_capture.py` 可将这三路记录逐位对照仓库 Golden；此数据出自 Icarus 仿真，**不是板端 DMA 输出**。

日志中核心 `673804 cycles` 和适配器加核心 `788884 cycles` 是测试平台的仿真周期；按 100 MHz 换算的时长不是板端 DMA、缓存和完整预处理延迟。

复现流程见 [复现指南](../report/reproduction_zh.md) 第 2、3 节：先恢复保留大小写的工程快照，再用 Python/NumPy 生成测试向量，使用 Icarus 运行 `tools/run_verification.py`。生成的 `vectors/` 和 `build/` 应位于独立目录。源文件与参考张量的哈希应先核验。

## 舵机与 ROI 控制

- [舵机 RTL 仿真日志](servo_iverilog_20261027.log)：Icarus Verilog 10.3，中心保持、20 ms 周期、10 µs 步进和 ROI 反向用例通过。该仿真不证明板端机械限位、供电、舵机方向或现场操作。
- [ROI 合同检查器测试](host_roi.log)：主机 1 项通过；检查器测试不证明鼠标、按键、暂停画面或舵机在板上运行。
- [预处理比较器测试](host_preproc.log)：主机 2 项通过，覆盖相等、差异与长度错误；不是板端张量对照。2026-10-28 复核见 [测试记录](host_skill_tests_20261028.txt)。
- 后续补充的两项 Skill 独立副本正反例、空配置和损坏归档测试见 [示例验证记录](skill_examples_20261028.txt)；XSA 文件检查也不能代表板端运行。

在各 Skill 目录可复现对应主机测试：

```sh
cd skill/preproc-fp32-golden && python -B -m unittest discover -s tests -v
cd ../roi-review-control && python -B -m unittest discover -s tests -v
```

## PicoDet 与注意力独立候选

[主机测试摘要](picodet_c_candidate/host_results.txt)记录临时 ID、无目标、错误输入以及模拟 Hold 行为。ARM 用户态 QEMU 使用离线输入和 CPU 预处理：[人体框与注意力请求摘要](picodet_c_candidate/qemu_predictor.txt)和[无目标摘要](picodet_c_candidate/qemu_empty.txt)均标明测试范围。这里的请求不是 Fovea 硬件完成信号；临时 ID 不保证遮挡后稳定。构建入口和边界见 [候选说明](../src/picodet_c_candidate/readme.md)。

## 旧 Lanczos 仿真及仍缺的验证

[旧 XSim 原始记录](lanczos_legacy_xsim.txt)报告 1536 个输出像素中 768 处错误；旧测试台与当前 RTL 的版本、参考算法对应关系尚未确认。既不能声称当前 RTL 逐像素通过，也不能单凭旧日志断言当前实板像素错误。工程源码和记录保留，独立 Lanczos Skill 不再展示；详见 [说明](lanczos_legacy_readme.md)。

现已收到 Attention 应用日志，现场已确认六项功能上板正常；后续[独立板端固定输入测试](../report/preproc_board_golden_20261029_zh.md)记录 307,200 个输出与 Golden 逐位一致，本次 BIT 哈希未核实。所需材料列于 [现场证据清单](../report/field_evidence_needed_zh.md)。[BIT 上板记录](../report/board_reproduced_20261028_zh.md)保存启动串口；[应用证据](../report/board_app_evidence_20261028_zh.md)记录了应用输出和 VDMA 警告，该次应用日志不包含实板张量比较。

## 2026-10-28 脱离工程复用补充

两项 Skill 分别复制到临时空目录，使用另一种张量形状及不同模块/地址的合成 HWH 运行正反例、长度错误和单元测试，命令及输出见 [复用测试记录](skill_portability_20261028.txt)。这些不是其他真实项目或 PYNQ 板端证据。

## PYNQ 示例的模拟验证

独立 PYNQ Skill 在空目录通过模拟对象测试；身份校验、下载许可、寄存器白名单、DMA 调用顺序、异常及长度检查的命令与输出见 [模拟记录](pynq_skill_mock_20261028.txt)。未加载 Overlay，未使用真实 DMA 或 PYNQ 板卡，不能视为板端结果。

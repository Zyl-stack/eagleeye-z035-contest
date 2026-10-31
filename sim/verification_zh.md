# 仿真与验证说明

本页按主机工具、RTL、QEMU 与板端四类整理验证结果，连接测试输入、原始输出和复现方法。原工程 [sim/readme.md](readme.md)保留历史快照状态，后续结果以本页索引查阅。

## 测试输入和身份

| 文件 | SHA256 | 用途 |
|---|---|---|
| `data/captured.rgb` | `5560050490623f2292416364a2de30ea0182ead515f774c860c2ef5750a39070` | 预处理采集帧输入 |
| `data/captured.golden.f32` | `aece2b12ba154bf37dc209440bdc8fa0a0b1bc9a44936d12c4e327b03f7d447f` | 对应的 CPU 参考张量 |
| `data/model.nb` | `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4` | PicoDet 模型文件身份；现场加载版本见应用测试记录 |

## 预处理 RTL：Icarus 离线仿真

2026-10-27 的实际工具命令、退出码和逐项输出保存在 [原始仿真日志](preproc_iverilog_20261027.log)。输入包括零值、满值、通道区分、边界及采集帧；每帧 307,200 个 FP32 值与对应参考逐位一致，最大和平均绝对误差均为 0。采集帧经 RGB 字节适配器和核心联合仿真后也逐位一致。另有字节适配器与 AXI-Lite 独立地址/数据通道和 busy 状态测试通过。

现补充本地保留的[采集帧三路仿真输出](preproc_captured_dma_20261027/)，分别为 R/G/B 32 位十六进制字，共 307,200 个值。输入和 Golden 与上述仓库文件哈希一致。运行 `python -B tools/verify_preproc_sim_capture.py` 可将这三路记录逐位对照仓库 Golden；该数据来源为 Icarus 适配器与核心仿真，板端张量输出见独立 Golden 记录。

日志中核心 `673804 cycles` 和适配器加核心 `788884 cycles` 按测试平台的仿真周期记录，100 MHz 为换算条件；板端 DMA/cache 及完整预处理延迟由实测计时。

复现流程见 [复现指南](../report/reproduction_zh.md) 第 2、3 节：先恢复保留大小写的工程快照，再用 Python/NumPy 生成测试向量，使用 Icarus 运行 `tools/run_verification.py`。生成的 `vectors/` 和 `build/` 应位于独立目录。源文件与参考张量的哈希应先核验。

## 舵机与 ROI 控制

- [舵机 RTL 仿真日志](servo_iverilog_20261027.log)：Icarus Verilog 10.3，中心保持、20 ms 周期、10 µs 步进和 ROI 反向用例通过。该项覆盖 RTL 行为，板端机械限位、供电和方向按现场条件核验。
- [ROI 合同检查器测试](host_roi.log)：主机 1 项通过；测试范围为接口合同检查器，板端功能见现场确认记录。
- [预处理比较器测试](host_preproc.log)：主机 2 项通过，覆盖相等、差异与长度错误；该项覆盖主机比较器。2026-10-28 复核见 [测试记录](host_skill_tests_20261028.txt)。
- 后续补充的两项 Skill 独立副本正反例、空配置和损坏归档测试见 [示例验证记录](skill_examples_20261028.txt)；XSA 检查覆盖文件元数据，运行行为由板端测试记录。

在各 Skill 目录可复现对应主机测试：

```sh
cd skill/preproc-fp32-golden && python -B -m unittest discover -s tests -v
cd ../roi-review-control && python -B -m unittest discover -s tests -v
```

## PicoDet 与注意力独立候选

[主机测试摘要](picodet_c_candidate/host_results.txt)记录临时 ID、无目标、错误输入以及模拟 Hold 行为。ARM 用户态 QEMU 使用离线输入和 CPU 预处理：[人体框与注意力请求摘要](picodet_c_candidate/qemu_predictor.txt)和[无目标摘要](picodet_c_candidate/qemu_empty.txt)均标明测试范围。这里记录软件请求；Fovea 完成状态由硬件事件核验，临时 ID 用于相邻结果关联。构建入口和边界见 [候选说明](../src/picodet_c_candidate/readme.md)。

## Lanczos 历史仿真与版本范围

[旧 XSim 原始记录](lanczos_legacy_xsim.txt)报告 1536 个输出像素中 768 处错误；旧测试台与当前 RTL 的版本、参考算法对应关系尚未确认。该日志按历史测试版本保存，当前 RTL 的逐像素状态需匹配版本后核验。工程源码和记录保留，独立 Lanczos Skill 不再展示；详见 [说明](lanczos_legacy_readme.md)。

另有历史独立测试工程的 [单通道](lanczos_single_xsim_pass_legacy.txt)和 [RGB](lanczos_rgb_xsim_pass_legacy.txt) XSim 日志，分别记录 768 和 1,536 个输出像素零错误。它们的测试工程源码未随本次材料提供，其与当前 Eyes RTL 的版本对应待核对，通过及失败日志分别保存。[测试范围与来源](lanczos_legacy_readme.md)

现已收到 Attention 应用日志，现场已确认六项功能上板正常；后续[独立板端固定输入测试](../report/preproc_board_golden_20261029_zh.md)记录 307,200 个输出与 Golden 逐位一致，本次 BIT 哈希未核实。所需材料列于 [现场证据清单](../report/field_evidence_needed_zh.md)。[BIT 上板记录](../report/board_reproduced_20261028_zh.md)保存启动串口；[应用证据](../report/board_app_evidence_20261028_zh.md)记录了应用输出和 VDMA 警告，该次应用日志不包含实板张量比较。

## 2026-10-28 脱离工程复用补充

两项 Skill 分别复制到临时空目录，使用另一种张量形状及不同模块/地址的合成 HWH 运行正反例、长度错误和单元测试，命令及输出见 [复用测试记录](skill_portability_20261028.txt)。该项验证独立目录及合成契约的可移植性，其他真实项目与 PYNQ 实板留待迁移测试。

## PYNQ 示例的模拟验证

独立 PYNQ Skill 在空目录通过模拟对象测试；身份校验、下载许可、寄存器白名单、DMA 调用顺序、异常及长度检查的命令与输出见 [模拟记录](pynq_skill_mock_20261028.txt)。该项以 PC 模拟对象覆盖控制流，Overlay 加载及真实 DMA/PYNQ 硬件行为留待板端验证。

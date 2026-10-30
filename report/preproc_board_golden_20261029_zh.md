# 固定输入的 FPGA 预处理板端 Golden 对比（2026-10-29）

## 结果与范围

A 回传的[原始归档](../board/measurements/preproc_golden_20261029/source_board_return.zip)记录了一次独立 Z035 板端 FPGA 预处理测试。输入为固定的 640×360 RGB888，输出为小端 FP32、NCHW、`1×3×320×320`：**307,200/307,200 个值与对应双线性缩放及归一化 Golden 的 32 位表示完全一致，差异 0 个，最大和平均绝对误差均为 0**。这是这一次固定输入、独立程序的实板结果，不是 307,200 次独立板测。

原始归档 SHA256 `b15fc2855426a4f0822f1c01ef5b21e46ff88051e529486869e30b366a572473`，ZIP CRC 和归档内 9 项 SHA256 核验均通过。随消息提供的独立结果说明与归档内 `RESULT_ZH.md` 逐字节一致；[原始说明副本](../board/measurements/preproc_golden_20261029/result_source_zh.md)、[串口/程序输出](../board/measurements/preproc_golden_20261029/preproc_golden_run.log)和[机器可读比较结果](../board/measurements/preproc_golden_20261029/compare_result.json)已保留。

| 文件 | SHA256 | 说明 |
|---|---|---|
| [固定输入](../data/captured.rgb) | `5560050490623f2292416364a2de30ea0182ead515f774c860c2ef5750a39070` | 与回传归档中的输入逐字节相同 |
| [参考张量](../data/captured.golden.f32) | `aece2b12ba154bf37dc209440bdc8fa0a0b1bc9a44936d12c4e327b03f7d447f` | 与回传归档中的 Golden 逐字节相同 |
| [板端输出](../board/measurements/preproc_golden_20261029/board_tensor.f32) | `aece2b12ba154bf37dc209440bdc8fa0a0b1bc9a44936d12c4e327b03f7d447f` | 1,228,800 字节，与 Golden 逐字节一致 |

## 身份、记录及限制

- A 记录板上 BOOT SHA256 为 `a13ae41c8a17f97a4cfdfab61cf462fa3e72eb976f786794755881fe51309e0b`；本次未提取和核实独立 BIT 哈希，不能将它与先前 `b286f480...` BIT 或 `816ebd03...` BOOT 等同。
- 内核版本记录为 `5.15.36-xilinx-v2022.2`；IP 版本寄存器为 `0x00010000`。测试使用独立 `preproc_board_golden_arm`，SHA256 `9043743a6ca54f8417af9e861b62eb29f776b93dbd652d96586336bfe182fa19`；本次未加载 `paddle_person_live` 和 `model.nb`，不证明其现场版本。
- 原始运行记录显示 IP 状态 `0x00000002`、周期 755818、输入计数 230400、每通道输出计数 102400，四路 DMA 状态均为 `0x00001002`。A 的说明记录运行退出码为 0；日志原文本身未单独打印 shell 退出码。
- 测试说明指出执行前自动推理进程已退出。独立测试程序及源码保留在原始归档；该源码直接使用 `/dev/mem` 和固定地址控制 DMA，不应在正在运行的稳定视频系统中直接复用。
- 本次对比不证明摄像头采集帧的新鲜度、完整视频共存、完整模型检测精度、性能加速比或不同版本 BIT 身份。既有应用日志的 VDMA 错误位、外部 I/O 和 CDC 告警继续保留，不能被本次对比替代。

## 离线复核

在仓库根目录运行：

```sh
python -B skill/preproc-fp32-golden/tools/verify_tensor.py --actual board/measurements/preproc_golden_20261029/board_tensor.f32 --golden data/captured.golden.f32 --shape 1x3x320x320 --atol 1e-6 --rtol 0 --require-exact
```

复核结果：`exact=307200`、`max_abs=0.0`、`mean_abs=0.0`、`over_tolerance=0`、`passed=true`，退出码 0。另以独立 Python 浮点读取和逐字节比较得到一致结果；这些复核读取回传文件，不代表再次上板。

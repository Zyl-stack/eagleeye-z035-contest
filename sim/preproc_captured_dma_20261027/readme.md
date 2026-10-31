# 采集帧预处理仿真输出

`captured_dma_r.hex`、`captured_dma_g.hex`、`captured_dma_b.hex` 是 2026-10-27 Icarus 适配器加预处理核心仿真的原始三路输出，每路 102,400 个 32 位字；来源和原始运行命令见 [仿真日志](../preproc_iverilog_20261027.log)。本地使用的 `captured.rgb` 和 `captured.golden.f32` 哈希与仓库 `data/` 下文件一致。该目录记录 Icarus 仿真输出，板端输出另见[固定输入 Golden](../../report/preproc_board_golden_20261029_zh.md)。

在仓库根目录执行：

```sh
python -B tools/verify_preproc_sim_capture.py
```

2026-10-28 实际结果：比较 307,200 值、0 差异、退出码 0。另在临时副本中修改输出的一个字后复核，报告 1 差异、退出码 1；未修改保留的原始输出。文件哈希由仓库 `sha256sums.txt` 记录。

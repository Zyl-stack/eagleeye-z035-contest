# 已有板端数据与口径

`board/attach_100.log` 中可解析 100 条 `TIME`，本次逐条计算：

复现命令：`python tools/summarize_board_log.py board/attach_100.log`。脚本只计算日志字段，不推断硬件身份或两次测试是否可比较。

| 历史 CPU 预处理运行指标 | 100 帧平均 |
|---|---:|
| capture | 13.607 ms |
| preprocess | 418.061 ms |
| inference | 481.955 ms |
| postprocess | 0.029 ms |
| total | 913.653 ms |
| FPS 字段算术均值 | 1.09475 FPS |
| 1000 / 平均 total | 1.09451 FPS |

该记录用于说明已有历史 CPU 基线；文件自身及原 Eyes README 说明它不是当前 Eyes FPGA 预处理版本的同条件对照。不能把这组数与不同版本或不同场景的 FPGA 结果相除宣称加速比。`board/attach_100_result.json` 仅对应第 100 帧，不能代表平均值。

当前 Eyes 的同条件 CPU/FPGA 预处理 A/B、完整视频检测更新率和准确率差异：**未在本目录中找到可配对的原始证据**。需要分别记录模型哈希、摄像头/显示、输入、线程、阈值、预热和样本数。视频画面刷新率与新检测结果更新率是不同指标。

另有历史 ARM98 单线程实板逐层计时和独立 V2/R3 实板 Golden 与固定输入推理记录，原始数据摘录、输出及其验证方法见 [本地实板证据补充](board_evidence_20261027.md)。这些实验不能替代当前 Eyes 的同条件测试。

## 新收到的 Attention 应用记录

2026-10-28 应用日志含 179 条 `TIME`，平均预处理 18.329 ms、平均 total 512.566 ms，`1000 / 平均 total` 为 1.951 FPS；详见 [日志证据及限制](board_app_evidence_20261028_zh.md)。日志含 VDMA 错误位警告且 `freshness=unverified`；该数据不能直接与上述历史 CPU 记录比较为加速比，也不能当成已验证的检测更新 FPS。

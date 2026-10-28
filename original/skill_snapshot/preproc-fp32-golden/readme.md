# PicoDet FP32 预处理 Golden 验证

## 适用范围

验证 RGB888 摄像头帧经 FPGA 预处理后生成的 FP32 NCHW 张量。默认示例契约为 640x360 RGB888 输入、`1x3x320x320` 输出；换模型或输入尺寸时，应先修改参考程序和 DMA 长度，再使用本 Skill。它是验证流程，不包含队友 IP 的所有权或许可证。

## 输入与结果

- 输入：同一帧原始 RGB、CPU 参考输出 `.f32`、从三个 S2MM DMA 缓冲区按 R/G/B 顺序拼成的 `.f32`。
- 输出：元素数、逐位相等数、最大/平均绝对误差、超出容差数；非有限值、长度不符或误差越界返回非零状态。
- 四路 DMA 示例：MM2S 输入 `691200` 字节；R/G/B 各 S2MM `409600` 字节。三个输出平面地址依次相隔 `409600` 字节；完成顺序无要求。

## 用法

```powershell
python tools/verify_tensor.py --actual board_tensor.f32 --golden captured.golden.f32 --shape 3x320x320 --require-exact
python -m unittest discover -s tests -v
```

先用零帧、全值帧、不同通道帧、边缘帧和真实采集帧运行完整 RTL 仿真，再在板上对**同一帧**跑 CPU 与 FPGA。只要 FPGA 输出与 CPU 双线性参考使用同一坐标、归一化和舍入规则，便可要求逐位相等；换工具链或算法契约时先定义容差，不能事后放宽阈值掩盖错误。

板端顺序：确认 HWH 地址/时钟与驱动一致；锁定输入帧槽；CPU 写过输入时先 clean cache；启动一条 MM2S 和三条 S2MM；等待三条输出、检查 DMA/IP 状态与超时；invalidate 输出 cache；导出张量比对。不得把摄像头仍在写入的槽当作固定输入。

测速要分别记录 IP `cycles`、DMA/DDR/cache 的端到端 `preprocess_ms`、纯 PS 参考 `preprocess_ms` 和检测结果。**IP 流接口仿真时延不是板端实测。**用户提供的本项目串口样例约为 FPGA 预处理 19 ms、此前纯 PS 约 420 ms；这些是特定板卡和帧的观测，不保证其他系统同速。完整 FP32 Golden 的独立交付记录为五类帧、每帧 307200 个值逐位一致。

## 失效条件

- 模型输入不再是 RGB888 到 `1x3x320x320` FP32，或 R/G/B 顺序改变。
- DMA 尚未全部完成、帧槽被覆盖、cache 未维护、出现 AXI 错误/超时。
- 仅拿 IP cycle 或一次 FPS 作为正确性证明；应停止并补完整张量比对。

参考来源：`D:\EagleEye_FPGA_Competition\Preproc_7035_Standalone_20261017\picodet_preproc_ip` 的 README、RTL、五类向量及 `tools/compare.py`；EagleEye Z035 板端日志由项目实测。使用者应替换这些示例路径为自己的工程路径。

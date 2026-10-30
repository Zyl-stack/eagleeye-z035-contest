# Z035 预处理 IP：板端输出与 Golden 对比

## 实测结论

**PASS。** 固定输入 `640×360 RGB888` 在开发板 FPGA 预处理 IP 上运行一次，保存完整输出张量。与相同双线性缩放和归一化算法的 Golden 比较：`1×3×320×320`、小端 FP32、NCHW，共 **307,200 个值**（1,228,800 字节）；**307,200/307,200 逐位一致，差异 0 个，最大绝对误差 0，平均绝对误差 0**。比较要求 32 位表示完全相同，同时检查 `atol=1e-6, rtol=0`，超容差数量为 0。本测试只证明固定输入的预处理数值结果，不证明摄像头新鲜帧或整网检测精度。

## 板端身份与运行记录

- 本次板上 `BOOT.BIN` SHA256：`a13ae41c8a17f97a4cfdfab61cf462fa3e72eb976f786794755881fe51309e0b`。
- 单独 BIT 哈希：未从本次 BOOT 中提取核实；不凭其他版本推定。
- 内核：`5.15.36-xilinx-v2022.2`。
- 预处理 IP 版本寄存器：`0x00010000`。
- 板端测试程序 `preproc_board_golden_arm` SHA256：`9043743a6ca54f8417af9e861b62eb29f776b93dbd652d96586336bfe182fa19`，ARMv7 Linux 静态链接。
- `paddle_person_live` 与 `model.nb`：本次独立测试没有加载或使用；当前板上文件位置与哈希未确认，不填写未经核实的版本。
- 运行退出码 0；IP 状态 `0x00000002`，周期 755818，输入像素计数 230400，输出每通道像素计数 102400；四路 DMA 状态均为 `0x00001002`。
- 运行前确认自动推理进程已退出；原视频通路未因本测试修改。

## 文件与方法

|文件|SHA256|
|---|---|
|`captured.rgb`|`5560050490623f2292416364a2de30ea0182ead515f774c860c2ef5750a39070`|
|`captured.golden.f32`|`aece2b12ba154bf37dc209440bdc8fa0a0b1bc9a44936d12c4e327b03f7d447f`|
|`board_tensor.f32`|`aece2b12ba154bf37dc209440bdc8fa0a0b1bc9a44936d12c4e327b03f7d447f`|

板端程序沿用已验证 PicoDet 应用的预处理四路 DMA 顺序、DDR 地址及 IP 寄存器地址，以固定 `captured.rgb` 运行一次并导出完整张量。随后在电脑上执行 `verify_tensor.py --actual board_tensor.f32 --golden captured.golden.f32 --shape 1x3x320x320 --require-exact`。原始板端输出见 `preproc_golden_run.log`，机器可读比较结果见 `compare_result.json`。

# EagleEye Z035 PicoDet 图像预处理 IP

独立源码交付目录，目标器件 `XC7Z035FFG900-2`。顶层 `picodet_preproc_ip` 把 DDR 中连续的 640×360 RGB888 帧转为三个 320×320 FP32 输出流。三个 S2MM DMA 分别写入 `tensor+0`、`tensor+409600`、`tensor+819200`，DDR 内最终布局为完整 R 平面、完整 G 平面、完整 B 平面，即 `1×3×320×320` NCHW，总计 1,228,800 bytes。三个平面的写入可并行进行，内存布局是 NCHW；不要求三次 DMA 按时间顺序完成。

输入 RGB 字节顺序为 R、G、B，每行 1920 bytes，无 padding；输入 AXI DMA MM2S 传输长度 691200 bytes。顶层 32 位 AXI4-Stream 输入从低字节开始重组 RGB 像素，输入 `TLAST` 只在整帧末尾。内部生成首像素 `TUSER` 和逐行 `TLAST`。插值坐标为 `(dx+0.5)*2-0.5`、`(dy+0.5)*360/320-0.5`，限制到图像范围。三个通道按请求中的 mean/std 归一化。核心使用两行 RGB RAM 和随源码提供的 FP32 ROM，不生成中间 DDR 图像。

## 文件

- `rtl/`：三个可综合 Verilog 文件和三个归一化 ROM `.mem`。
- `component.xml`、`xgui/`：Vivado IP 元数据；`scripts/build.tcl` 可从源码重新生成。
- `constraints/clock_100mhz.xdc`：100 MHz 独立 IP 时钟约束。
- `tb/`、`vectors/`、`tools/`：全尺寸 testbench、采集帧和四个角落用例、FP32 Golden、自动比较。
- `driver/preproc_dma_example.c`：Standalone BSP 四 DMA 调用示例。
- `build/vivado_impl/{utilization_impl,timing_impl}.rpt`：Vivado 2024.2 对上述器件的实际独立 IP 实现报告。`build/` 中工程、DCP 和仿真输出不是重新构建所需文件。

## Vivado 2022.2 重建

在本目录运行：

```text
vivado -mode batch -source scripts/build.tcl -tclargs package
vivado -mode batch -source scripts/build.tcl -tclargs impl
```

`package` 从三个 Verilog 源文件和 `.mem` 重建 `component.xml`。`impl` 独立综合、布局布线并写出 LUT/FF/BRAM/DSP 和时序报告。2024.2 已执行；本机未安装 2022.2，故 2022.2 的重新打包与结果需要验收机复跑。工程没有使用 2024 专有 IP、XCI、DCP 或 XSA 依赖。建议在 2022.2 对此目录的副本运行 `package`，以保留随包提供的 2024.2 元数据作为参照。

## 仿真验证

Python 需 `numpy`，仿真需 Icarus Verilog。`vectors/*.golden.f32` 是小端 IEEE-754 FP32，按 R/G/B 三平面保存；`.rgb` 是原始 RGB888，`.hex` 是 24 位像素，`.hex32` 是 DMA 的 32 位内存序字。`tools/generate.py` 可完全重生 ROM 和测试数据；采集帧来自附件的 `test_data/test.rgb`。

```text
python tools/generate.py --captured vectors/captured.rgb
python tools/run_verification.py
```

Windows 若工具未进 `PATH`，先设置 `IVERILOG_BIN=D:\iverilog\bin`。本机 Icarus 12.0 开发版在重复编译时偶发进程崩溃；可直接使用 Vivado 自带 XSim：

```text
vivado -mode batch -source scripts/sim_xsim.tcl -tclargs captured
```

把 `captured` 换成 `zero`、`full`、`distinct` 或 `edge` 可逐帧运行，脚本自动对比 Golden。五帧完整核心仿真各输出 307200 个 FP32 值，与双精度 CPU 双线性 Golden **307200/307200 逐位相同**。采集帧的 Vivado 2024.2 XSim 仿真也逐位相同。32 位 DMA 输入适配器单测通过，采集帧的适配器与核心联测也逐位相同；AXI-Lite 的分离 AW/W 写入和状态读取已测试。比较容差默认 `1e-6`，同时报告精确相等数量。

## 接口与寄存器

全部接口在同一 `aclk` 上升沿采样；`aresetn` 低有效、同步复位，运行中先停 DMA 再复位。时钟 100 MHz。AXI4-Lite 从口 32 位数据、6 位地址、寄存器 4 字节对齐。AXI4-Stream 输入 32 位、`TKEEP[3:0]=0xf`、整帧末 `TLAST=1`；输出 R/G/B 各 32 位，每通道 102400 个值，`TKEEP=0xf`，首值 `TUSER=1`，末值 `TLAST=1`。每条输出流独立握手，慢 DMA 会反压处理进度。

| 偏移 | 访问 | 含义 |
|---|---|---|
| `0x00` | W | bit0 写 1 启动一帧，自清；繁忙时再启动记错误 |
| `0x04` | R/W1C | bit0 busy，bit1 done/irq；bit2 输入像素 TKEEP 错，bit3 SOF 错，bit4 EOL 错，bit5 计数错，bit6 忙时启动，bit7 DMA 输入 TKEEP 错，bit8 DMA 帧 TLAST 错。写 bit1 清 done/irq |
| `0x08` | R | 启动至完成的时钟周期数，忙时继续计数 |
| `0x0c` | R | 已接受的 RGB 像素数，完成时 230400 |
| `0x10` | R | 每个输出平面的已完成像素数，完成时 102400 |
| `0x14` | R | 版本 `0x00010000` |

输出 `irq` 为 done 黏滞电平。输入不足或 DMA 中途停止会持续保持 busy；软件应设置超时、检查 DMA 错误并通过复位恢复。IP 不占用固定物理地址，集成时给 AXI-Lite 和新 DMA 分配空闲地址，不改变原摄像头、VDMA、HDMI、OSD、Fovea 模块及其既有地址。现有三帧相机槽以 `0x1e000000 + slot*0xb0000` 起始，实际送入的槽必须在本 IP 完成前保持不被摄像头覆盖。

DDR 最少读取原图 691200 bytes，写张量 1228800 bytes，总 1920000 bytes/帧；此外只有 DMA 描述和 AXI-Lite 控制流量。示例使用四个简单模式 AXI DMA（一 MM2S、三 S2MM），可以映射到可用 HP 端口和互连。帧与张量地址建议 64 字节对齐，三个平面偏移也是 64 字节整数倍。CPU 写过输入帧时，在启动 MM2S 前 clean/flush 输入 cache；直接由 VDMA 写 DDR 的帧不可用陈旧的 CPU cache 内容回写覆盖。DMA 写输出前处理可能脏的目标 cache 行，三个 S2MM 均完成后 invalidate 输出区域，再交给 PicoDet。驱动示例要求调用者确保相机槽所有权。

## 测量与边界

Icarus RTL 仿真在 100 MHz、输入连续有效及确定性输出背压下：核心 `673804` 周期，即 **6.73804 ms**；32 位 DMA 字节适配器加核心 `788884` 周期，即 **7.88884 ms**。这是预处理逻辑的流接口仿真延迟，输入数据由 testbench 立即提供，不代表 DDR 系统实测。包含真实 DDR、互连、四路 AXI DMA 配置和 cache 操作的端到端延迟目前**未实测**；驱动暴露周期寄存器，可在板上记录启动前/三个 DMA 完成后的计时器，报告同一帧的实测值。DDR 搬运量如上，不能仅以字节数准确推导时延。

Vivado 2024.2 对独立顶层的 XC7Z035 后布线报告：**709 LUT、333 FF、25.5 BRAM36 等效 tile、1 DSP**；100 MHz 约束下 WNS **+3.519 ns**、TNS 0、最差保持裕量 +0.115 ns。由单一 10 ns 约束的内部关键路径估算频率上限约 `1/(10-3.519 ns)=154.3 MHz`，不是板级频率扫描结果。报告仅约束 `aclk`，有 60 个无输入 delay、145 个无输出 delay 的端口；完整系统连接后必须重新跑时序以确认接口和 DDR 互连。

采集帧与附件 CPU `a=-0.75` bicubic 核的向量化数学对照：双线性/双三次张量最大绝对差 `0.842956781`、平均绝对差 `0.0621779355`、p99 `0.332894268`、RMSE `0.0940398358`。运行 `python tools/compare_bicubic.py captured` 可重算并生成 `captured.bicubic.f32`。附件提供的 `model.nb` 需目标 PicoDet/Paddle Lite 运行环境；本机未执行两次模型推理，因此**检测框变化尚未量化**。`tools/compare_detections.py` 接受两次推理导出的 JSON（`category_id`、`score`、`bbox:[x,y,w,h]`），报告框增减、IoU≥0.5 匹配数和分数差；完成板上验收时应补上实际结果。

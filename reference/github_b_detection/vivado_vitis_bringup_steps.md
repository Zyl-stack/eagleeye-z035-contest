# Vivado / Vitis Bring-up Steps

本文档用于 EagleEye C 侧上板前的最小 AXI-Lite bring-up。目标只是验证 Zynq PS 能否通过 AXI-Lite 访问 Fovea Control IP，不包含 DMA、VDMA、摄像头、HDMI、Lanczos RTL 或真实图像处理。

## 1. Vivado 里做什么

1. 打开真实 Vivado 工程。
2. 确认当前工程已经包含 Zynq Processing System。
3. 在 Block Design 中加入 Fovea AXI-Lite Control IP。
4. 建议 instance name 可以先用 `fovea_axil_ctrl_0`，但最终以 Vivado 实际生成结果为准。
5. 不要手动把 C 代码里的 base address 写成建议地址 `0x43C00000`。

这些步骤必须你在 Vivado 中手动完成，Codex 不能替代真实 Vivado 工程配置。

## 2. Block Design 里需要什么

Block Design 至少需要：

1. Zynq Processing System。
2. Fovea AXI-Lite Control IP。
3. AXI interconnect 或 SmartConnect。
4. Fovea IP 的 AXI-Lite slave interface。
5. clock 连接。
6. reset 连接。

如果 Fovea IP 暂时只是空壳，也要保证 AXI-Lite slave 能响应读写，否则 smoke test 无法读到有效 STATUS。

## 3. Zynq PS 怎么连接 AXI

1. 在 Zynq PS 配置中启用一个 General Purpose AXI Master，例如 `M_AXI_GP0`。
2. 将 PS 的 AXI Master 接到 AXI interconnect / SmartConnect。
3. 将 interconnect 的 master 侧接到 Fovea AXI-Lite Control IP 的 slave 口。
4. 确认 AXI clock 域一致，或者已经正确跨时钟。
5. 确认 reset 极性和 IP 要求一致。

如果 AXI 没连好，Vitis 代码可能卡住、读回全 0，或者触发 bus error。

## 4. Address Editor 怎么看地址

1. 打开 Vivado 的 Address Editor。
2. 找到 Fovea AXI-Lite Control IP 对应的 slave segment。
3. 点击 Assign Address，或者手动分配地址。
4. 记录 Vivado 给出的真实 base address。
5. 检查寄存器空间至少覆盖 `0x00` 到 `0x2C`。
6. 确认 `OUTPUT_BUFFER` 是 `0x20`，`QUALITY_FLAGS` 是 `0x2C`。

建议地址 `0x43C00000` 只是建议，不是最终定义。最终以 Address Editor 和导出的 `xparameters.h` 为准。

## 5. 如何生成 bitstream

1. 保存 Block Design。
2. Validate Design，确认没有连接错误。
3. Generate Output Products。
4. Create HDL Wrapper。
5. Run Synthesis。
6. Run Implementation。
7. Generate Bitstream。

如果 bitstream 没生成，Vitis 即使能编译，也不能说明硬件已经可访问。

## 6. 如何导出 XSA

1. 在 Vivado 中选择 `File -> Export -> Export Hardware`。
2. 勾选 include bitstream。
3. 导出 `.xsa` 文件。
4. 记住导出的路径，后面 Vitis 创建 Platform 要用。

如果重新分配地址或修改 IP，必须重新导出 XSA。

## 7. Vitis 怎么创建 Platform / Application

1. 打开 Vitis。
2. 创建新的 Platform Project。
3. 选择 Vivado 导出的 `.xsa` 文件。
4. 生成 platform。
5. 基于该 platform 创建 Application Project。
6. 选择 bare-metal / standalone domain。
7. 将本仓库的 Vitis-only 文件加入 application：
   - `include/eagleeye_types.h`
   - `include/register_map.h`
   - `include/platform_io_vitis.h`
   - `src/platform_io_vitis.c`
   - `vitis_axi_smoke_test.c`

PC 模拟文件 `src/platform_io_sim.c` 不要加入 Vitis smoke test。

## 8. 如何查看 xparameters.h

1. 在 Vitis 工程中展开 platform 或 BSP。
2. 找到 generated BSP include 目录。
3. 打开 `xparameters.h`。
4. 搜索 Fovea IP instance name，例如 `FOVEA`、`AXIL`、`CTRL`。
5. 找到类似下面的宏：

```c
#define XPAR_<REAL_INSTANCE_NAME>_BASEADDR ...
```

真实宏名必须来自这个文件，不能凭建议名字猜。

## 9. 如何把真实 BASEADDR 宏填入当前工程

在 `include/platform_io_vitis.h` 中找到：

```c
#define EAGLEEYE_FOVEA_BASEADDR ((uintptr_t)0u)
```

真实 IP 出现后，不要写死数字地址。应显式定义为 `xparameters.h` 里的真实宏，例如：

```c
#define EAGLEEYE_FOVEA_BASEADDR XPAR_<REAL_INSTANCE_NAME>_BASEADDR
```

如果你不想直接改公共头文件，也可以在 Vitis Application 的编译配置中定义 `EAGLEEYE_FOVEA_BASEADDR`，但它仍然必须指向 `xparameters.h` 的真实宏或真实 BSP 导出的符号。

没有真实宏时，程序会打印：

```text
FOVEA AXI-LITE IP NOT AVAILABLE
```

这表示软件安全跳过硬件访问，不代表硬件测试通过。

## 10. 如何运行 vitis_axi_smoke_test

1. 确认 application 的 `main` 使用 `vitis_axi_smoke_test.c`。
2. 编译 application。
3. 连接开发板和 JTAG。
4. 打开串口终端，波特率按 BSP/板卡配置设置。
5. 下载 bitstream。
6. Run application。
7. 观察串口输出。

smoke test 会：

1. 打印启动信息。
2. 打印 base address。
3. 构造 fake `EE_FoveaRequest`。
4. 调用 `fovea_submit_request()`。
5. 写 `CONTROL.ENABLE`。
6. 写所有 request shadow registers。
7. 最后写 `CONTROL.ENABLE | CONTROL.COMMIT`。
8. 轮询 `STATUS.busy / done / error`。
9. 软件 timeout 后退出，避免死循环。

## 11. 串口正常情况下应该看到什么

正常至少应看到：

```text
EagleEye AXI-Lite Smoke Test

base = 0xXXXXXXXX
request_id = 1
target_id = 202
frame_id = 100
ROI = (320,120,128,128)
request submitted
STATUS = 0xXXXXXXXX
active_mode=...
busy=...
done=...
error=...
CYCLE_COUNT = 0xXXXXXXXX
QUALITY_FLAGS = 0xXXXXXXXX
```

如果硬件还没有实现 done，可能会看到：

```text
software timeout while polling STATUS
```

这不一定是 C 代码错误，可能只是 RTL 还没有产生 done/error。

## 12. 如果 STATUS 一直为 0 该检查什么

1. Fovea AXI-Lite IP 是否真的加入 Block Design。
2. PS AXI Master 是否连接到 Fovea IP。
3. Address Editor 是否分配了地址。
4. `xparameters.h` 中 BASEADDR 宏是否和代码使用的一致。
5. bitstream 是否是最新生成并下载到板子上的。
6. Fovea IP 是否有 clock。
7. Fovea IP reset 是否已释放。
8. RTL 是否实现了 STATUS 寄存器读逻辑。
9. `CONTROL.ENABLE` 是否被 RTL 接收。
10. `CONTROL.COMMIT` write-1 pulse 是否被 RTL 锁存。
11. STATUS 的 `done/error/busy` bit 是否确实按 `register_map.h` 定义。
12. AXI-Lite 读写是否被地址 decode 到正确 IP。

STATUS 一直为 0 不能直接说明 C 代码错，也不能说明硬件通过；它表示需要继续查 Vivado 连接、地址、reset、clock 和 RTL 寄存器实现。

## Final Boundary

Codex 可以帮助检查代码、文档、寄存器定义和 smoke test 逻辑，但不能替代：

- 真实 Vivado block design 配置。
- 真实 bitstream 生成。
- 真实 XSA 导出。
- 真实 Vitis BSP 生成。
- 真实板卡串口输出。
- 真实 AXI-Lite 读写验证。

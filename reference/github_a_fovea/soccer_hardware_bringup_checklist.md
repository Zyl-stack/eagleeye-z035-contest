# Soccer Hardware Bring-up Checklist

这份清单用于足球赛事主动视觉版本第一次真实上板。当前软件仍是 PC 模拟和 Vitis 接口准备，不能把编译成功当作 FPGA 硬件验证成功。

## 一、A 侧必须提供

- [ ] 建立 fovea_axil_ctrl AXI4-Lite Slave IP。
- [ ] 实现并冻结本工程的寄存器偏移：CONTROL 0x00、STATUS 0x04、REQUEST_ID 0x08、SOURCE_FRAME_ID 0x0C、TARGET_ID 0x10、ROI_XY 0x14、ROI_WH 0x18、ENHANCE_MODE 0x1C、OUTPUT_BUFFER 0x20、DEADLINE 0x24、CYCLE_COUNT 0x28、QUALITY_FLAGS 0x2C。
- [ ] 确认 CONTROL 的 ENABLE、COMMIT、CANCEL、CLEAR_ERROR 行为，COMMIT/CANCEL/CLEAR_ERROR 必须是 write-1 pulse。
- [ ] 确认 STATUS 的 busy、done、error、active_mode bit 行为。
- [ ] 确认 QUALITY_FLAGS 的六个 bit 行为。
- [ ] 确认 done/error 的清除方式，以及 STATUS 是否可以被 PS 正常读回。

## 二、Vivado Block Design

- [ ] 添加 Zynq Processing System。
- [ ] 运行 PS 配置向导，启用 PS M_AXI_GP0。
- [ ] 添加 AXI Interconnect 或 SmartConnect。
- [ ] 将 PS M_AXI_GP0 连接到 Interconnect/SmartConnect，再连接到 fovea_axil_ctrl 的 AXI4-Lite Slave。
- [ ] 连接 AXI clock：PS 提供的 FCLK_CLK0 接 AXI 互连和 IP 时钟。
- [ ] 连接 AXI reset：使用与该时钟域匹配的 Processor System Reset，连接到互连和 IP 复位。
- [ ] 检查 AXI-Lite 数据宽度、时钟域和复位极性。

## 三、地址和生成

- [ ] 在 Address Editor 中为 fovea_axil_ctrl 分配地址。
- [ ] 记录 Vivado 最终分配的基地址，不使用文档中的建议地址作为最终地址。
- [ ] Validate Design，解决所有 AXI、clock、reset 和 address 警告。
- [ ] Generate Output Products。
- [ ] Create HDL Wrapper。
- [ ] Generate Bitstream。
- [ ] Export Hardware，生成包含 bitstream 的 XSA。

## 四、Vitis

- [ ] 用 XSA 创建 Vitis Platform。
- [ ] 创建 standalone bare-metal Application。
- [ ] 将 platform_io_vitis.c、platform_io_vitis.h、register_map.h 和 vitis_axi_smoke_test.c 加入应用工程。
- [ ] 在生成的 xparameters.h 中搜索 FOVEA、FOVEA_AXIL_CTRL 或实际 IP instance 名称。
- [ ] 由真实宏配置 EAGLEEYE_FOVEA_BASEADDR，例如 XPAR_<REAL_INSTANCE>_S00_AXI_BASEADDR；不要填写猜测地址。
- [ ] 确认 Vitis 工程能找到 xil_io.h、xparameters.h 和 xil_printf.h。
- [ ] 编译 smoke test。缺少真实宏时，程序必须报告 FOVEA AXI-LITE IP NOT AVAILABLE，不能访问地址 0，也不能伪造成功。

## 五、第一次硬件 smoke test

- [ ] 下载 bitstream 到真实 Zynq 板卡。
- [ ] 连接 UART，确认波特率使用板卡工程配置。
- [ ] 下载并运行 Vitis Application。
- [ ] 观察第一次 Xil_Out32 写入 request 寄存器，再观察 Xil_In32 读取 STATUS、CYCLE_COUNT、QUALITY_FLAGS。
- [ ] 确认 shadow request 的所有字段先写入，最后才写 CONTROL.COMMIT。
- [ ] 确认软件轮询有 timeout，不会无限循环。

正常输出应类似：

    EagleEye Soccer FOVEA REQUEST Smoke Test
    base = 0xXXXXXXXX
    FOCUS target = BALL
    target_id = 1 (BALL)
    source_frame_id = 100
    FOVEA REQUEST ROI = (320,120,128,128)
    FOVEA REQUEST submitted for BALL focus
    STATUS = 0xXXXXXXXX
    active_mode=1 (BILINEAR)
    busy=0 done=1 error=0
    CYCLE_COUNT = 0xXXXXXXXX
    QUALITY_FLAGS = 0xXXXXXXXX

这只能说明 PS 能够执行软件读写并观察寄存器。是否真的完成 ROI 处理，还要由 A 侧结合硬件波形、output buffer 和质量标志确认。

## 六、STATUS 一直为 0 的排查顺序

1. 确认 bitstream 是最新生成并已下载。
2. 确认 Vitis 使用的 XSA 与当前 bitstream 对应。
3. 确认 EAGLEEYE_FOVEA_BASEADDR 来自当前 xparameters.h。
4. 确认 PS M_AXI_GP0、AXI Interconnect/SmartConnect 和 IP AXI 时钟已连接。
5. 确认复位已经释放，AXI reset 极性正确。
6. 用 ILA 或仿真确认 AXI AWVALID/WVALID/ARVALID 是否到达 IP。
7. 确认 IP 地址范围没有重叠，Address Editor 没有未解决项。
8. 确认 IP 的 STATUS 寄存器确实实现为可读寄存器。
9. 确认 ENABLE/COMMIT 写入顺序和 write-1 pulse 定义与 A 侧 RTL 一致。
10. 检查串口打印的返回码；出现 FOVEA AXI-LITE IP NOT AVAILABLE 时先修复 BSP 宏配置，不要继续判断硬件状态。

## 七、必须等待真实硬件的事项

- A 侧 AXI4-Lite Slave RTL 和寄存器实际行为。
- Vivado instance name、Address Editor 基地址和 xparameters.h 宏。
- STATUS/QUALITY_FLAGS 的真实读回值。
- COMMIT 的帧边界生效行为。
- done/error 清除时序。
- 真实 ROI 输出和 FPGA 处理延迟。

以上内容不能由 PC 模拟、静态代码审查或 Codex 替代，必须由队员在 Vivado、Vitis 和真实 Zynq 板卡上完成。

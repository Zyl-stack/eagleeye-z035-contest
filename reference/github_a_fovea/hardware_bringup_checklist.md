# EagleEye Hardware Bring-up Checklist

当前仓库只提供 C 侧软件准备和 Vitis smoke test 代码。下面步骤需要在真实 Vivado/Vitis/板卡环境中手动完成，Codex 不能替代真实硬件验证。

更详细的新手操作步骤见 `VIVADO_VITIS_BRINGUP_STEPS.md`。

## Manual Vivado Steps

- [ ] 在 Vivado 中加入 Fovea AXI-Lite Control IP。
- [ ] 确认或创建建议 instance name：`fovea_axil_ctrl_0`。
- [ ] 连接 Zynq PS AXI Master 到 Fovea AXI-Lite slave。
- [ ] 连接 clock。
- [ ] 连接 reset。
- [ ] 在 Address Editor 中分配地址。
- [ ] 记录 Address Editor 给出的真实 base address。
- [ ] 确认 `QUALITY_FLAGS` 地址是 `0x2C`，不要放到 `0x20`。
- [ ] Generate Bitstream。
- [ ] Export XSA，包含 bitstream。

## Manual Vitis Steps

- [ ] 在 Vitis 中创建 Platform。
- [ ] 基于该 Platform 创建 bare-metal Application。
- [ ] 将以下文件加入 Vitis Application：
  - `include/eagleeye_types.h`
  - `include/register_map.h`
  - `include/platform_io_vitis.h`
  - `src/platform_io_vitis.c`
  - `vitis_axi_smoke_test.c`
- [ ] 查看 BSP 生成的 `xparameters.h`。
- [ ] 确认真实 BASEADDR 宏。
- [ ] 如果宏名不是代码默认识别的名字，在 Vitis 编译选项或公共头文件中定义 `EAGLEEYE_FOVEA_BASEADDR`。
- [ ] 编译 smoke test。
- [ ] 下载 bitstream。
- [ ] 运行程序。
- [ ] 通过串口查看启动信息、base address、request 信息、STATUS、CYCLE_COUNT、QUALITY_FLAGS。

## What Codex Can Check

- [x] PC 纯 C 模拟能否继续编译运行。
- [x] AXI-Lite register offset 和 bit mask 是否和文档一致。
- [x] Vitis 代码是否避免写死建议地址 `0x43C00000`。
- [x] 没有真实 base address 宏时是否明确打印 `FOVEA AXI-LITE IP NOT AVAILABLE`。
- [x] smoke test 是否有软件 timeout，避免死循环。

## What Requires Real Hardware

- [ ] PS 是否真的能访问 Fovea AXI-Lite IP。
- [ ] `CONTROL.ENABLE` 是否被 RTL 正确识别。
- [ ] `CONTROL.COMMIT` write-1 pulse 是否被 RTL 正确锁存。
- [ ] `STATUS.busy/done/error/active_mode` 是否按 A 侧定义变化。
- [ ] `CYCLE_COUNT` 是否递增或反映硬件处理周期。
- [ ] `QUALITY_FLAGS` 每个 bit 是否由 RTL 正确产生。
- [ ] `OUTPUT_BUFFER` 是否被 A 侧锁存并回报。

## Expected Serial Output Shape

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

如果真实 IP 还不存在，程序必须明确输出：

```text
FOVEA AXI-LITE IP NOT AVAILABLE
```

这表示软件已安全跳过硬件访问，不能解释为硬件测试通过。

# EagleEye Vitis Porting Notes

当前阶段：Zynq PS / Vitis / AXI-Lite 最小通信准备。本文档不包含真实图像处理、Lanczos、摄像头、HDMI、DMA 或 VDMA。

## Platform-Independent Core Logic

这些文件不依赖 Windows API，也不依赖 Xilinx BSP API，后续应尽量保持平台无关：

- `include/eagleeye_types.h`
- `include/attention_config.h`
- `include/attention.h`
- `include/attention_scheduler.h`
- `include/risk.h`
- `include/state_machine.h`
- `include/transaction.h`
- `src/attention.c`
- `src/attention_scheduler.c`
- `src/risk.c`
- `src/state_machine.c`
- `src/transaction.c`

### V2 portability audit

| Module | Standalone assessment | Note |
|---|---|---|
| `c_config.*` | A: can compile | Contains the existing scheduler `float switch_margin` configuration, but no Windows or filesystem dependency. |
| `attention.*` | A: can compile | Fixed-width types and deterministic integer Attention calculation. |
| `attention_scheduler.*` | A: can compile, with cost note | Existing `float` comparison implements Switch Margin. It is portable C, but the standalone build should measure software cost before optimizing it. |
| `roi_smoother.*` | A: can compile | Static state, fixed-width integer arithmetic, and `string.h` initialization only. |
| `enhance_policy.*` | A: can compile | Deterministic integer policy; no ML, dynamic memory, or platform API. |
| `director_state.*` | A: can compile | Deterministic state/event logic with a static context and `string.h` initialization only. |
| `decision_snapshot.*` | B: split use | The snapshot structure/build function is portable; `ee_decision_snapshot_print()` uses `stdio` and is for PC text explanation, not the minimal Vitis path. |
| `fovea_stats.*` | B: PC display/statistics | Uses `float` for ratios and averages. The fixed pixel counters are portable; omit this module from the minimal hardware smoke ELF unless a measured standalone cost is acceptable. |

No audited V2 module uses Windows API, C++ syntax, dynamic memory, or filesystem access. Tkinter/Python is confined to `tools/soccer_visualizer.py` and is PC-only. `c_runtime_log.c` and the `printf` helper in `decision_snapshot.c` are also PC logging paths. The minimal Vitis application should link the core decision files, `a_fovea_adapter.c`, `platform_io_vitis.c`, and one smoke-test `main`, while leaving the visualizer, PC logs, and statistics display out.

核心结构：

```text
Core Logic
|
+-- platform_io_sim.c
|
+-- platform_io_vitis.c
```

## PC Simulation Only

这些文件用于 Visual Studio / PC 模拟：

- `src/main.c`
- `src/fake_data.c`
- `include/fake_data.h`
- `include/platform_io.h`
- `src/platform_io_sim.c`
- `tests/test_scenarios.c`
- `tests/test_attention_score.c`
- `run_tests_msvc.bat`
- `EagleEye_C_Attention_Sim.sln`
- `EagleEye_C_Attention_Sim.vcxproj`

PC 模拟仍然使用 Fake Target / Fake Refined Result，不访问真实硬件。

## Vitis Only

这些文件只在 Vitis 裸机工程中使用：

- `include/register_map.h`
- `include/platform_io_vitis.h`
- `src/platform_io_vitis.c`
- `vitis_axi_smoke_test.c`

`src/platform_io_vitis.c` 使用标准 Xilinx BSP 头文件：

```c
#include "xil_io.h"
#include "xparameters.h"
#include "xil_printf.h"
```

Visual Studio 工程中不会编译 `platform_io_vitis.c`，因为普通 PC 环境没有 Xilinx BSP 头文件。

真实上板步骤见 `HARDWARE_BRINGUP_CHECKLIST.md`。

## AXI-Lite Register Map

寄存器偏移定义在 `include/register_map.h`。

| Offset | Register | Purpose |
|---:|---|---|
| `0x00` | `CONTROL` | enable and write-1 pulse command bits |
| `0x04` | `STATUS` | busy/done/error and active mode |
| `0x08` | `REQUEST_ID` | 当前 Fovea Request 事务 ID |
| `0x0C` | `SOURCE_FRAME_ID` | 源帧号 |
| `0x10` | `TARGET_ID` | 选中目标 ID |
| `0x14` | `ROI_XY` | 低 16 位 `roi_x`，高 16 位 `roi_y` |
| `0x18` | `ROI_WH` | 低 16 位 `roi_w`，高 16 位 `roi_h` |
| `0x1C` | `ENHANCE_MODE` | 增强模式 |
| `0x20` | `OUTPUT_BUFFER` | 输出 buffer 选择，保持 0x20 |
| `0x24` | `DEADLINE` | deadline frames |
| `0x28` | `CYCLE_COUNT` | 硬件周期计数 |
| `0x2C` | `QUALITY_FLAGS` | 质量标志，保持 0x2C，不能与 OUTPUT_BUFFER 冲突 |

注意：A 侧曾建议把 `QUALITY_FLAGS` 写成 `0x20`，但这会和 `OUTPUT_BUFFER` 冲突。本工程保持原映射：`OUTPUT_BUFFER = 0x20`，`QUALITY_FLAGS = 0x2C`。

## AXI-Lite Bit Masks

`CONTROL @ 0x00`：

| Bit | Mask | Meaning |
|---:|---|---|
| bit0 | `EAGLEEYE_CTRL_ENABLE_MASK` | ENABLE |
| bit1 | `EAGLEEYE_CTRL_COMMIT_MASK` | COMMIT, write-1 pulse |
| bit2 | `EAGLEEYE_CTRL_CANCEL_MASK` | CANCEL, write-1 pulse |
| bit3 | `EAGLEEYE_CTRL_CLEAR_ERROR_MASK` | CLEAR_ERROR, write-1 pulse |

`STATUS @ 0x04`：

| Bit | Mask | Meaning |
|---:|---|---|
| bit0 | `EAGLEEYE_STATUS_BUSY_MASK` | busy |
| bit1 | `EAGLEEYE_STATUS_DONE_MASK` | done |
| bit2 | `EAGLEEYE_STATUS_ERROR_MASK` | error |
| bit[5:4] | `EAGLEEYE_STATUS_ACTIVE_MODE_MASK` | active_mode |

`active_mode` encoding:

| Value | Meaning |
|---:|---|
| `00` | BYPASS |
| `01` | BILINEAR |
| `10` | LANCZOS |
| `11` | RESERVED |

`QUALITY_FLAGS @ 0x2C`：

| Bit | Mask | Meaning |
|---:|---|---|
| bit0 | `EAGLEEYE_QUALITY_ROI_CLIPPED_MASK` | ROI_CLIPPED |
| bit1 | `EAGLEEYE_QUALITY_MODE_FALLBACK_MASK` | MODE_FALLBACK |
| bit2 | `EAGLEEYE_QUALITY_TARGET_SWITCH_MASK` | TARGET_SWITCH |
| bit3 | `EAGLEEYE_QUALITY_REQUEST_CANCELLED_MASK` | REQUEST_CANCELLED |
| bit4 | `EAGLEEYE_QUALITY_FRAME_SAFE_LATCHED_MASK` | FRAME_SAFE_LATCHED |
| bit5 | `EAGLEEYE_QUALITY_OUTPUT_BUFFER_VALID_MASK` | OUTPUT_BUFFER_VALID |

写请求顺序：

```text
REQUEST_ID
SOURCE_FRAME_ID
TARGET_ID
ROI_XY
ROI_WH
ENHANCE_MODE
OUTPUT_BUFFER
DEADLINE
CONTROL.ENABLE | CONTROL.COMMIT
```

这体现：

```text
shadow register -> commit -> frame boundary effective
```

## Base Address From xparameters.h

不要在 C 代码里写死 FPGA IP 绝对地址。

A commit `729a8d0` 的 Block Design 和 XSA 已确认 instance 为 `fovea_axil_ctrl_0`，地址段为 `0x40000000` 到 `0x40000FFF`。由该 XSA 生成的 `eagleeye_platform` standalone BSP 已确认下列 `xparameters.h` 宏；代码不硬编码该数值地址。

当前 `include/platform_io_vitis.h` 会优先使用：

```c
XPAR_FOVEA_AXIL_CTRL_0_BASEADDR
```

如果 Vivado 里最终 IP 名称不同，需要在 Vitis 工程的 BSP 或编译选项中定义：

```c
#define EAGLEEYE_FOVEA_BASEADDR XPAR_<FINAL_IP_NAME>_BASEADDR
```

如果没有找到真实 base address，代码使用 `0` 作为占位并跳过硬件寄存器访问，只用于编译检查，不会声称硬件测试通过。

## Hardware TODOs To Freeze With A

仍需 A 同学或硬件接口负责人确认：

- `CONTROL.COMMIT` 已按 A 侧第一版定义为 bit1；仍需硬件实现最终确认。
- `STATUS` busy/done/error/active_mode 已按 A 侧第一版定义；仍需硬件实现最终确认。
- `QUALITY_FLAGS` bit0..bit5 已按 A 侧第一版定义；仍需硬件实现最终确认。
- `OUTPUT_BUFFER` 的来源：是否加入 `EE_FoveaRequest`，还是作为固定平台配置。
- Vitis BSP 已确认 `XPAR_FOVEA_AXIL_CTRL_0_BASEADDR` 和 `XPAR_FOVEA_AXIL_CTRL_0_HIGHADDR`；真实板卡运行前仍要确认 Vitis application 选用该最新 Platform。
- `deadline_frames` 是由 C 写给 A，还是 A 内部只作为调试/保护参考。

## Smoke Test

Vitis 最小 bring-up 文件：

- `vitis_axi_smoke_test.c`

目标只验证 PS 能否访问 Fovea IP AXI-Lite 寄存器：

1. 打印启动信息。
2. 打印 Fovea IP Base Address。
3. 构造一个 Fake `EE_FoveaRequest`。
4. 写入 AXI-Lite shadow registers。
5. 最后写 `CONTROL.ENABLE | CONTROL.COMMIT`。
6. 读取 `STATUS`、`CYCLE_COUNT`、`QUALITY_FLAGS`。
7. 轮询 `STATUS.busy/done/error`，带软件 timeout，禁止死循环。
8. 用 `xil_printf` 打印结果。

期望输出形状：

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

如果 base address 仍是占位，输出会明确说明硬件访问被跳过。

第二级动态测试位于仓库根目录 `vitis_dynamic_fovea_smoke_test.c`。它不是
Vitis workspace 的自动生成文件，需在固定 smoke test 已成功后手动加入
Application。测试只使用五帧固定目标，依次覆盖 ROI smoother、Enhance
Policy、A adapter 和 AXI-Lite 写入；直到 UART 观察到 `REQUEST LATCHED`
之前，都必须标记为 `NOT HARDWARE VERIFIED`。

## Decode Helpers

Vitis 平台层提供纯软件解析函数，只解读寄存器值，不改变硬件定义：

```c
void fovea_decode_status(uint32_t raw_status, EE_FoveaAxiStatus *out_status);
void fovea_decode_quality_flags(uint32_t raw_quality_flags,
                                EE_FoveaQualityDecode *out_quality);
```

解析字段包括：

- `busy`
- `done`
- `error`
- `active_mode`
- `ROI_CLIPPED`
- `MODE_FALLBACK`
- `TARGET_SWITCH`
- `REQUEST_CANCELLED`
- `FRAME_SAFE_LATCHED`
- `OUTPUT_BUFFER_VALID`

## Next Board Validation Steps

1. 在 Vivado 中确认 Fovea AXI-Lite IP instance name。
2. 生成 bitstream 和 exported hardware platform。
3. 在 Vitis 中创建 bare-metal application。
4. 加入 `include/register_map.h`、`include/platform_io_vitis.h`、`src/platform_io_vitis.c`、`vitis_axi_smoke_test.c`。
5. 确认 `EAGLEEYE_FOVEA_BASEADDR` 指向 `xparameters.h` 中真实宏。
6. 下载 bitstream，运行 smoke test。
7. 先只检查寄存器写入和 `STATUS` 读回，不接摄像头、不跑图像算法。

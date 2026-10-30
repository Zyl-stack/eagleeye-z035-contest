# Eyes 实现的硬件与软件接口表

本表对应现场回传并确认功能正常的 `b286f480...` BIT 所配套的 [`system.hwh`](../board/reproduced_20261028/system.hwh) 与 `src/ps/src/` AttentionPolicy 源码；用于报告和版本核对，不是操作寄存器的指导。HWH 所列区间为地址空间分配，应用实际映射通常为 4 KiB。独立 PicoDet 注意力候选的执行文件不同，需以其对应构建资料为准。

| 模块 | HWH 分配的 AXI 地址区间（含两端） | 在现有程序中的用途 |
|---|---|---|
| `fovea_axil_ctrl_0` | `0x40000000–0x40000fff` | 人体关注区域、状态与人工接管控制；[`fovea_control.cpp`](../src/ps/src/fovea_control.cpp) 在写入前检查 `0x64` 的 `EEM2` 标识。 |
| `picodet_preproc_ip_0` | `0x40400000–0x4040ffff` | 图像缩放/归一化；[`paddle_person.cpp`](../src/ps/src/paddle_person.cpp) 检查版本、启动并读取状态和计数。 |
| `axi_dma_pre_in` | `0x40410000–0x4041ffff` | 内存到预处理 IP 的输入通道。 |
| `axi_dma_pre_r/g/b` | `0x40420000–0x4042ffff`、`0x40430000–0x4043ffff`、`0x40440000–0x4044ffff` | 三路输出到内存，分别对应 R/G/B 平面。 |
| `axi_vdma_0` | `0x43000000–0x4300ffff` | 应用源码中为显示 MM2S。 |
| `axi_vdma_1` | `0x43010000–0x4301ffff` | 应用源码中为摄像头采集 S2MM。 |

上述 HWH 地址可对照 [`paddle_person.cpp`](../src/ps/src/paddle_person.cpp)、[`frame_layout.h`](../src/ps/src/frame_layout.h) 与 [`eagleeye_hw.cpp`](../src/ps/src/eagleeye_hw.cpp)；不应将这些地址移植到其他版本而不复核 HWH、设备树与实际所有权。

## 缓冲、数据类型和控制

- 视频：源码配置 640×360、RGB 三字节、stride 1920，三个大小各 `0xb0000` 的帧槽，从 `0x1e000000` 起共 `0x210000` 字节。源码中的长度和设备树预留区应一起核对；不能把地址表当作现场 DMA 已停止或可与其他程序共享的证明。
- 预处理：源码映射 `0x1e300000` 起 4 MiB；输入从 `0x1e300000` 开始为 691200 字节，输出从 `0x1e400000` 开始，R/G/B 每平面各 409600 字节，合计 `1×3×320×320` FP32 NCHW。三个输出地址依次按单平面长度递增。
- ROI：应用通过 AXI-Lite 传入请求序号、区域坐标、尺寸、模式等并检查状态；`0x64` 的值是该模块接口标识，不等同于完整 BIT 现场身份认证。本地匹配 ID 不保证跨遮挡稳定。
- 实际程序使用 `/dev/mem` 映射指定内存及寄存器，并在源码中插入内存屏障；屏障不单独证明所有缓存或不同设备/系统的内存一致性，移植时需遵循目标平台的 DMA 契约。

## 时钟与复位范围

[`z100_camera_i2c_clocks.xdc`](../src/vivado/eagleeye_7035_eyes.srcs/constrs_1/new/z100_camera_i2c_clocks.xdc) 将 `ov5640_pclk` 暂定为 20 ns，并为 MS7210 I²C 驱动时钟指定分频；文件自身明确该 PCLK 为 bring-up 期间的暂定约束，实际时钟与外设延迟仍需板级依据。复位、视频时钟与跨域应以所构建 BD、[`cdc.rpt`](../build/clean_rebuild_20261028/reports/cdc.rpt) 和 [`methodology.rpt`](../build/clean_rebuild_20261028/reports/methodology.rpt) 联合核对。相关问题和限制汇总见 [告警说明](warning_review_zh.md)。

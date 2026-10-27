# 给 A/B/C 队友的复现状态说明（2026-10-27）

| 部分 | 可复现内容与证据 | 尚不能声称 |
| --- | --- | --- |
| A 板级视频 | 本包 Z035 Eyes `system.bd` 与本机原工程一致；63 个 `.srcs` 文件逐字节一致。`build/recreate_vivado.tcl` 已在 Vivado 2022.2、短 ASCII 路径下从干净副本创建工程，BD Validation 和 Output Products PASS。原工程 BIT 与 BOOT 打包输入 BIT 哈希一致。 | 本包干净重建的 BIT 尚未重新生成、上板。原 routed timing 报告有 16 项缺失 I/O delay 以及时钟相关 Critical Warning，摄像头/HDMI 板级接口时序没有完整签核。AX7020 A/Fovea 参考分支不能替代这份 Z035 工程。 |
| B 检测/跟踪 | 原稳定版 PicoDet 人体检测的 GridPolicy 源码在 `src/ps_as_built/`；配套模型 `data/model.nb`。用匹配工具链重新编译，二进制 SHA256 与原应用包相同。后续 `src/ps/` 的 AttentionPolicy 已独立编译、打包，用户反馈新版本上板通过。 | B 参考分支的足球 PLAYER/BALL 检测/跟踪未证明接入 Z035；不得与当前人体检测混称。注意力版仍缺一份可归档的完整串口日志/录像。 |
| C 注意力/调度 | `src/attention_reference/` 是提取的两文件参考子集，有明确 Makefile，GCC 编译 PASS；板端使用的是 `src/ps/` 的 AttentionPolicy。完整 C 仓库 `c-attention` 可定位到提交 `25bd7a35ab1a072f004e75cad20331a3d128c166`。 | C PC 仿真并非直接运行在 Z035 的完整工程；本子集也不是完整 C 工程。不得把 C 的性能或功能当作板端程序实测。 |

构建依赖与操作见 `build/REPRODUCE_ZH.md`。截至本次核对，**可独立打开/验证 BD，可精确重编原稳定应用，注意力应用已有独立归档并获用户实板通过反馈**；完整从源码重新实现 BIT、对新 BIT 全功能板测及保存本次注意力版原始串口/视频证据仍待完成。报告应把原稳定版、注意力版和 C PC 参考分开陈述。

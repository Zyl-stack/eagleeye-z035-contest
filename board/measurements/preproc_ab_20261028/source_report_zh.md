# Z035 实板预处理 A/B 测速结果（2026-10-28）

## 测试对象与方法

- 板端启动 BOOT SHA256：`816ebd03dc533f857399938e533994c1989f8af4437ce5b361329da6d461b039`，含队友复现 BIT `b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15`。本轮**使用新复现 BIT，不是旧稳定 BOOT `54003cd2…`**。
- 同一 `paddle_person_live` SHA256：`785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`；同一 `model.nb` SHA256：`7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`。
- 连接原摄像头及运行中 VDMA，线程数 2、PERSON 阈值 0.3、RGB 像素序、Fovea ROI/鼠标功能均启用。程序各自先进行 1 次未计时模型预热，再连续运行 CPU 与 FPGA 后端各 35 帧；各自剔除最初 5 帧，对后 30 帧统计。测试期间不进行人工按键或鼠标交互。
- CPU 路径使用软件**双三次**缩放与归一化；FPGA 路径使用预处理 IP 的**双线性**缩放与归一化。PicoDet 卷积推理两组均运行在 ARM 上。本表比较的是两套实际系统配置，而不是同算法的纯硬件加速比。
- 原始板端归档为 `results/EE_PREPROC_AB_XX6qMnuD.tar.gz`，SHA256 `0059dc49e95b127ff6d162c56b2e2f5a026909afcb98525550dbb85c9b05744a`。`cpu.log` SHA256 `4a17d06f327f7136683ed97dacdfccf985f606b7c39b43455468ef0bff0c46fc`；`fpga.log` SHA256 `01695a83a8eb6e4215d4d7b57ce4b8ed1b6e833cf2467e8e9f475e1e07bbadde`。

## 结果（后 30 帧）

| 指标 | CPU 预处理 | FPGA 预处理 | 变化 |
| --- | ---: | ---: | ---: |
| 摄像头采集平均 / ms | 17.485 | 16.878 | — |
| 预处理平均 / ms | 416.960 | 17.537 | FPGA 路径快 23.776 倍 |
| PicoDet 推理平均 / ms | 468.659 | 468.204 | 基本一致 |
| 后处理平均 / ms | 0.029 | 0.029 | 基本一致 |
| 完整检测链路平均 / ms | 903.150 | 502.665 | 缩短 400.485 ms，约 44.3% |
| 完整检测链路标准差 / ms | 5.915 | 5.149 | — |
| 有效检测帧率 / FPS | **1.107** | **1.989** | **1.797 倍，约提升 79.7%** |

有效 FPS 按 `1000 / 平均 total_ms` 计算，反映摄像头快照采集、预处理、ARM 推理和检测结果解析的单帧链路耗时，**不是 HDMI 显示刷新率，也不含计时点之后的 ROI 控制与串口打印**。FPGA 预处理耗时是板端应用实测，包含输入/输出内存复制、`/dev/mem` 映射后的寄存器访问、AXI DMA 启动与完成轮询、以及向 Paddle-Lite 输入张量复制；不是仅采用 RTL 仿真周期估算。本程序未单独记录 cache 维护或独立内核驱动耗时，不能声称测出了这两项。

## 证据边界

- 两份日志各有 35 条 `TIME`、35 条匹配的 `PREPROC backend` 标记；未见后端回退或程序异常。CPU 组 0 条 FPGA 标记，FPGA 组 35 条 FPGA 标记。
- 两组各有 35 条 `SNAPSHOT`，且 `content_changed=1`；同一行仍标记 `freshness=unverified`。`video_before.txt`、`video_between.txt`、`video_after.txt` 中的 VDMA 状态一致，但均提示已有 latched error bits，脚本未清除它们。故本结果可用于当前检测循环的实板耗时比较，不单独证明摄像头每帧硬件完成事件或长时间稳定性。
- 两组按先 CPU 后 FPGA 顺序获取实时摄像头画面，**不是同一张静态图像**；不能据此声称数值输出与 Golden 逐位一致或检测框完全不变。
- 统计可用 `python analyze_results.py results/EE_PREPROC_AB_XX6qMnuD` 重算。

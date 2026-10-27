# EagleEye Z035 摄像头辅助观察系统——设计报告材料稿

**状态：材料稿，尚需补齐最终测试数据与团队确认。** 本报告对应仓库 `original/eagleeye_z035.zip` 中的 Eyes 板级版本；不把独立 V2/V3 卷积实验当作该系统已经集成的功能。

## 1. 背景、目标和范围

系统面向快速运动场景的辅助观察：保留摄像头全景显示，使用人体检测产生关注区域，提供局部放大、暂停和鼠标复判，并由双路舵机跟随。它是观察辅助系统，现有证据不能支持自动裁判结论或完整检测准确率数值。

实现使用 XC7Z035、OV5640、HDMI 和 ARM/Paddle-Lite。视频局部放大、预处理和检测是不同功能；ROI 放大并不代表新增真实图像细节。创新点与已有工作的详细对比、团队各成员贡献以及模型和厂商 IP 的版权授权仍待团队补充核实。

## 2. 设计原理与软硬件分工

![系统架构图](figures/system_architecture.png)

图为本地已有的系统架构说明图，不单独构成实板验证。PL 侧包括摄像头/视频通路、VDMA、ROI 放大及 OSD、图像预处理和 PWM；PS 侧负责 PicoDet 人体检测、ROI 策略、鼠标及控制交互。源码分别位于 `src/vivado/`、`src/ip_repo/`、`src/servo/` 和 `src/ps/`。

![预处理 IP 架构图](figures/preproc_architecture.png)

预处理示例将 640×360 RGB888 转为 `1×3×320×320` FP32 NCHW；具体位宽、DMA/cache、时钟、地址和复位以匹配版本的源码、XSA 和板端配置为准，不能仅凭图确认。

## 3. 验证、资源与时序

| 项目 | 仓库证据 | 范围 |
|---|---|---|
| 预处理张量比较器 | `sim/host_preproc.log`：2 项主机测试通过 | 比较工具测试，不是当前系统板端 DMA 输出 |
| ROI 硬件合同检查器 | `sim/host_roi.log`：1 项主机测试通过 | 检查器测试，不证明按键、鼠标和舵机实板功能 |
| 预处理 IP 独立历史记录 | `sim/preproc_standalone_readme.md` 报告五类帧；六个 RTL/ROM 哈希见 `sim/preproc_standalone_rtl_sha256.txt` | 这是交接文档的历史结果，原始逐帧仿真日志未随本材料提供；不是整个 Eyes 系统的板端输出 |
| Lanczos 源码来源 | `sim/host_lanczos_source.log`：7 个文件哈希匹配 | 不验证像素；旧 XSim 日志 `skill/lanczos-2x-stream/build/xsim.log` 显示 768 项错误，不能写成 PASS |
| 历史实现资源 | `build/z100_top_utilization_placed.rpt`：LUT 24,789；FF 28,234；BRAM 303/500；DSP 49 | 历史 Eyes 布局报告，不是本次重新综合 |
| 历史时序 | `build/z100_top_timing_summary_routed.rpt`：WNS +0.505 ns，WHS +0.051 ns | Vivado 2022.2，已约束路径；不能代替外设时序和稳定性验证 |
| 历史 CPU 预处理 | `board/attach_100.log` 和 `report/board_measurements.md` | 100 帧统计；不能与其他版本直接相除作为 FPGA 加速比 |

文件完整性检查见 `tools/verify_submission.py`。主机测试只覆盖工具自己的行为，不代表当前板端计算或完整系统 PASS。

## 4. 优化前后比较

原工程包含预处理和 ROI 的硬件实现与历史记录，但目前未找到当前 Eyes 版本同场景、同模型和相同参数的 CPU/FPGA 预处理成对原始数据。优化前后延迟、检测更新 FPS 和准确率应保持待测；不得将 HDMI 刷新率或 IP 内部周期当作完整检测 FPS。

## 5. 大模型协作与技能包

`report/ai_collaboration_record.md` 为已有的协作整理稿，不是完整原始会话。`skill/index.md` 索引三项可复用内容，分别记录适用范围、使用方式、已有验证及失效条件；独立逐像素 Lanczos 验证未通过，不得作为成功案例。完整提示词、回复、纠错轨迹和团队确认需另行补充。

## 6. 复现

1. 执行 `python tools/verify_submission.py` 验证当前仓库与原始快照。
2. 将 `original/eagleeye_z035.zip` 解压到新目录，保留原始文件名大小写；按原快照 `README.md` 和 `build/recreate_vivado.tcl` 使用 Vivado 2022.2。
3. 按原快照 `sim/README.md`、`src/ps/build_arm.sh` 准备工具链、Paddle-Lite 依赖及测试输入。当前材料尚未证明从干净目录完成整套构建。
4. 上板验证应使用获准的独立介质，记录镜像身份、测试条件、原始输出和恢复情况；不覆盖稳定系统。

详细缺口和证据边界见 `submission_audit.md`。

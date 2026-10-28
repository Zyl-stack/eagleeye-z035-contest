# EagleEye Z035 摄像头辅助观察系统——设计报告材料稿

**状态：材料稿，尚需补齐最终测试数据与团队确认。** 本报告对应仓库 `original/eagleeye_z035.zip` 中的 Eyes 板级版本；不把独立 V2/V3 卷积实验当作该系统已经集成的功能。

## 1. 背景、目标和范围

系统面向快速运动场景的辅助观察：保留摄像头全景显示，使用人体检测产生关注区域，提供局部放大、暂停和鼠标复判，并由双路舵机跟随。它是观察辅助系统，现有证据不能支持自动裁判结论或完整检测准确率数值。

实现使用 XC7Z035、OV5640、HDMI 和 ARM/Paddle-Lite。视频局部放大、预处理和检测是不同功能；ROI 放大并不代表新增真实图像细节。下列内容是本项目可核查的设计特点与拟申报方向；本节已补充相关公开工作的定性对照；同条件性能对比尚未完成，不能据此宣称首次提出或业界领先。

### 1.1 选题动机与实际需求

在足球等快速运动的画面中，观察者既要理解运动员在场地中的相对位置，也可能需要看清人物附近的局部细节。目标运动、遮挡、距离和屏幕显示尺寸都会增加观察难度：只看全景时局部目标较小；只盯住局部又容易失去场景上下文。人工在暂停画面中寻找和调整关注区域，也需要反复操作。

团队因此选择“全景观察与局部复判相结合”作为本项目的切入点：在一套嵌入式设备上保留全景画面，尝试根据人体检测结果提出关注区域，并提供暂停后由鼠标调整 ROI 的方式，供使用者观察细节；双路舵机用于演示随 ROI 变化的机械控制。这里的“复判”指对**当前暂停画面**的人工观察；现有材料不证明具备多机位、历史片段回放、慢动作或轨迹回放功能。

这一方案希望减少使用者在全景和局部之间寻找目标、调整观察区域的操作负担。但目前没有针对观察效率、使用体验或判断准确率的对照实验，不能宣称已经量化证明效果；当前模型输出的是人体框，不包含已验证的足球检测或球员身份识别。单摄像头视角、画面遮挡、低分辨率、检测误差和放大不产生原始画面中不存在的细节，均限制观察结果。

本项目定位为教学与研究性质的画面观察辅助原型，**不是正式 VAR 系统，也不能自动作出比赛裁判结论**。其设计目标和已有功能应与实际采集、原始测试记录和最终演示版本一起理解，不能把目标直接写成实测效果。

### 1.2 相关工作及本项目的区别

| 方向及代表性已有工作 | 已有方案解决什么 | 本项目采用或借鉴的部分、主要区别 |
|---|---|---|
| 专业视频回看与 VAR | FIFA 的场馆技术说明介绍了使用比赛转播画面进行视频复核，以及可用于分析的光学跟踪系统 [1]；IFAB 的 VAR 协议规定了受限的复核情形、程序和裁判最终决定权 [2]。 | EagleEye 结合单摄像头全景、暂停画面局部观察与人工 ROI 控制，旨在做观察辅助。现有工程没有证明实现专业 VAR 的多机位回放、赛事流程、认证或裁判系统能力，因此不能称其为 VAR 替代品。 |
| 轻量目标检测 | Yu 等提出 PP-PicoDet，研究轻量骨干、检测结构及移动端精度与效率的权衡 [3]；PaddleDetection 提供 PicoDet 的模型配置和部署说明 [4]。 | 本项目使用已有 PicoDet/Paddle-Lite 方案在 PS 侧产生人体框，并将其接入注意力和 ROI 控制。PicoDet 不是团队原创；论文中的移动设备性能也不是 XC7Z035 的实测性能。已有临时 ID 不等于跨遮挡稳定跟踪。 |
| FPGA 图像处理与预处理 | AMD 的 Vitis Vision 库提供颜色转换、分辨率变换等硬件图像处理功能 [5]，说明这类运算已有成熟的可编程逻辑实现思路。 | 本项目将已有工程中的 PL 预处理接入摄像头、PicoDet 与视频观察链路，并以张量 Golden 校验指定仿真输入。不能把缩放或 FPGA 预处理本身表述成首次提出；当前系统同条件板端加速比仍待测试。 |

**本项目拟体现的贡献**是上述功能面向同一观察场景的软硬件组合：将视频显示和预处理、PS 人体检测、ROI 人工接管及舵机控制组织成一套 XC7Z035 原型，并保留对应源码与复现材料。已有工作分别覆盖视频复核、检测和硬件图像处理；本报告没有与其他完整系统开展同条件实验，因此不声称本原型在成本、精度、速度或适用性上优于它们。具体实现与验证边界见下表及第 3、4 节。

**参考文献与资料**（网页访问日期：2026-10-28）：

1. FIFA. [Football Stadiums Guidelines: Players and Officials — Football Technology](https://publications.fifa.com/de/football-stadiums-guidelines/technical-guideline/main-user-groups/players-and-officials/). 场馆视频裁判和光学跟踪系统说明。
2. IFAB. [Video Assistant Referee (VAR) protocol, Laws of the Game 2025/26](https://www.theifab.com/laws/2025-26/video-assistant-referee-var-protocol/). 正式 VAR 的适用范围与复核流程。
3. Yu G, Chang Q, Lv W, et al. [PP-PicoDet: A Better Real-Time Object Detector on Mobile Devices](https://arxiv.org/abs/2111.00902). arXiv:2111.00902, 2021.
4. PaddlePaddle. [PaddleDetection PicoDet 模型与部署说明，release/2.6](https://github.com/PaddlePaddle/PaddleDetection/blob/release/2.6/configs/picodet/README_en.md). 模型和 Paddle-Lite 部署资料。
5. AMD. [Vitis Vision Library User Guide](https://docs.amd.com/r/en-US/Vitis_Libraries/vision/overview.html?contentId=tllvbqJ7TwA66BDW6JeAcw). 图像处理库功能参考；该页面不证明本项目直接调用了该库。

### 1.3 可核查的设计特点

| 特点 | 项目实现与证据 | 目前能说明的范围 |
|---|---|---|
| 全景观察与局部复判结合 | Eyes 视频通路保留全景显示，ROI 局部缩放用于观察；鼠标、按键和暂停/恢复控制见 `src/ps/` 和 `src/vivado/` | 工程和交互路径已有源码；A 对新 BIT 的总体功能反馈正常，逐项日志仍缺。放大不能增加原图真实信息量 |
| 预处理的软硬件划分 | PL 将 640×360 RGB888 转换为 320×320 FP32 NCHW，PS 负责 PicoDet 推理；`auto` 路径可退回 CPU。见 `src/ip_repo/preproc/` 和 `src/ps/src/paddle_person.cpp` | 五类帧离线 RTL 仿真与参考逐位一致；本次板端张量对照和同条件速度增益未提供 |
| 人体框驱动注意力与 ROI | `src/ps/src/fovea_roi.hpp` 将区域、运动、接近、小目标和不确定度评分并限制目标切换；独立 `src/picodet_c_candidate/` 接入 C 注意力和 ROI 平滑 | 属于具体版本的软件策略；ID 为临时匹配，不能声称跨遮挡稳定追踪；C 候选的现场细节缺少日志 |
| 可追溯验证和复现 | 保存输入、参考、RTL 日志、构建报告、BIT/XSA/HWH 与校验脚本；示例见 `sim/verification_zh.md` 和 `report/board_reproduced_20261028_zh.md` | 有助于复核指定产物；哈希、仿真和现场反馈各自有边界，不等同于系统精度或长期可靠性证明 |

这些特点是已有部件的系统组合与实现方法，不代表 PicoDet、Lanczos 或厂商 IP 均为团队原创。有关第三方来源、授权和与已有工作的对照须由团队最终核实。

## 2. 设计原理与软硬件分工

![系统架构图](figures/system_architecture.png)

图为本地已有的系统架构说明图，不单独构成实板验证。PL 侧包括摄像头/视频通路、VDMA、ROI 放大及 OSD、图像预处理和 PWM；PS 侧负责 PicoDet 人体检测、ROI 策略、鼠标及控制交互。源码分别位于 `src/vivado/`、`src/ip_repo/`、`src/servo/` 和 `src/ps/`。

![预处理 IP 架构图](figures/preproc_architecture.png)

预处理示例将 640×360 RGB888 转为 `1×3×320×320` FP32 NCHW；具体位宽、DMA/cache、时钟、地址和复位以匹配版本的源码、XSA 和板端配置为准，不能仅凭图确认。

### 数据流与接口边界

1. 摄像头数据进入 Eyes 视频链路；PS 程序从视频帧获取检测输入。HDMI 显示与检测更新是不同速率，不能用画面刷新率代表检测 FPS。
2. `src/ps/src/paddle_person.cpp` 中的预处理路径将 RGB 帧放入缓冲区，通过一条输入 DMA 和三条 R/G/B 输出 DMA 调用预处理 IP，再形成 NCHW 张量供 Paddle-Lite 使用；`auto` 模式下遇到不可用或运行错误会退回 CPU。源码中的寄存器和缓冲地址是该版本的配置示例，移植时必须对照 XSA、DT 和实际内存归属。
3. PicoDet 输出人体框后，`src/ps/src/fovea_roi.hpp` 中的 AttentionPolicy 对目标筛选、临时匹配并产生 ROI。该本地 ID 不保证跨遮挡稳定；无目标时清除匹配历史并保留最近安全 ROI。
4. 鼠标与 Fovea 请求经过已有控制路径，舵机由相应 PWM RTL 控制。独立 `src/picodet_c_candidate/` 还将人体框送入 C 注意力与调度逻辑；它与稳定 GridPolicy、后续 AttentionPolicy 是不同软件版本，不应混称同一程序。

接口源码说明预期行为，不单独证明现场调用、DDR 一致性或舵机动作。A 已反馈新候选功能正常，但登录后应用原始输出仍未提供。

### 2.1 团队分工与接口

以下按本地团队分工草案及后续交接整理为**职责范围**，不是未经确认的个人代码归属或工作量统计。早期草案面向 AX7020；本报告只将其职责划分用于说明 Z035 项目的组织方式，最终贡献请团队确认。

| 角色 | 分工范围 | 当前可查交接与边界 |
|---|---|---|
| A：板级与视频/Fovea | 视频输入输出、ROI/显示通路、工程集成及板端测试 | Eyes 工程和构建见 `build/full_rebuild_20261027/`；A 回传的 BIT 和启动串口见 `report/board_reproduced_20261028_zh.md`。功能正常为现场反馈 |
| B：模型与目标感知 | PicoDet 人体检测的模型、推理与人体框接口核对 | `data/model.nb`、`src/ps/` 及 `report/b_detection_integration_20261027.md`；当前 Z035 包未提供跨遮挡稳定 target_id。足球 YOLO 资料是独立参考路线，不能当成当前程序 |
| C：注意力、调度与接口 | 对人体框做目标选择、ROI 请求与系统接口适配 | `src/picodet_c_candidate/` 和 `sim/picodet_c_candidate/`；有主机/QEMU 测试及用户转述的现场反馈，尚无该候选的完整实板运行日志 |
| 团队与 AI 协作 | 需求拆分、接口核对、构建/测试和交付资料整理；实板操作与结论由团队核验 | `report/ai_collaboration_record.md` 是整理稿，不是完整原始聊天；模型、第三方 IP 作者与授权待补充 |

在 `src/ps/src/fovea_roi.hpp` 的 AttentionPolicy 示例中，五项分数依次以 3、1、3、2、3 相乘后求和；这些是人体目标选择策略参数，不表示球参与评分，也不证明数值经准确率或跟踪稳定性实验最优。独立 C 候选应以自己的源码和配置为准。

## 3. 验证、资源与时序

| 项目 | 仓库证据 | 范围 |
|---|---|---|
| 预处理张量比较器 | `sim/host_preproc.log`：2 项主机测试通过 | 比较工具测试，不是当前系统板端 DMA 输出 |
| 预处理 RTL 仿真 | `sim/preproc_iverilog_20261027.log`：五类帧及采集帧适配器联测逐位一致，每帧 307,200 值 | Icarus 10.3 仿真；不证明实板 DMA、缓存或整个视频系统 |
| 舵机 PWM RTL 仿真 | `sim/servo_iverilog_20261027.log`：中心保持、周期、渐变及反向测试通过 | 离线 RTL 仿真，不证明实物舵机机械安全 |
| ROI 硬件合同检查器 | `sim/host_roi.log`：1 项主机测试通过 | 检查器测试，不证明按键、鼠标和舵机实板功能 |
| 预处理 IP 独立历史记录 | `sim/preproc_standalone_readme.md` 报告五类帧；六个 RTL/ROM 哈希见 `sim/preproc_standalone_rtl_sha256.txt` | 这是交接文档的历史结果，原始逐帧仿真日志未随本材料提供；不是整个 Eyes 系统的板端输出 |
| Lanczos 源码来源 | `sim/host_lanczos_source.log`：7 个文件哈希匹配 | 不验证像素；原始旧 XSim 日志 `sim/lanczos_legacy_xsim.txt` 显示 768 项错误，不能写成 PASS |
| 历史实现资源 | `build/z100_top_utilization_placed.rpt`：LUT 24,789；FF 28,234；BRAM 303/500；DSP 49 | 历史 Eyes 布局报告，不是本次重新综合 |
| 历史时序 | `build/z100_top_timing_summary_routed.rpt`：WNS +0.505 ns，WHS +0.051 ns | Vivado 2022.2，已约束路径；不能代替外设时序和稳定性验证 |
| 独立 Eyes 重建 | `build/full_rebuild_20261027/`：LUT 24,986、FF 28,246、BRAM 303、DSP 49；WNS +0.608 ns、WHS +0.007 ns | 本机独立源码副本完成实现；12 项输入/33 项输出延迟缺失，CDC 和方法学仍有 Critical，不能代表完整时序签核 |
| 复现 BIT 上板 | `report/board_reproduced_20261028_zh.md`；BIT SHA256 `b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15` | A 反馈功能正常；U-Boot 到 Linux 登录有原始串口，FSBL、登录后应用和逐项功能记录未录到 |
| PicoDet+C 候选上板反馈 | `src/picodet_c_candidate/readme.md` | 用户确认 A 使用该候选后反馈功能正常；尚无该次串口、哈希读回和逐项测试记录 |
| 历史 CPU 预处理 | `board/attach_100.log` 和 `report/board_measurements.md` | 100 帧统计；不能与其他版本直接相除作为 FPGA 加速比 |

文件完整性检查见 `tools/verify_submission.py`。主机测试只覆盖工具自己的行为，不代表当前板端计算或完整系统 PASS。

各测试的输入、比较判据、执行记录和复现入口见 [仿真与验证说明](../sim/verification_zh.md)。独立实验的 ARM98 和 V2/R3 实板记录可作开发背景，不能称为本报告 Eyes 系统的同条件性能数据，详见 [历史实板证据](board_evidence_20261027.md)。

## 4. 优化前后比较

原工程包含预处理和 ROI 的硬件实现与历史记录，但目前未找到当前 Eyes 版本同场景、同模型和相同参数的 CPU/FPGA 预处理成对原始数据。优化前后延迟、检测更新 FPS 和准确率应保持待测；不得将 HDMI 刷新率或 IP 内部周期当作完整检测 FPS。

可引用的历史基线为 `board/attach_100.log` 的 CPU 预处理 100 帧：平均预处理 418.061 ms，平均 total 913.653 ms，1000 / 平均 total 约 1.09451 FPS，计算方式及边界见 [记录说明](board_measurements.md)。它没有匹配当前 BIT 的同条件 FPGA 对照，因此不计算加速比。

## 5. 大模型协作与技能包

`report/ai_collaboration_record.md` 为已有的协作整理稿，不是完整原始会话。`skill/index.md` 索引两项已有校验内容及一项 PYNQ 模拟候选，分别记录适用范围、使用方式、已有验证及失效条件；Lanczos 旧测试日志失败，测试台与当前版本的对应关系未核实；当前逐像素验证仍未完成，不得作为通过案例。完整提示词、回复、纠错轨迹和团队确认需另行补充。

可复核的协作案例包括：针对板端 BusyBox 差异修订只读采集；从固定输入的 V2/R3 实测发现卸载比 ARM 慢，保留真实日志而不将局部 Golden 等同整网提速；依据团队要求仅移除 Lanczos 的独立 Skill 展示，保留旧测试及失败记录；A 回传启动串口后，将 U-Boot 至 Linux 登录与登录后的现场功能反馈分开。各案例的来源、纠错和局限见 `report/ai_collaboration_record.md` 和相关证据页面，不应把整理稿冒充完整聊天逐字稿。

## 6. 复现

1. 执行 `python tools/verify_submission.py` 验证当前仓库与原始快照。
2. 先运行 `python tools/restore_a_repro.py --output <新目录>` 恢复 A 快照，再按 `report/reproduction_zh.md` 使用 `build/build_full_vivado.tcl` 运行 Vivado 2022.2。
3. 按 `report/reproduction_zh.md` 及原快照 `sim/README.md`、`src/ps/build_arm.sh` 准备工具链、Paddle-Lite 依赖及测试输入。独立构建已覆盖 Eyes FPGA、稳定 ARM 应用和固定源码 Linux 内核；A 所提供的本次板测 BIT 与复现 BIT 哈希一致，实验卡读回清单及现场应用身份仍待核对。
4. 上板验证应使用获准的独立介质，记录镜像身份、测试条件、原始输出和恢复情况；不覆盖稳定系统。

详细缺口和证据边界见 `submission_audit.md`。

# EagleEye C 部分进度

## Player-driven Attention V3 A/B

- Added optional `PLAYER_DRIVEN_V3` alongside the unchanged default `SOCCER_V2` mode.
- V3 uses valid PLAYER-only Scheduler candidates, while BALL remains optional context and cannot become the selected target.
- Added fixed-memory, target-ID-keyed player and pair history for Motion, Interaction, Motion Saliency, Uncertainty, Goal Zone and Aging features. No frozen B/C descriptor field changed.
- Goal Zone and BALL context are disabled by default pending calibrated field geometry and reliable BALL confidence data. Small Target remains an Enhance Policy input, not a V3 ranking input.
- Added V3 feature-unit tests, a four-way real-stream A/B harness, and C-bridge/Python read-only V3 diagnostics. The supplied 272-frame real stream completed V3 with original and BALL-masked inputs, no selected BALL targets, and no invalid Fovea transactions.
- Added `PLAYER_DRIVEN_ATTENTION_V3.md`. This is a behavior and robustness experiment without human action ground truth; it does not claim that a selected PLAYER is the semantically correct broadcast subject.

## BALL Challenger Eligibility Gate

- Added an independent, configurable Scheduler-only gate after Attention and before Scheduler.
- Default challenger threshold is confidence `25`, chosen from a real football-stream replay; it does not change Attention scores or frozen B/C fields.
- A locked, valid BALL remains eligible below the threshold; PLAYER targets are unchanged.
- Added threshold, incumbent protection, player passthrough, and Scheduler handoff tests. See `C_BALL_CHALLENGER_GATE.md`.

## Feedback-Aware Attention V1

- Added independent `feedback_adjustment` after the unchanged base Attention formula: `final_attention = base_attention + bounded feedback_adjustment`.
- Added `EE_FeedbackAttentionPolicy` to `EE_CConfig`. It is disabled by default; only the PC visual bridge explicitly selects the experimental enabled profile.
- Accepted feedback must still match `request_id`, `source_frame_id`, and `target_id`; it is target-specific, bounded, and age-limited. Stale, mismatch, duplicate, and invalid feedback cannot affect Attention.
- Added C decision/JSON/Python fields for base attention, feedback influence, final attention, reason, and transaction age. Python displays C output only.
- Added focused default-disabled B 12-frame sample, accepted, timeout, rejected-status, expiration, strong-target, close-score/hold, and full regression coverage.
- Added `C_FEEDBACK_AWARE_ATTENTION_V1.md`. This is PC transaction-level feedback verification only; no refined confidence, bbox, class, quality interpretation, or high-resolution hardware detection is claimed.

## Competition Features V2

- Added deterministic adaptive Enhance Mode policy with frozen mode IDs 0/1/2 and explicit reasons.
- Added display-only Auto Director states/events without changing Scheduler selection.
- Added per-frame Decision Snapshot and text explanation using the production Attention breakdown.
- Added cumulative foveated pixel-workload statistics: fixed ROI is 25% of the PC frame, so theoretical pixel reduction is 75% (not measured bandwidth or power savings).
- Integrated Decision, Attention, and Statistics panels into the PC visualizer through the C JSON bridge.
- Added `tools/run_competition_demo.py`: a Python PC-only driver that runs the C Mini Match bridge, visualizes C JSONL output, or writes a headless event trace without duplicating any core algorithm.
- The visual bridge now exports both the C desired ROI and smoothed ROI for display-only comparison; the Mini Match still uses its real C mode output rather than Python-selected modes.
- Extended all eight match regression scenes and added focused V2 policy/state/statistics tests.

## Latency-Aware Prediction

- Added `EE_LatencyModel` to `EE_CConfig`: processing, request, and frame-latch delay in configurable frame units.
- Added `ee_motion_predict_with_latency()` while preserving the original one-frame `ee_motion_predict()` API.
- Default latency compensation is zero frames, so existing scenarios retain their V1 prediction behavior until board timing is measured.
- Added PC-only comparison tests for stationary, positive/negative velocity, fast ball, abrupt reversal, and A-side fixed-ROI boundaries.
- Fast constant-velocity ball test improves target-to-ROI-center error from current-position `100 px` and existing prediction `75 px` to latency-aware `0 px`; abrupt reversal explicitly demonstrates the constant-velocity limitation.
- Added `C_LATENCY_AWARE_PREDICTION.md`. Delay values are CONFIGURED / ESTIMATED, not measured hardware latency.

## Dual-Mode ROI Control

- Added a lightweight TRACK / FAST_JUMP controller between the desired ROI and the existing integer ROI smoother; it changes smoother step limits only, not Attention, Scheduler selection, Switch Margin, Hold Time, or AXI-Lite interfaces.
- Default TRACK remains `32x18 px/frame`. FAST_JUMP uses `128x72 px/frame` for three frames through the same bounded smoother.
- FAST_JUMP triggers only for a valid target on pass start, ball recovery, current-target loss/invalidity, or a true selected target-ID change. HOLD, margin suppression, attack-zone-only events and invalid-ball loss remain TRACK.
- The PC C bridge exports the selected ROI motion mode, remaining fast frames and applied step limits. Python only displays this C output.
- Added `C_DUAL_MODE_ROI_CONTROL.md` and focused controller tests, including a 10-frame TRACK versus 3-frame FAST_JUMP displacement comparison and latency-aware fast-ball integration.

## Fovea Feedback V1

- Added a fixed-memory transaction feedback store with strict `request_id` + `source_frame_id` + `target_id` validation and explicit PENDING/ACCEPTED/TIMEOUT/STALE/mismatch/INVALID/DUPLICATE outcomes.
- Added a PC-only feedback candidate simulator for exact, stale, request/frame/target mismatch, invalid, and quality-flag cases. It does not simulate image processing or refined detection.
- Added Decision Snapshot and C JSON bridge feedback fields; the Python visualizer displays C-produced feedback status, request, target, and quality flags only.
- Confirmed Feedback V1 is observability-only: it does not change Attention, Scheduler target selection, Hold, Switch Margin, ROI control, or AXI-Lite definitions.
- Added `C_FOVEA_FEEDBACK_V1.md`. Current Vitis hardware can read STATUS, CYCLE_COUNT, and QUALITY_FLAGS, but cannot yet return the transaction ID triple needed for a tagged hardware feedback claim.

## Pre-board A/C Audit

- Audited V2 portability: Attention, Scheduler, ROI smoother, Enhance Policy and Director are suitable for standalone source compilation; text printing, PC runtime logging and floating-point display statistics remain outside the minimal smoke path.
- Added `vitis_dynamic_fovea_smoke_test.c` as a second-level, five-frame source-only Vitis test. It uses fixed fake targets and the real C ROI smoother, Enhance Policy, A adapter and Vitis AXI write path.
- Added `A_C_BOARD_BRINGUP.md` with Level 1 fixed ROI, Level 2 dynamic ROI, UART expectations and fault isolation.
- ARM GCC source compilation against the generated standalone BSP succeeded. No FPGA, JTAG, UART hardware run or image pipeline verification was performed.

## 当前选题

面向足球赛事观赛的 FPGA 仿生主动视觉/智能关注与自动特写系统。当前阶段仍为纯 C 软件模拟，未连接真实 FPGA。

## 已完成

- 模块化 C 工程，.h/.c 分离，固定宽度整数接口。
- 复用并验证 Global Target、Fovea Request、Enhanced/Refined Result 结构。
- Attention Scheduler 支持 argmax、Switch Margin、Hold Time、Aging 和 timeout 接口。
- Attention 迁移为足球语义：Ball Priority、Motion、Goal Zone、Small Target、Uncertainty、Ball Proximity、Aging。
- 权重集中在 include/attention_config.h，输出每个分项和总分。
- Fake 足球目标和可重复场景：正常移动、快速运动、Goal Zone、遮挡/消失、接近候选竞争。
- 新增 motion_predictor.c/.h，使用常速度模型预测下一帧 ROI；首次出现时不预测。
- 保留状态机总体架构：GLOBAL_SCAN -> CANDIDATE -> FOVEA_LOCK -> ENHANCING -> REFINE_INFER -> CONFIRMED -> HOLD -> RELEASE。
- 保留 request_id/target_id/frame_id 严格匹配、stale reject、timeout、release 清理和 Risk Action 一次性事件机制。
- 保留 AXI-Lite register map、Vitis 平台边界和 smoke test，不写死 Base Address。
- 新增 SOCCER_HARDWARE_BRINGUP_CHECKLIST.md，整理足球版真实上板前的 Vivado/Vitis/UART 步骤和硬件 TODO。
- 新增 possession_estimator.c/.h：以距离、运动连续性和有限历史计算 Estimated Ball Carrier。
- PC Mini Match 升级为 Ball-Carrier-Oriented Simulation，支持 #7 -> #10 传球焦点交接和 Player+Ball Joint ROI。
- Visualizer 增加 Possession Score、Estimated Carrier、轨迹、预测 ROI 和目标 ID 一致性检查。
- 根据 A 侧确认接口新增 a_fovea_adapter.c/.h：输入坐标 640x360，动态/Joint ROI 转固定 320x180 左上角窗口，范围限制为 x=0..320、y=0..180。
- 固定 mode 语义：0=BYPASS、1=BILINEAR 2x、2=LANCZOS 2x，并保留 use_lanczos 到明确 mode 的兼容映射。
- Vitis 寄存器写入改为可测试序列：全部参数先写，CONTROL.ENABLE | CONTROL.COMMIT 最后写。
- 明确 STATUS.done 是帧边界请求锁存确认，不是图像增强处理完成。
- 已复核 A commit 729a8d0：fovea_axil_ctrl RTL、PS M_AXI_GP0/AXI Interconnect/时钟/复位/视频控制连接、0x40000000 4K 地址段及含 bitstream 的 XSA 均已存在。
- C/A 寄存器 offset、CONTROL/STATUS、ROI_XY/ROI_WH、mode 和 QUALITY_FLAGS 完全一致，无需修改冻结 register map。
- B->C V1 ASCII stream 已冻结并增加严格损坏输入拒绝测试；原 12 帧样例继续验证稳定 ID 和第 8/9 帧遮挡恢复。
- Scheduler 已暴露 FIRST_TARGET、KEEP_CURRENT、HOLD_ACTIVE、MARGIN_NOT_MET、HIGHER_ATTENTION、CURRENT_INVALID、CURRENT_LOST 决策原因，不改变原比较逻辑。
- 新增可关闭的统一 C runtime log、PC 轻量计时、Attention breakdown 一致性接口和 EE_CConfig 默认配置聚合。
- 新增独立整数 ROI smoother，限制每帧移动并保证最终固定 ROI 为 320x180、x=0..320、y=0..180。
- 增加 8 类足球比赛回归：正常跟踪、快速传球、球/球员遮挡、接近竞争、注意力突增、长时间丢球、边界运动。

## 验证结果

Visual Studio 2026 x64 Debug 构建通过，未出现 warning/error。run_tests_msvc.bat 通过：

- CONFIRMED
- DOWNGRADED
- TIMEOUT
- STALE RESULT REJECTED
- request_id、target_id、frame_id 错配拒绝
- 足球 BALL focus、Goal Zone、运动预测、Switch Margin/Hold、长时间消失输入
- Possession Estimator：近球、传球交接、运动连续性、历史稳定和旧球员释放
- Mini Match：球员焦点交接、BALL 遮挡、PLAYER 遮挡、确定性重放
- A/C Adapter：动态 ROI、左上/右下 clamp、中央 ROI、模式映射、固定 ROI_WH、CONTROL 位和 COMMIT 写入顺序
- C 稳定性：Scheduler reason、Attention breakdown、统一配置、ROI 平滑和边界
- B->C robustness：重复/倒退帧、count 不符、缺 END_FRAME、非法类型/置信度/bbox/ID/帧号
- 8 类比赛回归场景及标准 frame/selection/reason/possession/attention/ROI/request 输出

## 当前边界

Fake 数据不代表真实 B 模块检测结果；Motion Predictor V1 不代表 Kalman 或深度学习跟踪；2x ROI 重采样不恢复不存在的细节；Vitis 文件不代表真实 AXI-Lite 已上板。

## 下一步

使用 A 提供的 system_wrapper.xsa 刷新 Vitis Platform/BSP，从生成的 xparameters.h 获取实际 Fovea BASEADDR 宏名，映射到 EAGLEEYE_FOVEA_BASEADDR，然后执行板上串口 smoke test。

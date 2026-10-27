# EagleEye Soccer Foveated Broadcast Simulation

EagleEye 是一个面向足球赛事观赛的 FPGA 仿生主动视觉系统。系统从全局比赛画面中得到多个候选目标，通过轻量、可解释的 Attention 选择当前最值得关注的足球或球员，结合常速度运动预测生成动态 Fovea ROI，再由后续 FPGA 模块对局部区域进行 2x 增强。

当前版本是纯 C PC 软件模拟，不连接 FPGA、摄像头、HDMI、DMA 或 VDMA。2x 插值只表示对已有 ROI 的 Crop 加 Bilinear/Lanczos 重采样，不声称恢复原图中不存在的真实细节，也不用于裁判判罚。

## 队友阅读顺序

1. include/eagleeye_types.h：共享目标、请求和结果结构体。
2. src/fake_data.c：足球、球员和比赛场景的可重复 fake 输入。
3. src/attention.c：七项 Attention 分量和总分。
4. src/attention_scheduler.c：argmax、Switch Margin、Hold Time 和 Fovea Request。
5. src/motion_predictor.c：常速度、可配置延迟补偿预测器 V1。
6. include/roi_motion_controller.h、src/roi_motion_controller.c：TRACK / FAST_JUMP 双模式 ROI 限速控制器。
7. include/fovea_feedback.h、src/fovea_feedback.c：Fovea 请求反馈三元组匹配、状态和固定内存反馈存储。
8. include/feedback_attention.h、src/feedback_attention.c：默认关闭、按目标生效且有界的 Feedback-Aware Attention 修正。
9. src/transaction.c：request/target/frame 严格匹配、stale reject、timeout 和一次性事件。
10. src/state_machine.c：保留的状态机骨架。
9. include/a_fovea_adapter.h、src/a_fovea_adapter.c：将动态/Joint ROI 转换为 A 侧固定 320x180 ROI，并生成寄存器写入顺序。
9. tests/test_scenarios.c、tests/test_a_fovea_adapter.c：事务、足球场景和 A/C 接口适配测试。
10. include/bc_adapter.h、src/bc_adapter.c：B ASCII 目标流校验与 B->C 适配层。
11. B_C_PC_INTEGRATION.md：B->C 流协议、运行方式和边界。
12. B_C_INTERFACE_V1.md：冻结的 B->C V1 字段、类型和错误处理规则。
13. C_RUNTIME_TUNING.md：Scheduler、Attention、ROI 平滑和诊断参数说明。
14. A_C_INTERFACE_ANALYSIS.md、A_C_INTEGRATION_PLAN.md：A 侧真实工程检查结果和联调计划。
15. SOCCER_MIGRATION.md：本次选题迁移说明。
16. SOCCER_HARDWARE_BRINGUP_CHECKLIST.md：足球版真实上板前清单。
17. BALL_CARRIER_FOCUS.md：持球队员估计、传球交接和 Joint ROI 说明。
18. C_COMPETITION_FEATURES_V2.md：自适应增强、自动导播、决策快照和理论资源统计。
19. C_FOVEA_FEEDBACK_V1.md：Fovea 反馈事务三元组、超时/陈旧结果边界和真实硬件反馈限制。
20. C_FEEDBACK_AWARE_ATTENTION_V1.md：Feedback adjustment 的边界、原因、年龄限制和 PC 验证范围。

## 软件流程

Global Match View -> Detection/Tracking -> Possession -> Attention -> BALL Challenger Gate -> Scheduler -> Director State -> Latency-Aware Prediction -> ROI Motion Controller -> ROI Smoothing -> A Adapter -> Fovea Request -> Feedback Validation -> Local View

其中 Detection/Tracking 输入、Attention、Scheduler、Director、Enhance Policy 和 ROI 都在 C 软件链路中运行。当前 Local View 是 PC simulation/demo；尚未完成真实板卡图像链路验证，不能标记为 hardware verified。

足球目标类型为 BALL、PLAYER、REFEREE、UNKNOWN；场地区域第一版为 FIELD_NORMAL、ATTACK_ZONE、GOAL_ZONE。

Attention 公式为：

Attention = w_ball*BallPriority + w_motion*Motion + w_goal*GoalZone + w_small*SmallTarget + w_uncertainty*Uncertainty + w_ball_near*BallProximity + w_aging*Aging

Uncertainty = 255 - coarse_conf。权重集中在 include/attention_config.h，没有自动优化。

Feedback-Aware Attention V1 保持该 `base_attention` 公式不变，并显式计算
`final_attention = base_attention + bounded feedback_adjustment`。默认配置
关闭反馈修正；PC visual demo 才显式启用实验配置。只有严格匹配的、未过期
反馈能够影响同一个 `target_id`，陈旧/错配/重复/无效反馈永远是 0 调整。

低置信的新 BALL challenger 会在 Attention 后、Scheduler 前经过独立 eligibility gate。默认门槛为 confidence `25`；当前锁定且有效的 BALL 和所有 PLAYER 均不受影响。详见 `C_BALL_CHALLENGER_GATE.md`。

## 构建和运行

Visual Studio：打开 EagleEye_C_Attention_Sim.sln，选择 Debug、x64，按 F5 调试或按 Ctrl+F5 运行。

在 Visual Studio Developer Command Prompt 中运行 build_msvc.bat 和 run_tests_msvc.bat。

PC 构建只使用 platform_io_sim.c；platform_io_vitis.c 是单独的 Vitis 平台实现，不能和模拟平台同时链接。

B->C PC 联调：运行 `build\\bc_stream_demo.exe` 可读取 `examples/b_target_stream_v1_sample.txt`，依次输出 B 输入目标、C Attention/Scheduler 的选中目标和 Fovea ROI。追加 `--summary`、`--verbose` 或 `--perf` 可启用统一运行日志和可选 PC 计时。详见 B_C_PC_INTEGRATION.md。

## A/C 接口使用

C 的 Scheduler、Motion Predictor 或 Joint ROI 仍可产生动态 ROI。Vitis 下发前调用 a_fovea_adapter，将动态区域中心转换为 A 侧 640x360 坐标系中的固定 320x180 窗口：

fixed_x = clamp(center_x - 160, 0, 320)

fixed_y = clamp(center_y - 90, 0, 180)

模式定义为 0=BYPASS、1=BILINEAR 2x、2=LANCZOS 2x。platform_io_vitis.c 已通过 Adapter 写入 REQUEST_ID、SOURCE_FRAME_ID、TARGET_ID、ROI_XY、ROI_WH、ENHANCE_MODE、OUTPUT_BUFFER、DEADLINE，最后才写 CONTROL.ENABLE | CONTROL.COMMIT。

STATUS.done 只表示请求已在帧边界锁存（REQUEST_LATCHED），不表示 Bilinear/Lanczos 图像处理已经完成。

## 测试覆盖

- 正常足球关注：BALL 竞争胜出并生成 Fovea Request。
- 快速运动：运动分量提高，预测器给出下一帧 ROI。
- GOAL_ZONE：目标关注度提高并映射为 FOCUS/HIGHLIGHT 语义。
- 短暂遮挡、长时间消失：为预测/保持/timeout 接入预留测试入口。
- CONFIRMED、DOWNGRADED、TIMEOUT。
- STALE RESULT REJECTED，以及 request/target/frame 三种错配。
- Switch Margin/Hold Time 防止接近分数候选频繁切换。
- 持球队员估计、传球交接和球员+足球 Joint ROI（PC Fake Match）。
- 动态 ROI 转固定 320x180、四角/中央 clamp、BYPASS/BILINEAR/LANCZOS 模式映射和 COMMIT 最后写入。
- Scheduler 决策原因、Attention breakdown 总分一致性、整数 ROI 最大步长平滑及 8 类比赛回归场景。
- 自适应 BYPASS/BILINEAR/LANCZOS 策略、明确 reason、Director 五状态/七事件、决策快照和 Foveated 理论像素统计。
- Fovea Feedback V1：request/frame/target 三元组严格匹配、stale/timeout/duplicate/out-of-order 拒绝、质量标志存储，以及不改变 Attention/Scheduler 的反馈可视化。
- Feedback-Aware Attention V1：默认关闭回归稳定性、合法 ACCEPTED 目标专属有界修正、TIMEOUT 小幅惩罚、过期/陈旧/错配/重复/无效反馈零修正，以及 base/feedback/final 可解释输出。
- B->C V1 损坏输入拒绝：重复/倒退帧、count 不符、缺 END_FRAME、非法类型/置信度/bbox/ID/帧号。

## 硬件边界

platform_io_vitis.c、a_fovea_adapter.c、register_map.h 和 vitis_axi_smoke_test.c 已与 A commit 729a8d0 中的真实 fovea_axil_ctrl RTL、BD 和 XSA 对齐。A 的 XSA 地址为 0x40000000、范围 4K，但 C 不硬编码该地址；仍需用 XSA 刷新 Vitis BSP，从生成的 xparameters.h 获取真实宏名。当前尚无板卡 UART 结果，不能声称 PS/PL 已经联调通过。

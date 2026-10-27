# EagleEye Soccer Foveated Broadcast Simulation

EagleEye 是一个面向足球赛事观赛的 FPGA 仿生主动视觉系统。系统从全局比赛画面中得到多个候选目标，通过轻量、可解释的 Attention 选择当前最值得关注的足球或球员，结合常速度运动预测生成动态 Fovea ROI，再由后续 FPGA 模块对局部区域进行 2x 增强。

当前版本是纯 C PC 软件模拟，不连接 FPGA、摄像头、HDMI、DMA 或 VDMA。2x 插值只表示对已有 ROI 的 Crop 加 Bilinear/Lanczos 重采样，不声称恢复原图中不存在的真实细节，也不用于裁判判罚。

## 队友阅读顺序

1. include/eagleeye_types.h：共享目标、请求和结果结构体。
2. src/fake_data.c：足球、球员和比赛场景的可重复 fake 输入。
3. src/attention.c：七项 Attention 分量和总分。
4. src/attention_scheduler.c：argmax、Switch Margin、Hold Time 和 Fovea Request。
5. src/motion_predictor.c：常速度预测器 V1。
6. src/transaction.c：request/target/frame 严格匹配、stale reject、timeout 和一次性事件。
7. src/state_machine.c：保留的状态机骨架。
8. tests/test_scenarios.c：旧事务测试和足球场景测试。
9. SOCCER_MIGRATION.md：本次选题迁移说明。
10. SOCCER_HARDWARE_BRINGUP_CHECKLIST.md：足球版真实上板前清单。
11. BALL_CARRIER_FOCUS.md：持球队员估计、传球交接和 Joint ROI 说明。

## 软件流程

Global Target List -> Possession Estimator -> Attention Competition -> Selected Target -> Motion Prediction -> Joint Fovea Request -> Refined Result -> Focus/Risk Update

足球目标类型为 BALL、PLAYER、REFEREE、UNKNOWN；场地区域第一版为 FIELD_NORMAL、ATTACK_ZONE、GOAL_ZONE。

Attention 公式为：

Attention = w_ball*BallPriority + w_motion*Motion + w_goal*GoalZone + w_small*SmallTarget + w_uncertainty*Uncertainty + w_ball_near*BallProximity + w_aging*Aging

Uncertainty = 255 - coarse_conf。权重集中在 include/attention_config.h，没有自动优化。

## 构建和运行

Visual Studio：打开 EagleEye_C_Attention_Sim.sln，选择 Debug、x64，按 F5 调试或按 Ctrl+F5 运行。

在 Visual Studio Developer Command Prompt 中运行 build_msvc.bat 和 run_tests_msvc.bat。

PC 构建只使用 platform_io_sim.c；platform_io_vitis.c 是单独的 Vitis 平台实现，不能和模拟平台同时链接。

## 测试覆盖

- 正常足球关注：BALL 竞争胜出并生成 Fovea Request。
- 快速运动：运动分量提高，预测器给出下一帧 ROI。
- GOAL_ZONE：目标关注度提高并映射为 FOCUS/HIGHLIGHT 语义。
- 短暂遮挡、长时间消失：为预测/保持/timeout 接入预留测试入口。
- CONFIRMED、DOWNGRADED、TIMEOUT。
- STALE RESULT REJECTED，以及 request/target/frame 三种错配。
- Switch Margin/Hold Time 防止接近分数候选频繁切换。
- 持球队员估计、传球交接和球员+足球 Joint ROI（PC Fake Match）。

## 硬件边界

platform_io_vitis.c、register_map.h 和 vitis_axi_smoke_test.c 只提供 Vitis/AXI-Lite 接口准备。真实 fovea_axil_ctrl IP、Base Address、STATUS 行为和板卡串口结果仍需 Vivado/Vitis/真实 Zynq 板卡确认。当前不能声称 PS/PL 已经联调通过。

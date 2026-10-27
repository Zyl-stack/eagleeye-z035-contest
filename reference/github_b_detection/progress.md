# EagleEye C 部分进度

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

## 当前边界

Fake 数据不代表真实 B 模块检测结果；Motion Predictor V1 不代表 Kalman 或深度学习跟踪；2x ROI 重采样不恢复不存在的细节；Vitis 文件不代表真实 AXI-Lite 已上板。

## 下一步

由硬件侧在 Vivado 建立真实 Fovea AXI-Lite IP，完成 Address Editor、Bitstream、XSA、Vitis BSP 和串口 smoke test；之后再由 A/B/C 共同冻结真实字段和联调时序。

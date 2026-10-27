# Soccer Scenario Migration

## 1. 迁移原因

项目从道路危险目标确认调整为足球赛事主动视觉/自动特写。新应用仍然需要全局候选、有限 Fovea 资源分配、局部增强、跟踪和释放，因此现有 C 模块的核心调度链可以继续复用。

## 2. 完全复用的模块

- attention_scheduler.c 的 argmax、Switch Margin、Hold Time、Aging 和 request 生命周期。
- transaction.c 的 request/target/frame 严格匹配、stale result rejection、timeout、release 清理和一次性事件。
- state_machine.c 的状态值和总体状态流。
- platform_io_sim.c、platform_io_vitis.c、register_map.h 和 vitis_axi_smoke_test.c。
- Visual Studio 工程、PC 测试框架和固定宽度通信结构。

## 3. 只修改语义的模块

- eagleeye_types.h 在保留旧字段兼容性的基础上增加足球目标类型、场地区域、球距和关注属性。
- attention.c 将原道路分量解释为足球的 Ball Priority、Goal Zone 和 Ball Proximity 等七项分量。
- risk 的旧枚举和测试接口保留，同时提供 FOCUS/HIGHLIGHT 语义映射，避免破坏已验证接口。
- main.c 的日志改为足球关注和预测 ROI。

## 4. 新增模块

- motion_predictor.h/.c：常速度模型 x_pred=x_now+vx、y_pred=y_now+vy。首次见到目标没有历史时 prediction_valid=0，后续可替换为 Kalman 接口。
- ee_fake_generate_soccer_scenario()：提供可重复的足球测试输入。

## 5. Attention 字段

BallPriority 是足球本身的基础关注优先级；Motion 是速度大小；GoalZone 是目标所在场地区域；SmallTarget 是小目标需要更大特写的程度；Uncertainty 是 255-coarse_conf；BallProximity 是球员与足球的近似距离分量；Aging 是等待时间补偿。

公式保持轻量、可解释，不使用深度学习 Attention，权重集中于 attention_config.h，不自动优化。

## 6. 当前仍是假数据

足球检测、球员/裁判分类、球距、场地区域、遮挡和 refined result 都是软件 fake 或简化规则。没有球员号码 OCR、战术分析、判罚识别或真实视频输入。

## 7. 硬件状态

当前没有真实 fovea_axil_ctrl Vivado IP，也没有真实 Base Address、Bitstream、XSA 或 Zynq 串口结果。AXI-Lite 代码只是接口准备，不能当作硬件测试通过。

## 8. Ball-Carrier Focus

PC 足球 Demo 额外增加 `possession_estimator.c/.h`。它输出 `Estimated Ball Carrier`，依据球员 bbox 与足球的归一化距离、速度连续性和有限历史分数。该分数是估计值，不是严格规则判定。现有 Attention/Scheduler 仍负责最终选择，传球交接仍由 Switch Margin/Hold Time 约束。选中球员时，bridge 生成包含球员和足球的 Joint ROI；这只是足球上层语义，不修改 AXI-Lite 接口。

## 9. 后续分工

B：提供真实全局候选目标和 frame/target 生命周期。C：维护 Attention、Scheduler、预测、事务和系统状态。A：实现/确认 ROI 局部增强、STATUS/QUALITY 行为并回传匹配结果。三方共同冻结真实字段、时序和板卡验证标准。

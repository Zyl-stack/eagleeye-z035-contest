# EagleEye Soccer Interface Specification

当前阶段为纯 C PC 模拟。所有共享结构位于 include/eagleeye_types.h，使用 stdint.h 固定宽度类型、静态数组和无动态内存。

## B -> C GlobalTarget

EE_GlobalTargetList 包含 frame_id、count 和最多 EE_MAX_TARGETS 个 EE_GlobalTargetDescriptor。

EE_GlobalTargetDescriptor 的核心字段：

- target_id: uint16_t，跨帧目标 ID。
- frame_id: uint32_t，目标所在全局帧。
- x/y/w/h: uint16_t，检测框。
- target_type: uint8_t，BALL、PLAYER、REFEREE、UNKNOWN。
- coarse_class: uint8_t，旧接口兼容字段。
- coarse_conf: uint8_t，0..100 粗检测置信度。
- vx/vy: int16_t，像素/帧速度估计。
- motion_level: uint8_t，简化运动等级。
- small_target_score: uint8_t，小目标关注分量；为 0 时由框面积估算。
- uncertainty: uint8_t；为 0 时按 255-coarse_conf 估算。
- field_zone: uint8_t，FIELD_NORMAL、ATTACK_ZONE、GOAL_ZONE。
- ball_distance: uint16_t，和足球的简化距离；第一版为 fake 值。
- is_ball: uint8_t，是否足球。
- valid: uint8_t，目标是否有效。
- attention_score: uint32_t，C 计算的总关注分数。

## C -> A FoveaRequest

EE_FoveaRequest 保持冻结，不改变字段含义：

- request_id: uint32_t，新事务创建时分配。
- target_id: uint16_t，选中目标。
- source_frame_id: uint32_t，请求使用的源帧。
- roi_x/roi_y/roi_w/roi_h: uint16_t，局部 ROI。
- enhance_mode: uint8_t，增强模式。
- deadline_frames: uint16_t，结果截止帧数。
- valid: uint8_t，请求有效标志。

## A -> B/C EnhancedROIResult

EE_EnhancedROIResult 携带 request_id、target_id、source_frame_id、ROI、enhance_mode、result_valid、done、error。三个 ID 必须对应原请求。

## B -> C RefinedTarget

EE_RefinedTargetDescriptor 携带 request_id、target_id、frame_id、valid/result_valid、done/error、target_type、refined_class、refined_confidence、tracking_quality、focus_quality，以及兼容保留的 Risk 字段。

使用 refined result 前必须同时满足：

received.request_id == request.request_id
received.target_id == request.target_id
received.frame_id == request.source_frame_id

任一字段不匹配就打印 STALE RESULT REJECTED，禁止更新 Focus/Risk。

## Attention

Attention = w_ball*BallPriority + w_motion*Motion + w_goal*GoalZone + w_small*SmallTarget + w_uncertainty*Uncertainty + w_ball_near*BallProximity + w_aging*Aging

Uncertainty = 255 - coarse_conf。当前权重集中在 include/attention_config.h，没有自动优化。total_attention 是加权和，不是分量简单相加。

## Frozen AXI-Lite Map

0x00 CONTROL，0x04 STATUS，0x08 REQUEST_ID，0x0C SOURCE_FRAME_ID，0x10 TARGET_ID，0x14 ROI_XY，0x18 ROI_WH，0x1C ENHANCE_MODE，0x20 OUTPUT_BUFFER，0x24 DEADLINE，0x28 CYCLE_COUNT，0x2C QUALITY_FLAGS。

OUTPUT_BUFFER 必须保持 0x20，QUALITY_FLAGS 必须保持 0x2C。

## 平台边界

PC 使用 platform_io_sim.c；Vitis 使用 platform_io_vitis.c。两者不能同时链接。真实 Base Address 必须来自 Vivado 生成的 xparameters.h，当前没有真实 IP 或硬件测试结果。

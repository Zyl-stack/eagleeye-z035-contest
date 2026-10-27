# Ball-Carrier Focus

## 目的

足球很小且运动快，只盯 BALL 会让自动特写缺少球员上下文。本阶段把 `Estimated Ball Carrier` 作为主要候选语义，同时保留 BALL 作为辅助候选。

## Possession Score

第一版是可解释的估计值，不是严格的足球规则判定，也不是 AI 识别：

```text
Possession Score = 0.60 * BallDistance
                 + 0.30 * MotionContinuity
                 + 0.10 * PreviousPossession
```

三个分量均为 `0..255`。BallDistance 使用足球中心到球员 bbox 的最近距离，并按确定的画面尺度归一化；MotionContinuity 根据球员与足球速度差计算；PreviousPossession 只在旧球员仍与球接近时提供有限历史保持，不会永久锁定。

## 交接和 ROI

每帧先由 C `possession_estimator` 给球员写入派生 `possession_score`，再进入现有 Attention 和 Scheduler。Scheduler 仍负责 argmax、Switch Margin 和 Hold Time，因此传球时不会因为单帧距离变小就立即切换。

当 Selected Target 是球员时，bridge 计算球员 bbox 与足球 bbox 的 union，再加 padding 并 clip 到 `640x360`，形成 Joint Player+Ball ROI。Fovea Request 的 `target_id` 仍是球员 ID，通信结构没有改变。

Motion Predictor 的基础常速度算法没有重写。新的 bridge 优先对 Selected Player 预测，并用预测位置更新展示 ROI；足球预测只作为辅助。

## 遮挡边界

BALL 遮挡时球员仍可保持关注，界面会标记 `BALL TEMP LOST`。Player 遮挡时，现有 Scheduler 会依据有效候选继续工作；短时预测/保持是展示语义，不能替代真实检测器和完整跟踪器。所有结果目前仍来自 Fake Detection。

## 当前限制和未来接口

当前不包含 YOLO、OpenCV、Kalman、号码识别、战术分析、判罚、摄像头、DMA/VDMA 或真实视频。未来 B 侧只需提供同样的 `EE_GlobalTargetList` 和 frame/target 生命周期，C 侧即可复用估计、Attention、Scheduler、ROI 和事务链。

# PC 足球赛事主动视觉可视化 Demo

## 这是什么

这是现有 EagleEye 足球版 C Core 的外挂可视化工具。C bridge 直接调用现有 Fake Target、Attention、Scheduler 和 Motion Predictor，输出 JSON Lines；Python Tkinter 只负责绘图、播放和界面显示。

它不是 FPGA 真机结果，也不是摄像头识别结果。右侧 2x Fovea View 只是把图形目标放大显示，不是超分辨率，也不是 FPGA Lanczos 验证。

## 如何运行

最简单的方法是双击工程根目录的 run_soccer_visualizer.bat。它会检查 Python/Tkinter、编译 bridge，然后打开界面。

如果首次运行提示找不到 Visual Studio 编译器，请打开 Visual Studio Developer Command Prompt，进入工程目录后运行 build_visualizer_bridge.bat，再双击 run_soccer_visualizer.bat。

Python 只使用标准库 tkinter、json 和 subprocess，不需要安装 OpenCV、PyQt、pygame 或 matplotlib。

## 界面操作

- Start / Play：按选择的速度连续播放。
- Pause：暂停并保持当前画面。
- Next Frame：暂停后向前走一帧。
- Restart：回到场景第一帧。
- Scenario：选择 Ball Tracking、Fast Ball Motion、Goal Zone、Temporary Occlusion、Switch Margin 或 Mini Match Simulation。
- Speed：选择 0.5x、1x 或 2x。

## 画面含义

- 黄色目标：当前 Selected Target。
- 黄色边框：当前 Fovea ROI。
- 粉色虚线十字：Motion Predictor 产生的 predicted position。
- 右侧 Fovea View：当前目标的图形放大示意。
- FOCUS/HIGHLIGHT：可视化层的显示语义；底层状态机仍使用原有状态值，不修改事务状态机。

Attention 的七个分量和 Total Attention 来自 C bridge 的真实计算结果，Python 不重新计算 Attention。

## 场景

1. Ball Tracking：BALL 被选择，ROI 随足球移动。
2. Fast Ball Motion：速度分量增大，并显示预测位置。
3. Goal Zone：球进入 Goal Zone，Attention 增加，界面显示 HIGHLIGHT。
4. Temporary Occlusion：fake 输入短暂移除 BALL。界面会如实显示当前 C Scheduler 的结果；由于当前冻结 Motion Predictor 接口只接受有效目标描述，缺失描述时不能伪造一个核心预测结果。
5. Switch Margin：两个 PLAYER 分数接近，观察 Hold/Switch Margin 对选择稳定性的影响。
6. Mini Match Simulation：280 帧的确定性小型比赛回放，包含 A 队 #7/#10/#17/#21、B 队 #4/#6/#8/#22 和 BALL #1。Fovea 以估计持球队员为主，并在传球后交接焦点。

### Mini Match 时间线

- 1-50：#7 持球跟踪。
- 51-90：#7 高速推进并保持 Fovea。
- 91-130：#7 向 #10 传球，两个候选的 Possession Score 发生交接。
- 131-180：Scheduler 实际切换到 #10，#10 持球推进。
- 181-210：#10 进入 Goal Zone，显示关注增强语义。
- 211-225：BALL 短暂遮挡，#10 仍可见。
- 226-245：BALL 恢复。
- 246-265：#10 短时无效，界面继续使用核心输出并显示可用的候选变化。
- 266-280：最终进攻。

球员使用确定性的慢速三角波移动，足球使用分段线性轨迹，因此每次 Restart 都得到相同结果。候选目标、Possession Estimator、Attention、Scheduler 选择和 ROI 来自 C bridge 调用的核心模块；Python 不复制算法，也不强行指定 Selected Target。事件文字、球队颜色和轨迹尾巴只属于 UI 展示。

可视化器会检查 `selected` 与 `fovea_view_target_id`，并检查对应目标是否存在。若 ID 不一致，会暂停并报告错误，避免黄色目标、Fovea View、ROI 和 Attention 面板显示不同对象。

Mini Match 右侧信息区还显示 AUTO DIRECTOR、Estimated Ball Carrier、Possession、Switch Margin、Hold Remaining 和 Attention Top 3。Top 3 只是对 bridge 已输出的 C Core 分数排序，Python 不重新计算 Attention 或替代 Scheduler。`scenario_event` 是 Fake 比赛事件，`algorithm_state` 是算法输出标识，两者在 JSON 中分开。

球员持续移动来自 C bridge 每帧生成的 bbox；Python 只把这些坐标映射到 Canvas。PLAYER #10 OCCLUDED 阶段使用 `visible=0` 隐藏其图形，同时保留有效 C focus/request 以展示预测保持，这属于 PC Demo 的显示建模，不等于真实 B 检测器的 invalid 语义。

## 当前 Fake Data 和未来替换

目标、位置、速度、区域、球距、遮挡和 refined 结果仍是 Fake Data。以后只需要让 B 侧产生同样的 EE_GlobalTargetList，再通过同一个 C Core 生成结果；真实 A 侧 Fovea 和硬件状态仍由 Vitis/AXI-Lite 路径负责。

## 已知限制

- 当前 bridge 是离线 40 帧回放，不读取实时视频。
- 右侧 2x 是图形示意，不验证 FPGA 像素处理。
- Mini Match 的遮挡阶段使用现有 Motion Predictor 接口对最后一个有效球描述生成短时展示预测；这不是新增算法，也不代表缺失视频帧已被 FPGA 恢复。`prediction_source` 会标明来源。
- 没有 YOLO、OCR、人脸识别、摄像头、HDMI、DMA、VDMA、Kalman 或真实判罚功能。

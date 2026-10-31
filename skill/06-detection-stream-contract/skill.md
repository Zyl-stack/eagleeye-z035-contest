---
name: 06-detection-stream-contract
description: 审计检测到注意力模块的数据流，确认源坐标系、置信度、速度单位、帧时间与目标 ID 语义，并校验 EE_B_TARGET_STREAM_V1 文件。
---

# 检测流坐标与时间契约

## 适用范围

适用于检测/跟踪结果跨模块交接、视频换尺寸或按 stride 抽帧。附带校验器针对本项目 V1 文本协议；其他协议先按同样方法建契约，不直接套用字段表。

## 工作流

1. 取得流的生成代码、原始输入元数据和实际文件。分别记录 sensor/source 分辨率、模型 imgsz、letterbox/stretch、显示尺寸及 C 内部坐标。不能仅因框未越界就推断源分辨率。
2. 新 V1 流必须声明 `STREAM_INFO,source_width,source_height`。旧流缺失时，先向源图/视频与生成记录追溯；校验工具要求通过 `--legacy-size` 显式输入核实值，不静默使用 demo 默认。
3. 冻结 bbox 左上角/宽高、边界裁剪、类别映射、置信度取值范围、valid、velocity_valid 和 ID 生命周期。V1 TARGET 共 15 个逗号字段，包含 record；FRAME 内记录数严格一致。
4. 宽高独立缩放时，`x,w,vx` 乘目标宽/源宽，`y,h,vy` 乘目标高/源高；统一舍入和裁剪。存在 letterbox/crop 时先逆变换。ID 和置信度不随坐标缩放。
5. 记录 source_fps、frame_stride、每个输出的实际 source_frame_id/timestamp。只有固定步长才能用 `dt=stride/fps`；末帧、跳帧、可变帧率和实时掉帧用真实时间戳。
6. 查 tracker 中速度究竟按源帧还是处理步计算。明确 `px/source-frame`、`px/detection-step` 或 `px/s`，再换算；旧 V1 的 `px/frame` 含义不足时需补说明。不能只凭 source_fps 推定速度单位。
7. 运行独立校验器，再让接收方 replay，至少测试空帧、上限数量、非法置信度、重复/保留 ID、截断帧、丢失/恢复和不同源尺寸。

## 使用方法

```sh
python scripts/check_stream.py targets.txt
python scripts/check_stream.py legacy.txt --legacy-size 640 640
```

校验器检查帧序、计数、字段、范围、越界、同帧重复 ID，以及已观测到的 ID 类别变化。跨遮挡身份及接收方行为还需真值评价与端到端回放。保留窗口之后 ID 可重新分配时，须更换会话或扩充带 generation 的协议。

## 失效条件

缺源尺寸证据、把推理尺寸当坐标系、把源采样率当处理吞吐、把 px/step 当 px/s，或仅能解析而未核对单位时，交接不通过。新生成流不能继续依赖历史默认尺寸。

## 已验证效果与失败来源

[E06](../evidence.md#e06检测流需要显式坐标与时间契约)：通用流校验器已覆盖源尺寸、字段范围和异常输入的主机正反例；源采样频率与实际处理吞吐按各自时间基准报告。

## 可直接使用的提示词

> 核对这份检测流与生成记录，为每个坐标和时间字段写出来源、单位与转换公式。源分辨率必须有输入元数据证据，缺失时明确待确认。校验每帧计数、ID、范围、截断和类别变化，再给出接收端 replay 用例，分别报告源采样频率、实际吞吐和端到端延迟。

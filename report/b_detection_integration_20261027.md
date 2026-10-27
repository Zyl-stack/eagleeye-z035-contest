# 足球检测 B→C 联调与模型边界（2026-10-27）

## 三条路线

1. **比赛场景足球路线**：B 使用足球数据微调的 YOLO11n 做 PLAYER/BALL 检测、跟踪并产生 `EE_B_TARGET_STREAM_V1`；C 的 `bc_adapter` 接收结果，交给注意力/调度，并生成 Fovea 请求。原始训练数据类编号 PLAYER=0、BALL=1；传输协议的类型枚举 BALL=1、PLAYER=2，不能直接混用。参考：`reference/github_b_detection/b_to_c_yolo_handoff.md`、`reference/github_b_detection/b_detection_interface.md`、`reference/github_c_attention/b_c_interface_v1.md`。
2. **PicoDet 人体路线**：`data/model.nb` 和 `src/ps/` 是 PERSON 检测，不能当作足球 PLAYER/BALL 检测器。AX7020 材料中的 QEMU 推理也不证明真实板端 B/C 联调。
3. **足球 YOLO ONNX/ARM 移植路线**：根据 B 的说明，ARM 构建、上板启动和真实图片输出仍待验收。本仓库当前 B/C/Linux 各固定分支快照中未发现足球 `.pt`、`.onnx` 权重或完整 ONNX ARM 移植包；也未证明 ONNX Runtime/ARM 环境可用。不能将文档提及的 `best.pt` 当成仓库已经包含。请根据实际交付位置核对模型、程序、导出契约与 SHA256，并确认模型分发许可。

## 交接数据与 PC 检查

`data/bc/ball_static_b_target_stream.txt` 取自本地 `B_to_C_YOLO11n_Handoff.zip`，SHA256 `5a7b31ff9b7ea0055b989bee17aab657002baf011ca9876dd1a2c3ceee665c3a`。交接说明称该流来自 PC 上的 YOLO 处理同一张足球图形成的 24 帧序列；这是交接记录，不是本次重新跑模型、真实运动视频或板端结果。前 12 帧无目标，后 12 帧 BALL 的 ID 为 1。原流不包含 `STREAM_INFO`，现有 C 解析器会按 V1 兼容规则默认 640×360；原图源尺寸尚无证据，不能宣称这里的坐标转换正确。

原 `reference/github_c_attention/tools/bc_stream_demo.c` 在第 0 帧无目标时因调度器无候选返回失败。`tools/bc_stream_handoff.c` 是仅供 PC 联调的独立副本：接受输入路径，对空目标帧不产生 Fovea 请求并继续读下一帧，非空帧仍复用 C 解析、注意力及调度流程。它没有修改稳定工程或冻结 ABI；连续空帧期间保持的 C 状态、超时策略及真实视频帧同步仍需单独验证，不能当成已部署的生产逻辑。

在 Linux/WSL 安装 GCC 后从仓库根目录运行：

```sh
bash tools/test_bc_stream.sh
```

本次实测：C 原 12 帧样例、交接 24 帧流（12 帧空目标，12 帧 BALL ID=1）和非法输入拒绝均通过；只覆盖 PC 软件解析和演示链，未运行训练权重、摄像头、ARM/ONNX 或 Z035 硬件。B 文档中的训练指标为历史记录，尚未独立复现。

## 仍需确认

- B 的足球 YOLO 最终权重和导出 ONNX 的文件、哈希、模型输入输出及其提供位置；足球 ONNX/ARM 移植包和依赖的版本与许可。
- B 交接流的原始画面宽高；将来实际视频流必须正确填写 `STREAM_INFO`，并验证与 A 的显示/ROI 坐标变换一致。
- 哪个模型及程序实际用于最终演示；A/B/C 和设备身份对应的串口、检测流、ROI 结果及板端测试记录。
- 足球视频真实运动、遮挡、多目标、无目标/丢失恢复及 B→C→A 与摄像头/HDMI 端到端测试。

不要把报告中的足球 PC 指标、PicoDet QEMU 或空帧联调结果称为足球模型的板端 PASS。

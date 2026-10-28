# 可暂停的 ROI 自动/手动复判

## 适用范围

适合已有“检测器给出 ROI + 视频放大窗口”的 Zynq/FPGA 工程。实时播放时自动跟踪；暂停后由 USB 相对鼠标接管 ROI，KEY0 切换放大/边缘模式；可选两路位置舵机跟随最终 ROI。此 Skill 提供控制契约和验收方法，**不是**可以直接接到任意板卡的引脚文件。

## 状态契约

| 状态 | ROI 来源 | KEY0 | 鼠标 | 屏幕提示 |
| --- | --- | --- | --- | --- |
| 播放 | 检测器提交的 ROI | 双线性/Lanczos 切换 | 不接管 | 绿色 `AUTO TRACKING` |
| 暂停，从双线性进入 | 冻结帧，鼠标可手动提交 | 双线性 → Lanczos → Edges | 移动、右键复位、中键居中、滚轮调灵敏度 | 蓝色 `MANUAL INSPECTION` |
| 暂停，从 Lanczos 进入 | 同上 | Lanczos ↔ Edges | 同上 | 蓝色 `MANUAL INSPECTION` |
| 恢复 | 重新交给检测器 | 按恢复的播放模式继续 | 自动退出接管 | 绿色 `AUTO TRACKING` |

KEY1 控制暂停/恢复；模式只能在帧边界切换，不能在 ROI 缩放器还在输出上一帧时复位它。这个失败曾造成整屏黑屏，断电才恢复，因此帧边界握手是本 Skill 的**硬性验收项**。摄像头输入、输出显示与 ROI 应使用一致的帧/坐标契约；320x180 ROI 在 640x360 原图中的左上角范围为 `x=0..320, y=0..180`。

## 快速检查

先导出含 BIT 的 XSA，按本板实际命名修改 `expected_hw.example.json`，执行：

```powershell
python tools/check_hw_contract.py --xsa path/to/design.xsa --config expected_hw.example.json
python -m unittest discover -s tests -v
```

该脚本检查 XSA/HWH 中的模块、外部按键/舵机端口和 AXI-Lite 地址，**不能证明**鼠标事件、暂停后帧保持或舵机方向正确。随后至少做：KEY1 暂停 10 秒且 HDMI 不黑屏；两种起始模式各循环多次 KEY0；鼠标四个动作及拔插恢复；ROI 在四边界保持合法；KEY1 恢复后自动跟踪；断电冷启动和 30 分钟连续演示。

软件必须识别 `/dev/input/event*` 中的相对鼠标，写手动 ROI 命令时保持 `manual` 标志，不能让检测器在暂停状态覆盖鼠标位置。机械眼只接两路 PWM 信号和公共地；位置舵机使用独立稳压电源，禁止从 FPGA I/O 供电。不同 SG90 批次、连续旋转版和机构限位要重新标定，不能把 1.3–1.7 ms 当作通用角度标尺。

## 已验证与失效条件

EagleEye Z035 的摄像头/HDMI、KEY1 暂停、KEY0 模式切换、USB 鼠标手动 ROI、两路舵机和自动/暂停 OSD 已获用户上板确认。这个事实仅覆盖当前板卡/镜像，不保证其他板卡复现。若 XSA 契约不匹配、帧边界切换不受控、USB 鼠标未枚举、DDR 画面黑屏或舵机持续旋转，应停止并回到已知可启动的 BOOT 版本。

参考实现：Eyes 工程的 `video_mode_switch_top.v`、`fovea_axil_ctrl.v`、`roi_servo_pwm.v`，以及 Mouse 应用的 `mouse_roi_controller.cpp`。移植时先对比端口、时钟/复位、地址和 I/O 电平，不应直接照搬 J37 引脚。

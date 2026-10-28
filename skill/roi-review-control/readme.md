# 可暂停的 ROI 自动/手动复判

## 问题来源与提炼

项目曾出现暂停后切换模式导致 HDMI 黑屏的反馈；复盘将暂停状态、帧边界、模式切换和人工接管拆成需要核对的条件，而不是仅凭检测进程仍在运行判断视频正常。相关历史过程见 [AI 协作记录](../../report/ai_collaboration_record.md)的黑屏排查案例；本 Skill 将可检查的静态接口与必须上板观察的行为分开。

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

### 不依赖开发板的最小示例

将本目录完整复制到任意位置，在该目录下用 Python 3.10 或更新版本运行；脚本仅使用标准库。`examples/sample.xsa` 是只包含演示用 `system.hwh` 的合成 ZIP，**不是 Vivado 生成的可配置硬件**：

```sh
python -B tools/check_hw_contract.py --xsa examples/sample.xsa --config examples/pass.json
python -B tools/check_hw_contract.py --xsa examples/sample.xsa --config examples/fail.json
python -B -m unittest discover -s tests -v
```

第一条返回 `{"passed": true, "problems": []}`、退出码 0；第二条故意要求不存在的 `missing_key`，返回 `passed: false` 和对应问题、退出码 1。配置为空、文件无法读取或 XSA 不是 ZIP 时退出码为 2；若命令链会在非零退出码时中止，请分别运行各条命令。

### 检查自己的工程

先导出含 BIT 的 XSA，按本板实际命名修改 `expected_hw.example.json`，执行：

```powershell
python tools/check_hw_contract.py --xsa path/to/design.xsa --config expected_hw.example.json
python -m unittest discover -s tests -v
```

该脚本检查 XSA/HWH 中配置列出的模块、外部端口和寄存器基地址，**不能证明**鼠标事件、暂停后帧保持或舵机方向正确。配置应列出实际需要检查的项目；空配置会被拒绝。脚本要求 ZIP 根目录包含名为 `system.hwh` 的文件；其他导出结构或 HWH 格式需先核对，不能忽略异常。随后至少做：KEY1 暂停 10 秒且 HDMI 不黑屏；两种起始模式各循环多次 KEY0；鼠标四个动作及拔插恢复；ROI 在四边界保持合法；KEY1 恢复后自动跟踪；断电冷启动和 30 分钟连续演示。这里列的是建议验收步骤，不代表这些步骤已有本次原始记录。

软件必须识别 `/dev/input/event*` 中的相对鼠标，写手动 ROI 命令时保持 `manual` 标志，不能让检测器在暂停状态覆盖鼠标位置。机械眼只接两路 PWM 信号和公共地；位置舵机使用独立稳压电源，禁止从 FPGA I/O 供电。不同 SG90 批次、连续旋转版和机构限位要重新标定，不能把 1.3–1.7 ms 当作通用角度标尺。

## 已验证与失效条件

用户曾反馈 EagleEye Z035 的摄像头/HDMI、按键、USB 鼠标 ROI 和舵机功能正常；A 对复现 BIT 的总体功能也反馈正常，但本次逐项应用日志和视频未随包提供，不能据此补写逐项实测结果。现场证据范围见 [上板记录](../../report/board_reproduced_20261028_zh.md)。

当前仓库的主机测试及合成示例只能验证检查器能够识别匹配和缺失项目，不证明帧边界握手或物理行为。若 XSA 契约不匹配、帧边界切换不受控、USB 鼠标未枚举、DDR 画面黑屏或舵机持续旋转，应停止并按现场已验证的恢复方案处理。不同分辨率、板卡、XSA/HWH 结构、引脚和外围设备均须重新配置并独立验证。

参考实现：Eyes 工程的 `video_mode_switch_top.v`、`fovea_axil_ctrl.v`、`roi_servo_pwm.v`，以及 Mouse 应用的 `mouse_roi_controller.cpp`。移植时先对比端口、时钟/复位、地址和 I/O 电平，不应直接照搬 J37 引脚。

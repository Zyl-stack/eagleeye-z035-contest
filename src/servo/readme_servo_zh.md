# Z035 EagleEye SG90 机械眼接入

这是从已验证的 Mouse 视频工程出发的新增功能。**不要在稳定 Mouse 工程里直接运行 Tcl。** 先在 Vivado 中 `File > Project > Save As`，新项目建议命名 `EagleEye_7035_Servo`。保存副本后打开其 `system.bd`，执行：

```tcl
source {D:/EagleEye_FPGA_Competition/Z035_Servo_Stage_20261020/servo_apply_bd.tcl}
servo_roi::apply
```

Tcl 在副本中加入两路 PWM 模块、替换顶层包装文件、添加 XDC，并把输入接到现有视频模块的实际 ROI 坐标；不修改摄像头、VDMA、HDMI、OSD、预处理或 PicoDet 应用。完成后生成 BD Output Products、bitstream，并检查时序/DRC。USB 鼠标暂停移动 ROI 与实时 PicoDet 自动 ROI 均驱动同一组 PWM 输出。

Vivado 2022.2 已完成独立短仿真，检查了中位保持、20 ms 周期、10 us/周期缓动及 ROI 往返。尚未完成新工程 BD Validation、综合、时序及实板舵机验证。

## J37 接线

| J37 针脚 | FPGA 管脚 | 用途 |
| --- | --- | --- |
| 5 | AD13 | 水平 SG90 信号（pan） |
| 6 | AC16 | 俯仰 SG90 信号（tilt） |
| 4 或 2 | GND | 板卡与外部舵机电源的公共地 |

J37-1 是板上 +5 V，J37-3 是 Bank 10 的 VCC_B10，**都不是 GND**。不要从 FPGA 信号脚或 J37-3 给舵机供电。两只舵机使用独立、稳压、足够电流的 5 V 电源，电源地与 J37 GND 相连。J5 需在完全断电时设到 3.3 V 档；首次连接信号前实测 J37-3 对 J37-4 约 3.3 V。J6 不用改变。

PWM 为 20 ms 周期，开机先保持 1.5 ms 中位约 1 秒，随后按当前 ROI 在 1.3–1.7 ms 之间变化，单个周期最多调整 10 us。上板先空载确认中位及转向，再安装眼球机构；实际机械角度与两轴反向要按实物校准。这些脉宽是保守初值，不是 SG90 角度标定值。

软件与 SD 卡自启动应用沿用已成功的 Mouse 版。2026-10-20，Eyes 工程已生成 bitstream 并导出含 bitstream 的 XSA；完整实现 WNS +0.530 ns、TNS 0、WHS +0.033 ns、THS 0，DRC 无 Error/Critical。新 XSA 的 PS 初始化文件与 Mouse 版逐字节一致。

Bootgen 2022.2 使用原 FSBL、U-Boot、DTB 和新 Eyes bitstream 生成 `BOOT.BIN`，SHA256 `84FDD1CD5EEB4DDF48B6776A3373AB06928F375688DEAD16CCC5B0B3D6DAE08B`。Bootgen 显示了与之前相同的两条分区地址重叠 Warning，并成功解析四分区。新 BOOT 已写入 `E:\BOOT.BIN` 并读回校验；旧 Mouse BOOT 保留为 `E:\BOOT_PRE_EYES_20261020.BIN`，完整卡备份在 `D:\EagleEye_FPGA_Competition\Z035_Camera_PicoDet_Preboard\eyes_20261020\sd_backup_before_eyes`。`image.ub` 和 `boot.scr` 未改。FAT 启动分区仅余约 0.56 MB，请勿再向该分区写测试日志。Eyes 舵机实板功能尚未验证。

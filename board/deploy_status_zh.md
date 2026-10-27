# Eyes 暂停状态文字部署状态（2026-10-22）

- Vivado Eyes 工程 XSA 已导出，XSA 内 BIT 与实现 BIT SHA256 一致：`09486C3E7F0410482AAD560D0E4AA22BC6F604FC140518AB61E06EBE203E6397`。
- 新实现 WNS +0.505 ns、WHS +0.051 ns，无实现 Error/Critical；PS 初始化文件与上次 Eyes XSA 字节一致。
- 使用上次已验证的 `zynq_fsbl.elf`、`u-boot.elf`、`system.dtb` 与新 `system.bit` 生成本目录 `BOOT.BIN`，SHA256：`54003CD2490C38D5CAF208791E2995DE7112A8E3B5B826A39B0F1811735E2765`。Bootgen 成功，出现上次同类的两条分区地址重叠 Warning。
- 写卡前，E: 的 `BOOT.BIN` SHA256 为 `9BA9AB2FA69BED4F7B270825F37DB12D74F990C5996E11197B0E959990CB8C03`，与上次 Eyes 部署记录一致。已在本目录保存 `BOOT_PRE_PAUSE_STATUS_20261022.BIN`，哈希一致。
- **SD 写入未完成，卡上新 BOOT 未验证。**尝试在 E: 建立额外回退副本时，Windows 将 FAT32 分区识别为 RAW，`chkdsk E:` 只读检查报告 RAW；系统日志有磁盘 1 的 I/O 重试（事件 153）及设备错误（事件 51）。随即停止所有写入。不能推定 E: 当前 `BOOT.BIN` 的内容或完整性。
- 上次完整卡内容备份仍在 `D:\EagleEye_FPGA_Competition\Z035_Camera_PicoDet_Preboard\eyes_mode_switch_20261021\sd_backup_before_mode_switch`，其中有 `BOOT.BIN`、`image.ub`、`boot.scr` 和应用包。

下一步：安全弹出 SD 卡，换读卡器或 USB 口重新插入；先只读检查是否恢复 FAT32。若仍为 RAW，不格式化、不运行写入修复，优先换卡并从 D: 备份恢复。确认介质稳定后才复制本目录新 `BOOT.BIN` 并读回 SHA256 校验。

## 新卡恢复（后续）

- 用户插入新 E: 卡，检查为 MBR、约 32 GB 单 FAT32 分区、Healthy、非 Dirty；原卡仍保留，不再写入。
- 为先隔离卡故障，复制的是上次已验证的 Eyes `BOOT.BIN`（SHA256 `84FDD1CD5EEB4DDF48B6776A3373AB06928F375688DEAD16CCC5B0B3D6DAE08B`），**不是本目录的新暂停状态 BOOT**。
- 同时恢复 `image.ub`、`boot.scr`、`linux_camera_smooth_trial_v7_20261013.tar.gz` 和 `z035_picodet_ps_app.tar.gz`；五个文件均从新卡读回并与 D: 来源 SHA256 一致。
- 写后 E: 仍为 Healthy FAT32、非 Dirty；最近 3 分钟 Windows System 日志未见 disk 7/11/15/51/153/157 错误。尚未上板验证。新卡只有 FAT32 分区，旧卡的 ext4 第二分区未恢复；当前 `image.ub` 含内核、DT、RAMDisk，可先验证启动，但不能保证原 ext4 上的用户数据或服务齐全。
- 上板第一轮只测试旧版 Eyes：串口启动、HDMI、PicoDet、鼠标 ROI 和机械眼。确认新卡稳定后，再备份新卡 BOOT，替换为本目录的新暂停提示 BOOT 并重新上板测试。

## 新卡更新为暂停提示版（后续）

- 用户确认新卡上的旧 Eyes 版本已在板上启动，基本功能正常；旧版不含 2026-10-21 的 KEY0 新模式切换逻辑。
- 再插入电脑时 E: 为 FAT32，但 Dirty/Health Warning。只读 `chkdsk E:` 未发现文件错误；卡上五个文件均与 D: 备份哈希一致，最近 20 分钟无 disk I/O 错误。随后运行 `chkdsk E: /f`，恢复 Healthy、非 Dirty，旧 BOOT 哈希未变。
- 先将本目录新版 `BOOT.BIN` 复制到卡内临时文件并读回核对 SHA256，再将卡上旧 `BOOT.BIN` 同卷改名为 `BOOT_PRE_PAUSE_STATUS_20261022.BIN`，临时文件改名为正式 `BOOT.BIN`。
- 写后 `E:\BOOT.BIN` SHA256 为 `54003CD2490C38D5CAF208791E2995DE7112A8E3B5B826A39B0F1811735E2765`，卡内回退 BOOT SHA256 为 `84FDD1CD5EEB4DDF48B6776A3373AB06928F375688DEAD16CCC5B0B3D6DAE08B`。`image.ub`、`boot.scr` 和应用包未修改。
- 最终 E: 仍为 Healthy FAT32、非 Dirty；最近 5 分钟无 Windows disk 7/11/15/51/153/157 事件。**写卡时新 BOOT 尚未实板验证**。计划检查正常启动、运动状态 KEY0 双线性/Lanczos 切换、暂停后 KEY0 的模式循环，以及左下角绿色 AUTO TRACKING / 蓝色 MANUAL INSPECTION。
- 用户随后确认新版上板运行正确，回应了上述启动、KEY0 模式切换和自动/暂停状态提示的检查要求。这是用户现场验证反馈；尚无独立串口日志或长期稳定性测试记录。

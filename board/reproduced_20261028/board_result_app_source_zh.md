# 队友复现 BIT 实板结果记录（2026-10-28）

- 队友复现的 `z100_top.bit` SHA256：`b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15`；文件头标识 Vivado 2022.2、`z100_top`、器件 `7z035ffg900`。
- 本机用原稳定版 FSBL、U-Boot、system.dtb 和上述新 BIT 通过 Bootgen 2022.2 生成 `BOOT.BIN`，SHA256：`816ebd03dc533f857399938e533994c1989f8af4437ce5b361329da6d461b039`。先用原 BIT 重建控制 BOOT，结果与原稳定 BOOT 逐字节相同。
- 实验 SD 卡同时使用原 `image.ub`、`boot.scr` 及已验证注意力版应用归档，哈希见 `../../../../Z035_REPRO_BIT_BOARD_TRIAL_20261028/SHA256SUMS_SD.txt`（此相对路径仅指向本机准备目录；提交包内原文件见 `../sd_boot/`、`../attention/`）。四个文件写入后均已从实验卡读回校验，E: 当时为 Healthy、NOT Dirty。
- 用户在 2026-10-28 将实验卡上板后反馈：“功能一切正常”。这是本次新 BIT 的**现场功能通过反馈**，可区别于旧 BIT 的上板结果。
- 已保存用户提供的原始串口片段 `BOARD_SERIAL_BOOT_20261028_RAW.txt`，SHA256 `6b25da2bf152b74528fa3f8bd5cba20e3972152f09211444f1ee1185fd8144b1`。它从 U-Boot 开始，包含读取 `boot.scr`、校验 FIT、Linux 内核启动与 PetaLinux `login:` 提示；**不含 FSBL 阶段，也不含登录后的应用 `ATTENTION`、`PREPROC`、`TIME` 输出**。串口中的 U-Boot 构建日期与 Linux 无 RTC 的 2018 时间不能当作本次现场日期。
- 已另从实验 SD 卡读回并保存登录后的应用日志 `ATTENTION_APP_20261028.log`，SHA256 `9bfd5245ad2952fd429a61092e10841dd77bfcc7041eb31720df9891dd69a5e0`，151456 字节。卡上同次 `BOOT.BIN` 和注意力应用归档哈希分别为 `816ebd03dc533f857399938e533994c1989f8af4437ce5b361329da6d461b039`、`b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59`。日志含 179 条 `ATTENTION`、179 条 `PREPROC backend=fpga`、179 条 `TIME`，可作为该实验组合的自动检测、注意力 ROI 与 FPGA 预处理运行证据；其中未见 `ERROR` 或 `PREPROC_FALLBACK`。日志是运行中的文件快照，非完整停机闭合会话。
- 尚未随本包保存逐项功能测试表、鼠标/按键/舵机操作日志、照片/视频、同一输入的板端输出与 Golden 对比或长期稳定性数据；“功能一切正常”仍是用户现场反馈。此记录不声称 I/O/CDC 时序签核完成，也不包含新 BIT 的同次实现报告。若比赛报告要列具体功能或性能数字，应引用相应原始日志及统计方法。

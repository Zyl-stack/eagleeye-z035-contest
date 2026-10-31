# 复现 BIT 的上板记录（2026-10-28）

## 版本和资料来源

现场提供的 `eagleeye_z035_repro_with_board_log_20261028.zip` 的 SHA256 为 `a16ef3e1e1e14cb349ab24ef3bf425734e8b371763a7edb58a13aeeecc731921`。ZIP CRC 检查通过；交接包内 SHA256 清单的 **279 项**均核验通过。原归档保留在本地，本仓库只补充其中与本次上板直接有关的文件。

| 文件 | SHA256 | 核验范围 |
|---|---|---|
| [z100_top.bit](../board/reproduced_20261028/z100_top.bit) | `b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15` | 与本地 2026-10-27 独立复现所生成 BIT 的 SHA256 一致。 |
| [eagleeye_z035.xsa](../board/reproduced_20261028/eagleeye_z035.xsa) | `18667ead62c25cde514e871cba67e927c5ffd32af80d3a0d5e0565a8a56a7e42` | 本地同次独立构建产物，XSA 内 BIT 与上述 BIT 一致；不是板端读回。 |
| [system.hwh](../board/reproduced_20261028/system.hwh) | `fd6938d2dae6bbcd363df5d176892c38e714fbc9734d0be80c4eaeb1086ae9fc` | 本地同次独立构建产物，与 XSA 内 `system.hwh` 一致；不是板端读回。 |
| [boot.bin](../board/reproduced_20261028/boot.bin) | `816ebd03dc533f857399938e533994c1989f8af4437ce5b361329da6d461b039` | 现场结果说明记载使用此 BOOT；原始 SD 读回哈希清单未随本包提供。 |
| [原始串口记录](../board/reproduced_20261028/board_serial_boot_raw.txt) | `6b25da2bf152b74528fa3f8bd5cba20e3972152f09211444f1ee1185fd8144b1` | 记录从 U-Boot 到 Linux 登录提示，不包含 FSBL 或登录后应用输出。 |
| [现场原始结果说明](../board/reproduced_20261028/board_result_source_zh.md) | `7c7c1ff84ab9db6309ceb4eb6f42891610f0867a88955308edc7a21fd391170b` | 原文保留；其中指向现场电脑 `SHA256SUMS_SD.txt` 的相对路径不在本仓库内。 |

**后续验证：**登录后的 Attention 应用日志和六项功能确认见 [应用证据](board_app_evidence_20261028_zh.md)，固定输入预处理结果见[独立板端 Golden](preproc_board_golden_20261029_zh.md)。原启动串口与登录后应用记录分别归档。

## 已核验的启动记录和版本关系

- 用 Vivado 2022.2 的 Bootgen 只读解析 BOOT，发现 `zynq_fsbl.elf`、`system.bit`、`u-boot.elf` 和 `system.dtb` 四个分区。经字节序转换，`system.bit` 分区从配置同步字起至末尾的内容与交接 BIT 的相应部分一致；该项完成 BOOT 与 BIT 的文件配置数据核验，硬件配置读回另作验证。
- 串口直接记录了 U-Boot 2022.01 查找并执行 `boot.scr`、FIT 内核/ramdisk/DT 的 SHA256 哈希检查、Linux 5.15.36 启动，最终出现 PetaLinux `login:`。日志出现环境区无效并采用默认环境的提示；该提示对运行场景的影响待专项确认。
- FIT 日志记录 SHA256 完整性检查，来源签名认证属于单独机制。日志不含 FSBL 阶段，也没有登录后的 `ATTENTION`、`PREPROC`、`TIME` 等应用输出。
- 现场已确认使用本次复现 BIT 上板后，摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 均正常。本项依据现场功能确认；量化性能由后续专项测试记录。

## 验证范围与后续检查

本页覆盖复现 BIT 文件关联、U-Boot 至 Linux 登录及现场功能确认。实验卡 BOOT 读回清单和运行应用、模型的独立读回哈希待核对。后续 [应用日志](board_app_evidence_20261028_zh.md)、[CPU/FPGA 配置计时](preproc_ab_20261028_zh.md)及[独立固定输入 Golden](preproc_board_golden_20261029_zh.md)按各自条件补充验证，独立 Golden 的 BIT 关联仍待核对。同算法性能、经帧新鲜度确认的检测更新率及长期稳定性为后续测试项。旧 `board/attach_100.log` 和独立 V2/R3 实验分别归档，外部 I/O 与 CDC 的专项状态见[告警说明](warning_review_zh.md)。

本次日期按现场回传记录标注；U-Boot 构建时间和 Linux RTC 保留为各自时间字段。

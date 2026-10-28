# 复现 BIT 的上板记录（2026-10-28）

## 版本和资料来源

A 提供的 `eagleeye_z035_repro_with_board_log_20261028.zip` 的 SHA256 为 `a16ef3e1e1e14cb349ab24ef3bf425734e8b371763a7edb58a13aeeecc731921`。ZIP CRC 检查通过；交接包内 SHA256 清单的 **279 项**均核验通过。原归档保留在本地，本仓库只补充其中与本次上板直接有关的文件。

| 文件 | SHA256 | 核验范围 |
|---|---|---|
| [z100_top.bit](../board/reproduced_20261028/z100_top.bit) | `b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15` | 与本地 2026-10-27 独立复现所生成 BIT 的 SHA256 一致。 |
| [eagleeye_z035.xsa](../board/reproduced_20261028/eagleeye_z035.xsa) | `18667ead62c25cde514e871cba67e927c5ffd32af80d3a0d5e0565a8a56a7e42` | 本地同次独立构建产物，XSA 内 BIT 与上述 BIT 一致；不是板端读回。 |
| [system.hwh](../board/reproduced_20261028/system.hwh) | `fd6938d2dae6bbcd363df5d176892c38e714fbc9734d0be80c4eaeb1086ae9fc` | 本地同次独立构建产物，与 XSA 内 `system.hwh` 一致；不是板端读回。 |
| [boot.bin](../board/reproduced_20261028/boot.bin) | `816ebd03dc533f857399938e533994c1989f8af4437ce5b361329da6d461b039` | A 的结果说明记载使用此 BOOT；原始 SD 读回哈希清单未随本包提供。 |
| [原始串口记录](../board/reproduced_20261028/board_serial_boot_raw.txt) | `6b25da2bf152b74528fa3f8bd5cba20e3972152f09211444f1ee1185fd8144b1` | 记录从 U-Boot 到 Linux 登录提示，不包含 FSBL 或登录后应用输出。 |
| [A 的原始结果说明](../board/reproduced_20261028/board_result_source_zh.md) | `7c7c1ff84ab9db6309ceb4eb6f42891610f0867a88955308edc7a21fd391170b` | 原文保留；其中指向 A 本机 `SHA256SUMS_SD.txt` 的相对路径不在本仓库内。 |

**后续补充：**A 已回传登录后的 Attention 应用日志与逐项功能正常反馈；范围、VDMA 警告和仍未完成的输出对比见 [应用证据](board_app_evidence_20261028_zh.md)。下文对“不含应用输出”的描述仅指原启动串口文件。

## 已核验的启动记录和版本关系

- 用 Vivado 2022.2 的 Bootgen 只读解析 BOOT，发现 `zynq_fsbl.elf`、`system.bit`、`u-boot.elf` 和 `system.dtb` 四个分区。经字节序转换，`system.bit` 分区从配置同步字起至末尾的内容与交接 BIT 的相应部分一致；该校验是文件内容核验，并非硬件配置读回。
- 串口直接记录了 U-Boot 2022.01 查找并执行 `boot.scr`、FIT 内核/ramdisk/DT 的 SHA256 哈希检查、Linux 5.15.36 启动，最终出现 PetaLinux `login:`。日志出现环境区无效并采用默认环境的提示；单凭日志不能推断其对所有运行场景的影响。
- FIT 日志中的哈希检查是完整性检查，不能据此宣称签名认证通过。日志不含 FSBL 阶段，也没有登录后的 `ATTENTION`、`PREPROC`、`TIME` 等应用输出。
- A/用户反馈：使用本次复现 BIT 上板后“功能一切正常”。这是现场功能反馈；包内没有逐功能测试表或应用视频/串口，不能扩写为逐项自动化通过或有量化性能证据。

## 尚未证明

当前记录不能独立核对实验卡的 BOOT 读回清单、板上正在运行的应用及模型哈希；本次新增应用日志的 `TIME` 统计见应用证据；仍没有同一输入板端输出与 Golden 的逐值对比、同条件 CPU/FPGA 性能数据、经帧新鲜度验证的检测更新 FPS 或长期稳定性记录。旧 `board/attach_100.log` 及独立 V2/R3 数据属于历史实验，不能代替本次证据。复现实现报告里的外部 I/O 约束和 CDC 等未闭环问题也不能因现场反馈正常而视为时序签核完成。

用户提供的日期为本次记录日期；启动日志中的 U-Boot 构建时间和 Linux RTC 时间不应当被当成本次现场测试时间。

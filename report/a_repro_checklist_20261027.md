# 发给队友的复现清单

**2026-10-28 更新：**以下为 10 月 27 日的历史材料；A 此后已确认摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能上板正常，Attention 应用日志也已收录。无需再为这些功能重复索要资料；最新范围见 [上板应用证据](board_app_evidence_20261028_zh.md)。

目前同一提交目录区分两版：原稳定 GridPolicy 见 `src/ps_as_built/` 和 `board/sd_boot/`；后续实板通过反馈的注意力 ROI 版见 `src/ps/` 和 `board/attention/`。两版共用原稳定 BOOT、image.ub、boot.scr 和 model.nb。`src/attention_reference/` 仅是 C 算法参考子集，未作为板端程序直接运行。

## 可以立即复现

1. 先校验根目录 `SHA256SUMS.txt`，再按 `build/REPRODUCE_ZH.md` 使用 Vivado 2022.2 在短 ASCII 路径重建工程。干净重建已完成 BD Validation 和 Output Products；原已验证 BIT 与 BOOT 输入 BIT 哈希一致。
2. 使用匹配的 PetaLinux 2022.2 ARMv7 工具链及 `build/deps/` 的 Paddle-Lite 头文件/库，分别编译 `src/ps_as_built/` 和 `src/ps/`。两版对应 ARM 二进制 SHA256 分别是 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea` 和 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`。
3. 在 `board/attention/` 的归档内运行 `sha256sum -c SHA256SUMS`，核验模型、源码和程序；独立实验卡使用原三项启动文件配该归档。原稳定卡不修改。

## 还需要补的证据/条件

1. **本次注意力版实板原始证据：** 请保存从自启动到运行的完整串口日志，至少含 `ATTENTION`、`PREPROC backend=fpga`、`TIME`，以及 ROI 切换场景的短视频或逐项记录。现在只有用户口头反馈“验证通过”，不能把旧 Grid 的 evidence/logs 冒充新算法证据。
2. **板级独立重建：** 现有脚本只验证到干净工程 BD/输出文件；仍需在复现机上综合、实现、生成 BIT/XSA，核对时序和 I/O 约束，再用新 BIT 实板验证摄像头、HDMI、鼠标 ROI、按键、舵机。现有稳定 BIT/BOOT 可以复用作功能基线，但不能替代“从提交源码重新生成新 BIT”的证据。
3. **外部工具链：** 小包包含 Paddle-Lite 构建依赖，不包含完整 PetaLinux 2022.2 工具链/sysroot；队友需使用匹配环境并记录版本。Vivado 2022.2 的第三方 IP 许可/安装也需在其机器确认。
4. **注意力参考范围：** `src/attention_reference/` 只复现两文件静态库，不等于检测模型，也不等于完整 `c-attention` PC 仿真。若报告要写完整注意力工程，应单独固定 Git 提交、补全其构建脚本所需源文件并实际运行；若只写板端注意力逻辑，以 `src/ps/` 为准。

因此，队友现在可以根据本包重建板级工程入口、编译两版 PS 应用并撰写源码级报告；完整“净源→新 BIT→新应用→全功能实板”复现证据还需要上述第 1、2 项。

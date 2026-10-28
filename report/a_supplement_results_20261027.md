# A 的 2026-10-27 复现材料补充

**后续更新（2026-10-28）：**本文保留当时记录；完整构建及 A 回传的匹配 BIT、启动串口见 [复现 BIT 的上板记录](board_reproduced_20261028_zh.md)。文末有关 BIT 哈希和启动串口未收到的说法只适用于 10 月 27 日，不适用于后续结果。

A 交接归档 SHA256：`ebdc78bc05cbae70bba3b78baf2ca23668cc0b243e4bcb060135704b7015ade4`。原本地归档保留；仓库保留原始 `original/eagleeye_z035.zip` 不变，新增 `original/a_repro_delta_20261027.zip`，包含 A 相对于该快照的 54 个新增或变更文件。增量通过 Git LFS 获取，保留原始文件名及大小写。仓库展示副本包含 `src/ps_as_built/` 和修订 Tcl，但应通过下面的恢复命令获得实际构建用的大小写目录。

## 恢复和核验

```sh
git lfs pull
python3 tools/restore_a_repro.py
python3 tools/restore_a_repro.py --output /tmp/eagleeye_a_repro_new
```

输出目录必须不存在。恢复程序先核验 A 清单中全部 275 文件；生成目录为 `/tmp/eagleeye_a_repro_new/eagleeye_z035`。其 `build/REPRODUCE_ZH.md` 含实际构建说明；`build/deps/paddle_lite_armv7_build_dep.tar.gz` 在恢复目录内，不能将小写展示副本直接当作构建目录。PetaLinux 2022.2 工具链和完整 sysroot 未随增量包提供；模型和 Paddle-Lite 依赖对外分发前需确认授权。

## 实际核验与界限

- A 包 CRC 和 275 项 SHA256 核验通过。A Tcl 在 Vivado 2022.2 独立目录通过 BD 校验和目标生成；未对 A 新脚本生成的工程重复实现。日志中仍有复位连接等警告。
- 使用 A 提供的 Paddle-Lite 构建依赖和本机 PetaLinux 2022.2 工具链，重编稳定 GridPolicy 应用；`paddle_person_live` SHA256 为 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，与 A 所述稳定包二进制一致。编译告警指出静态链接中的 dlopen 需要匹配的 glibc 动态库；本次未在板上执行。
- 先前独立重建同一仓库 Eyes 源码并对旧路径脚本做局部修复，综合、实现及 BIT 成功。布线后 WNS +0.608 ns、WHS +0.007 ns；LUT 24986、FF 28246、BRAM 303、DSP 49。报告在 `build/independent_rebuild_20261027/`；12 个输入和 33 个输出延迟未约束，Methodology 有 8 项 Critical Warning，CDC 有 Critical 分类。已约束时序通过不代表 I/O 或 CDC 签核完成。
- `src/ps_as_built/` 是稳定 GridPolicy；`src/ps/` 是后续 AttentionPolicy。A 的交接材料记载有用户上板通过反馈，但完整串口及视频证据未随包提供；历史日志不能冒充新注意力版本的实测。
- `report/submission_audit.md` 和 `report/reproduction_zh.md` 已更新独立构建状态。独立软件/内核构建不证明与现场镜像完全相同。

仍需真实资料：板级摄像头及 HDMI 设备时序约束依据；CDC/时钟告警的处理；AttentionPolicy 当前板端完整日志与功能记录；用户反馈已上板成功的 BIT 哈希及原始记录；最终检测程序/模型和 B 模块的实际集成关系。未声称完整板端功能或加速性能 PASS。

# 竞赛提交材料核查与缺口（2026-10-27）

本页保存比赛 3.3.5 材料的阶段核查表。当前提交导航见[比赛材料与证据索引](contest_evidence_index_zh.md)，以下各项按对应工程和测试日期引用。

| 项目 | 已有 | 尚需完成 |
|---|---|---|
| 工程与代码 | Z035 Eyes RTL、BD、XDC、IP、PicoDet 应用、注意力候选和 Linux 固定源码；原快照与补充材料均可哈希核验 | 团队主要分工及 [实测版本](release_identity_zh.md)已记录；仍需确认最终演示应用选择及具体版本对应关系，并核对 [第三方许可](third_party_sources_zh.md)；若足球检测列为实际参赛功能，需补齐其最终模型及板端链路，PERSON 与 PLAYER/BALL 按独立模型路线管理 |
| 仿真 | 预处理五类帧及适配器、AXI-Lite、舵机 RTL；注意力模块主机与 QEMU | Lanczos 历史独立工程有通过日志，旧逐像素日志亦有 768 个错误；版本与测试条件需分别说明，见[记录范围](../sim/lanczos_legacy_readme.md) |
| 硬件实现 | 工程快照使用完整构建入口完成综合、实现、BIT 和 XSA，报告见 `build/full_rebuild_20261027/` | 12 项输入和 33 项输出延迟缺失；8 项 Methodology Critical Warning 和 CDC Critical 分类尚需核查。已约束 STA、板端功能和专项时序签核分别记录 |
| 可复现构建 | 工程快照恢复、Vivado 全流程入口、Paddle-Lite 依赖、固定 Linux 源码及配置 | Vivado 和 PetaLinux 2022.2 工具链/sysroot 需自行准备；重建内核与历史镜像哈希不同；rootfs 与现场镜像一致性未证明 |
| 上板证据 | 现场已确认 PicoDet 注意力候选功能正常；现场提供 `b286f480...` 复现 BIT、配套实验 BOOT、U-Boot 至 Linux 登录的原始串口和现场六项功能正常的确认，另有[独立板端 Golden 对比](preproc_board_golden_20261029_zh.md) | Golden 一次固定输入逐位通过，但该次独立 BIT 哈希未核实；A/B 计时两组缩放算法和实时帧不同，日志还提示 VDMA 错误位和帧新鲜度未验证 |
| Skill 与报告 | 技能包、PYNQ 模拟候选、报告材料稿和 AI 协作记录 | 主要分工已确认；仍需核对来源和授权；Lanczos 独立仿真日志与当前工程的版本对应尚待核实 |

现场人员或资料所有者需要补充的内容见 `report/field_evidence_needed_zh.md`。现场功能确认与 QEMU、仿真和工具报告应分别标注，各自保留证据范围。Git LFS 对象须 `git lfs pull`，并在对外分发前确认模型、依赖及第三方 IP 授权。

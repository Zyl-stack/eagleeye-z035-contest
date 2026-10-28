# 竞赛提交材料核查与缺口（2026-10-27）

依据已提供的比赛 3.3.5 截图检查；不代替组委会对完整规则的最终审核。

| 项目 | 已有 | 尚需完成 |
|---|---|---|
| 工程与代码 | Z035 Eyes RTL、BD、XDC、IP、PicoDet 应用、C 候选和 Linux 固定源码；原快照与 A 增量均可哈希核验 | 由团队确认最终演示版本及 A/B/C 对应关系；若足球检测列为实际参赛功能，需补齐其最终模型及板端链路，不能把 PERSON 模型冒充 PLAYER/BALL |
| 仿真 | 预处理五类帧及适配器、AXI-Lite、舵机 RTL；C 主机与 QEMU | Lanczos 旧逐像素日志有 768 个错误，需确认测试版本并解决或如实保留失败 |
| 硬件实现 | A 快照使用完整构建入口完成综合、实现、BIT 和 XSA，报告见 `build/full_rebuild_20261027/` | 12 项输入和 33 项输出延迟缺失；8 项 Methodology Critical Warning 和 CDC Critical 分类尚需核查。已约束时序通过不等于板端安全或功能通过 |
| 可复现构建 | A 快照恢复、Vivado 全流程入口、Paddle-Lite 依赖、固定 Linux 源码及配置 | Vivado 和 PetaLinux 2022.2 工具链/sysroot 需自行准备；重建内核与历史镜像哈希不同；rootfs 与现场镜像一致性未证明 |
| 上板证据 | A 已确认 PicoDet+C 候选功能正常；A 提供 `b286f480...` 复现 BIT、配套实验 BOOT、U-Boot 至 Linux 登录的原始串口和现场六项功能正常的确认，详见 `report/board_reproduced_20261028_zh.md` | 已补 Attention 应用日志，A 已确认六项功能正常；当前版本 FPGA/CPU 同条件对照及板端预处理输出对 Golden 逐值比较仍缺；日志还提示 VDMA 错误位和帧新鲜度未验证 |
| Skill 与报告 | 两项已有技能、PYNQ 模拟候选、报告材料稿和 AI 协作记录 | 需团队核对贡献、创新对比、授权、原始交互材料并形成正式定稿；Lanczos 旧测试失败且版本对应待核实，不得宣称当前逐像素验证 PASS |

A/B 分工下真正需要现场人员或资料所有者补充的内容见 `report/field_evidence_needed_zh.md`。A 的现场功能确认与 QEMU、仿真和工具报告应分别标注，各自保留证据范围。Git LFS 对象须 `git lfs pull`，并在对外分发前确认模型、依赖及第三方 IP 授权。

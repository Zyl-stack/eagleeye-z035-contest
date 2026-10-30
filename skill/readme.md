# EagleEye 可复用技能包

这些是从 Z035 EagleEye 已完成工作中提炼的**方法、校验脚本与参考源码**，不是另一份可直接烧录的 BOOT 镜像。每项有自己的适用范围、用法、已验证效果和停止条件。

| Skill | 主要产物 | 已验证边界 |
| --- | --- | --- |
| [`preproc-fp32-golden`](preproc-fp32-golden/readme.md) | FP32 NCHW 全张量比较器、五类帧与 DMA/cache 验收流程 | 五类帧 RTL 仿真通过；另一次[独立板端固定输入](../report/preproc_board_golden_20261029_zh.md)逐位一致，BIT 哈希未核实 |
| [`roi-review-control`](roi-review-control/readme.md) | 自动/暂停/鼠标复判状态契约、XSA 端口和地址校验器 | 主机合同测试通过；A 已确认摄像头、显示及 ROI 等六项功能上板正常 |
| [`pynq-overlay-dma`](pynq-overlay-dma/readme.md) | Overlay 文件身份核验、寄存器白名单、简单 DMA 与缓存示例 | 仅模拟对象离线验证；尚无 PYNQ 实板或其他真实工程验证 |
| [B 交接的 11 项 Skill](b_team_20261029/readme.md) | 视频、张量、部署、检测流、ROI 和可复现发布等工作流与工具 | 各项按原交接记录区分主机、仿真、历史资料和未验证内容；当前 Z035 实板结果以对应证据为准 |

使用前先复制到自己的项目，替换示例器件、视频尺寸、时钟、地址和引脚；不要把 Eclipse/Vivado 缓存或板卡专属 BOOT 当作可移植文件。独立 V2/R3 实验曾在开发板上完成 conv43/44 Golden（单次加 20 次重复共 21/21）和固定输入推理测试，见[历史实板证据](../report/board_evidence_20261027.md)。这不能证明 V2/R3 已集成到当前 Eyes 系统或完整视频检测实现 FPGA 卸载；卷积加速器不属于上述 Skill；当前 Eyes 系统不是 PYNQ Overlay。新增 PYNQ 示例是独立候选，不代表当前系统已使用 PYNQ。

上表前三项提供 `examples/` 和离线测试、标准库 Python 命令及问题来源、验证和失效边界；示例数据是人为构造的主机测试，不是板端测量。复现方式和验证记录见各自说明及 [Skill 验证记录](../sim/skill_examples_20261028.txt)。B 的 11 项以其独立索引和测试记录为准。

为保留原始工程快照的可追溯性，更新前的 Skill 说明及 ROI 检查器原件另存于 `original/skill_snapshot/`，完整原始归档仍为 `original/eagleeye_z035.zip`。现行可使用版本在本目录中。

## 脱离工程复用和 PYNQ 边界

两项 Skill 可分别复制到空目录，使用自身的 `examples/` 和 Python 标准库运行；详见各目录 README。独立 HWH 和另一种张量形状的合成示例用于检查不同接口命名、地址及数据形状，不是来自其他真实项目。脱离工程的实际运行记录见 [本次复用测试](../sim/skill_portability_20261028.txt)。

已有 PYNQ Overlay、寄存器、简单 DMA 与缓存的可复用示例，详见新 Skill；只在 PC 模拟对象验证；团队目前没有 PYNQ 实板或真实外部工程的验证资料，不要求队友重复提供不存在的材料。不能宣称硬件操作已通过或已获得比赛加分。

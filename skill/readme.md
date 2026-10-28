# EagleEye 可复用技能包

这些是从 Z035 EagleEye 已完成工作中提炼的**方法、校验脚本与参考源码**，不是另一份可直接烧录的 BOOT 镜像。每项有自己的适用范围、用法、已验证效果和停止条件。

| Skill | 主要产物 | 已验证边界 |
| --- | --- | --- |
| [`preproc-fp32-golden`](preproc-fp32-golden/readme.md) | FP32 NCHW 全张量比较器、五类帧与 DMA/cache 验收流程 | 五类帧 RTL 仿真通过；本次版本板端张量对照尚缺 |
| [`roi-review-control`](roi-review-control/readme.md) | 自动/暂停/鼠标复判状态契约、XSA 端口和地址校验器 | 主机合同测试通过；现场总体反馈功能正常，缺本次逐项原始记录 |

使用前先复制到自己的项目，替换示例器件、视频尺寸、时钟、地址和引脚；不要把 Eclipse/Vivado 缓存或板卡专属 BOOT 当作可移植文件。独立 V2/R3 实验曾在开发板上完成 conv43/44 Golden（单次加 20 次重复共 21/21）和固定输入推理测试，见[历史实板证据](../report/board_evidence_20261027.md)。这不能证明 V2/R3 已集成到当前 Eyes 系统或完整视频检测实现 FPGA 卸载；卷积加速器也不属于上述两个 Skill。当前系统不是 PYNQ Overlay，故不标作 PYNQ Skill。

两项 Skill 均提供 `examples/` 正反例、标准库 Python 命令及问题来源、验证和失效边界；示例数据是人为构造的主机测试，不是板端测量。复现方式和验证记录见各自说明及 [Skill 验证记录](../sim/skill_examples_20261028.txt)。

为保留原始工程快照的可追溯性，更新前的 Skill 说明及 ROI 检查器原件另存于 `original/skill_snapshot/`，完整原始归档仍为 `original/eagleeye_z035.zip`。现行可使用版本在本目录中。

## 脱离工程复用和 PYNQ 边界

两项 Skill 可分别复制到空目录，使用自身的 `examples/` 和 Python 标准库运行；详见各目录 README。独立 HWH 和另一种张量形状的合成示例用于检查不同接口命名、地址及数据形状，不是来自其他真实项目。脱离工程的实际运行记录见 [本次复用测试](../sim/skill_portability_20261028.txt)。

工具可以辅助检查 PYNQ 项目的 HWH 元数据和比较导出的 FP32 结果，尚未实现或验证通用 Overlay 加载、寄存器访问、DMA 搬运及缓存一致性处理；也未在 PYNQ 板卡或真实外部工程验证，因此目前不能宣称具备通用 PYNQ Skill 的实板加分资格。

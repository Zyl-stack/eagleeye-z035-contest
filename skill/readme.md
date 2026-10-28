# EagleEye 可复用技能包

这些是从 Z035 EagleEye 已完成工作中提炼的**方法、校验脚本与参考源码**，不是另一份可直接烧录的 BOOT 镜像。每项有自己的适用范围、用法、已验证效果和停止条件。

| Skill | 主要产物 | 已验证边界 |
| --- | --- | --- |
| [`preproc-fp32-golden`](preproc-fp32-golden/readme.md) | 通用 FP32 NCHW 全张量比较器、五类帧与 DMA/cache 验收流程 | 独立 IP 的五类完整帧 Golden；项目中 PL 预处理已上板 |
| [`roi-review-control`](roi-review-control/readme.md) | 自动/暂停/鼠标复判状态契约、XSA 端口和地址校验器 | 当前 Eyes 工程的鼠标、按键、舵机和状态提示已由用户上板确认 |

使用前先复制到自己的项目，替换示例器件、视频尺寸、时钟、地址和引脚；不要把 Eclipse/Vivado 缓存或板卡专属 BOOT 当作可移植文件。独立 V2/R3 实验曾在开发板上完成 conv43/44 Golden（单次加 20 次重复共 21/21）和固定输入推理测试，见[历史实板证据](../report/board_evidence_20261027.md)。这不能证明 V2/R3 已集成到当前 Eyes 系统或完整视频检测实现 FPGA 卸载；卷积加速器也不属于上述两个 Skill。当前系统不是 PYNQ Overlay，故不标作 PYNQ Skill。

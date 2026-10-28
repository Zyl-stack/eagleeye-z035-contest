# 可复用技能索引

原工程快照保持原样；以下链接使用仓库中实际的小写文件名。若原文中的路径依赖大小写，请先解压 `original/eagleeye_z035.zip` 还原原始目录。

| 项目 | 用途与使用方法 | 已验证范围和失效条件 |
|---|---|---|
| [FP32 预处理 Golden](preproc-fp32-golden/readme.md) | 对同一帧的参考与实际张量进行逐值/容差比较 | `sim/host_preproc.log` 为比较器主机测试；模型格式变化、DMA 未完成或缓存未同步时不能当作有效板端结果 |
| [ROI 复判控制](roi-review-control/readme.md) | 检查视频暂停、鼠标与 ROI 控制合同 | `sim/host_roi.log` 为合同检查器测试；不证明具体板卡外设行为 |
| [PYNQ Overlay 与 DMA 示例](pynq-overlay-dma/readme.md) | 核验身份、受控访问寄存器、DMA/cache 顺序 | 模拟对象测试；尚未验证真实 PYNQ 板卡、DMA 状态和故障恢复 |

这些方法不是已验证的通用 PYNQ Overlay，也不表示 V2/V3 卷积加速器已集成到此版本。证据缺口见 `report/submission_audit.md`。

脱离原工程的合成示例和退出码记录见 [复用测试](../sim/skill_portability_20261028.txt)。独立 HWH 检查支持其他模块名与地址；PYNQ 和真实外部工程尚未验证。

# 可复用技能索引

原技能材料保持原样；以下链接使用仓库中实际的小写文件名。若原文中的路径依赖大小写，请先解压 `original/eagleeye_z035.zip` 还原原始目录。

| 项目 | 用途与使用方法 | 已验证范围和失效条件 |
|---|---|---|
| [FP32 预处理 Golden](preproc-fp32-golden/readme.md) | 对同一帧的参考与实际张量进行逐值/容差比较 | `sim/host_preproc.log` 为比较器主机测试；模型格式变化、DMA 未完成或缓存未同步时不能当作有效板端结果 |
| [ROI 复判控制](roi-review-control/readme.md) | 检查视频暂停、鼠标与 ROI 控制合同 | `sim/host_roi.log` 为合同检查器测试；不证明具体板卡外设行为 |
| [Lanczos 2× 数据流](lanczos-2x-stream/readme.md) | 参考 RTL 和来源核验 | `sim/host_lanczos_source.log` 仅为哈希比对；旧逐像素日志 FAIL，不适合作为通过证据 |

这些方法不是已验证的通用 PYNQ Overlay，也不表示 V2/V3 卷积加速器已集成到此版本。证据缺口见 `report/submission_audit.md`。

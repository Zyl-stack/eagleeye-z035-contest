# EagleEye 可复用技能包

这些是从 Z035 EagleEye 已完成工作中提炼的**方法、校验脚本与参考源码**，不是另一份可直接烧录的 BOOT 镜像。每项有自己的适用范围、用法、已验证效果和停止条件。

| Skill | 主要产物 | 已验证边界 |
| --- | --- | --- |
| [`preproc-fp32-golden`](preproc-fp32-golden/readme.md) | 通用 FP32 NCHW 全张量比较器、五类帧与 DMA/cache 验收流程 | 独立 IP 的五类完整帧 Golden；项目中 PL 预处理已上板 |
| [`roi-review-control`](roi-review-control/readme.md) | 自动/暂停/鼠标复判状态契约、XSA 端口和地址校验器 | 当前 Eyes 工程的鼠标、按键、舵机和状态提示已由用户上板确认 |
| [`lanczos-2x-stream`](lanczos-2x-stream/readme.md) | 当前 Eyes 工程的七个 RGB 2x Lanczos RTL 文件、来源哈希校验 | 当前系统的 Lanczos ROI 画面已上板；独立逐像素 testbench 尚未以本版通过 |

使用前先复制到自己的项目，替换示例器件、视频尺寸、时钟、地址和引脚；不要把 Eclipse/Vivado 缓存或板卡专属 BOOT 当作可移植文件。卷积加速器的真实 PL 推理仍未验证，不列入上述 Skill。当前系统不是 PYNQ Overlay，故不标作 PYNQ Skill。

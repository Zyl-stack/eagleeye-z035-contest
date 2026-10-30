# ROI 配对消融实验记录

- 实验 ID / 日期 / 负责人：
- 问题与预先定义的通过标准：
- 输入场景与采集设备：
- 原始图像、manifest、模型、代码 commit 的 SHA-256：
- 数据来源与适用许可；未知处填“待确认”：
- 按场次划分的方法及泄漏检查结果：
- source ROI / applied ROI / clamp / source frame ID：
- 输出尺寸 / RGB 顺序 / dtype / 归一化 / 隐藏 resize 检查：
- 各插值的坐标、边界、精度/舍入规格：
- 模型是否固定；训练方式和 seed：
- UNKNOWN 阈值、标定集及固定时间：
- 子集预先定义、样本量与场次数：

| 指标定义 | Nearest | Bilinear | Lanczos |
|---|---:|---:|---:|
| Closed-set accuracy（全部样本 top1 正确数/N） | | | |
| Operational accuracy（接受且正确数/N） | | | |
| Coverage（接受数/N） | | | |
| Accepted accuracy（接受且正确数/接受数） | | | |
| Macro-F1（写明 UNKNOWN 计入方式） | | | |
| 困难子集结果 / N | | | |
| 预处理时延 / 推理时延 / 总时延 | | | |

- 成对统计的正确判据：
- 都正确 / 仅 A 正确 / 仅 B 正确 / 都错误：
- 区间估计方法及视频相关性处理：
- 原始预测表、混淆矩阵、命令与日志位置：
- CPU/RTL/板端证据分别有哪些：
- 观察到的失败、结论适用范围与待测项：

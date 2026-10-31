# Lanczos 旧仿真记录（非独立 Skill）

本目录保留旧 XSim 的原始 `lanczos_legacy_xsim.txt` 和旧 RTL 哈希清单 `lanczos_legacy_rtl_sha256.txt`，文件字节与原始快照内对应文件一致。全部其他旧 Skill 文件仍完整保存在 `original/eagleeye_z035.zip`，未删除或改写原始快照。



当前工程仍使用 Lanczos，原有 RTL 保存在 `src/vivado/`。`python tools/check_lanczos.py` 可核对七份工程 RTL 与原始快照中的来源，用于来源一致性核验，像素计算按下列仿真记录评估。

## 历史独立工程的通过记录

新增的两份 XSim 日志来自历史独立测试工程：

- [单通道测试](lanczos_single_xsim_pass_legacy.txt)：16×12 输入、32×24 输出，768 个输出像素，像素及控制错误为 0；日志记录边界钳位和 AXI 停顿检查通过。
- [RGB 测试](lanczos_rgb_xsim_pass_legacy.txt)：同样的输入和输出尺寸，两帧共 1,536 个输出像素，像素及控制错误为 0；日志记录 RGB 锁步、边界钳位和 AXI 停顿检查通过。

两份日志保留原始字节，覆盖历史独立测试工程。对应完整源码和环境未随材料归档，因此本页引用其日志结果。来件记录部分 RTL 与当前 Eyes 的哈希不同，两套工程的完整版本关系仍待核对，当前 Eyes 的同版本逐像素验证按专项测试处理。

原有 [XSim 失败日志](lanczos_legacy_xsim.txt)仍然保留：其中 1,536 个输出像素有 768 处错误。该记录与新增 PASS 日志的测试版本及参考条件未建立完整对应，各份日志按独立测试版本保存。

团队另提供 [Lanczos 技术说明](../report/lanczos_fpga_brief_20261030.docx)，介绍固定 2× 相位分流及对称折叠，并记录 DSP 用量节省约 67% 的团队验证结果；本次材料未附优化前后综合报告和资源数量，暂无法独立重算该比例。


# Lanczos 旧仿真记录（非独立 Skill）

本目录保留旧 XSim 的原始 `lanczos_legacy_xsim.txt` 和旧 RTL 哈希清单 `lanczos_legacy_rtl_sha256.txt`，文件字节与原始快照内对应文件一致。全部其他旧 Skill 文件仍完整保存在 `original/eagleeye_z035.zip`，未删除或改写原始快照。



当前工程仍使用 Lanczos，原有 RTL 保存在 `src/vivado/`。`python tools/check_lanczos.py` 可核对七份工程 RTL 与原始快照中的来源，但它不验证像素计算。

## 历史独立工程的通过记录

新增的两份 XSim 日志来自历史独立测试工程：

- [单通道测试](lanczos_single_xsim_pass_legacy.txt)：16×12 输入、32×24 输出，768 个输出像素，像素及控制错误为 0；日志记录边界钳位和 AXI 停顿检查通过。
- [RGB 测试](lanczos_rgb_xsim_pass_legacy.txt)：同样的输入和输出尺寸，两帧共 1,536 个输出像素，像素及控制错误为 0；日志记录 RGB 锁步、边界钳位和 AXI 停顿检查通过。

两份文件保留来件原始字节；测试工程源码及完整环境未随本次材料提供，因此这里引用的是日志记录，未重新运行仿真。来件说明这些测试工程与当前 Eyes 中部分 RTL 的哈希不同，但本次缺少其完整源码，无法独立核对两套工程的全部版本关系；**不能将这两份 PASS 直接归为当前 Eyes RTL 的逐像素测试结果**。

原有 [XSim 失败日志](lanczos_legacy_xsim.txt)仍然保留：其中 1,536 个输出像素有 768 处错误。该记录与新增 PASS 日志的测试版本及参考条件未建立完整对应，不能把不同日志当成同一次测试。

团队另提供 [Lanczos 技术说明](../report/lanczos_fpga_brief_20261030.docx)，介绍固定 2× 相位分流及对称折叠，并记录 DSP 用量节省约 67% 的团队验证结果；本次材料未附优化前后综合报告和资源数量，暂无法独立重算该比例。


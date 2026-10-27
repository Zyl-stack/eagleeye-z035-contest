# Lanczos 旧仿真记录（非独立 Skill）

本目录保留旧 XSim 的原始 `lanczos_legacy_xsim.txt` 和旧 RTL 哈希清单 `lanczos_legacy_rtl_sha256.txt`，文件字节与原始快照内对应文件一致。全部其他旧 Skill 文件仍完整保存在 `original/eagleeye_z035.zip`，未删除或改写原始快照。

旧日志显示两帧 16×12 输入放大到 32×24，1536 个 RGB 输出像素中报告 768 处错误。旧测试台与当前 Eyes RTL 的参考算法及版本对应关系尚未独立确认，因此不能用该旧结果断言当前上板 RTL 有错，也不能据此声称当前 RTL 的逐像素 Golden 验证通过。已上板的功能反馈与逐像素数值验证是不同证据。

当前工程仍使用 Lanczos，原有 RTL 保存在 `src/vivado/`。`python tools/check_lanczos.py` 可核对七份工程 RTL 与原始快照中的来源，但它不验证像素计算。

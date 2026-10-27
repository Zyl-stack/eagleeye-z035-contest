# AXI4-Stream Lanczos-3 RGB 2x 放大

## 适用范围

把一帧 RGB888 视频流按宽高各 2 倍放大。`rtl/lanczos_scaler_rgb_top.v` 对 R/G/B 三通道保持锁步；单通道顶层用六行环形存储提供 6x6 窗口，并按边界 clamp。最适合在已存在 ROI 裁剪/帧缓冲的管线中把 320x180 放大到 640x360。它不是任意倍率的超分辨率神经网络，也不会凭空恢复细节。

## 输入、输出与算法

- 输入：24 位 RGB888 AXI4-Stream，R=`TDATA[23:16]`，G=`[15:8]`，B=`[7:0]`；输入帧首像素 `TUSER=1`，逐行末尾 `TLAST=1`。
- 输出：宽高各 2 倍，颜色通道和 AXI4-Stream 帧/行标记保持一致；`TREADY` 背压必须被上游遵守。
- 半像素 1-D Q10 系数为 `[25, -139, 626, 626, -139, 25]`；对角位置使用对称分组压缩系数。最终 RTL 为定点/饱和像素实现，比较应基于此固定契约，而非未经量化的浮点插值。
- 输入帧必须完整、宽高参数必须与实际流一致；切换放大模式只能在输出帧结束且 scaler 准备接受下一帧时进行，不能中途复位正在输出的流。

## 使用

目录中的 `rtl/` 是从已上板 EagleEye Eyes 工程提取的当前实现。先校验源码没有混入早期试验版本：

```powershell
python tools/check_provenance.py
python tools/check_provenance.py --source-dir 'D:\EagleEye_FPGA_Competition\Vivado_EagleEye_7035_Eyes\EagleEye_7035_Eyes\EagleEye_7035_Eyes.srcs\sources_1\imports\rtl'
```

上面的脚本只验证版本来源，不验证像素数学。旧 `lanczos_RGB` 目录中的 testbench 是为旧 RTL 编写的，与当前 Eyes RTL 的半像素参考计算不兼容，**不能把其 FAIL 或 PASS 作为本版本结论**。要移植到不同分辨率，应为本实现重新建立独立 Golden/testbench，再跑目标尺寸完整帧仿真、综合/实现和板上摄像头/HDMI 检查。

工作区可能留有旧 testbench 生成的 `build/` 诊断文件；它们不属于本 Skill，也不应打包提交。

集成时先决定 ROI/整幅视频的缓冲所有权和输出时序。若上游不能响应 `TREADY`，必须加足够缓冲；若模式切换会重置本模块，必须等旧输出帧结束。不要直接套用早期 `hardware/test3_broken_*` 或未经验证的 1280x960 时钟参数。

## 已验证与失效条件

本版本来自已成功显示 Lanczos ROI、KEY0 模式切换和暂停复判的 EagleEye Z035 Eyes 工程，且包内七个 RTL 文件与该工程源文件 SHA256 一致。板端观感是当前系统级证据；**本 Skill 尚未提供通过的独立逐像素 Golden 仿真**。工程最终显示还依赖摄像头、VDMA、视频时钟、模式选择和 HDMI。发生 RGB 错位、帧标记错位、背压丢像素、边界异常、模式切换黑屏或时序失败时，不应上板替换稳定 BOOT。

复用时把本目录当作可审查参考实现，按目标 FPGA 资源与时钟重新约束/实现；不要把旧项目目录里的综合缓存和 DCP 当作源码。

# PicoDet FP32 预处理 Golden 验证

## 问题来源与提炼

在摄像头、四路 DMA 和 FPGA 预处理对接时，仅看到 IP 完成或画面正常，并不能确认模型实际收到的张量正确：输入帧可能被覆盖，通道顺序、缩放或缓存状态也可能不一致。因此将“固定同一输入、比较完整参考张量、检查错误和退出码”整理成可复用流程；项目背景与局限见 [AI 协作记录](../../report/ai_collaboration_record.md)的预处理案例。

## 适用范围

验证 RGB888 摄像头帧经 FPGA 预处理后生成的 FP32 NCHW 张量。默认示例契约为 640x360 RGB888 输入、`1x3x320x320` 输出；换模型或输入尺寸时，应先修改参考程序和 DMA 长度，再使用本 Skill。它是验证流程，不包含队友 IP 的所有权或许可证。

## 输入与结果

- 输入：同一帧原始 RGB、CPU 参考输出 `.f32`、从三个 S2MM DMA 缓冲区按 R/G/B 顺序拼成的 `.f32`。
- 输出：元素数、逐位相等数、最大/平均绝对误差、超出容差数；非有限值、长度不符或误差越界返回非零状态。
- 四路 DMA 示例：MM2S 输入 `691200` 字节；R/G/B 各 S2MM `409600` 字节。三个输出平面地址依次相隔 `409600` 字节；完成顺序无要求。

## 不依赖开发板的最小示例

将本目录完整复制到任意位置，在该目录下用 Python 3.10 或更新版本运行；脚本仅使用标准库。`examples/` 中是人工构造的 2×2 小端 FP32 测试数据，**不是真实摄像头或板端输出**：

```sh
python -B tools/verify_tensor.py --actual examples/actual_pass.f32 --golden examples/golden.f32 --shape 2x2 --require-exact
python -B tools/verify_tensor.py --actual examples/actual_fail.f32 --golden examples/golden.f32 --shape 2x2 --require-exact
python -B -m unittest discover -s tests -v
```

第一条输出 `passed: true`、`values: 4`、`exact: 4`，退出码 0；第二条故意改动一个值，输出 `passed: false`、`exact: 3`、`over_tolerance: 1`，退出码 1。文件缺失、长度不对或含非有限值时退出码为 2。负例应当失败；若命令链会在非零退出码时中止，请分别运行各条命令。

## 非 EagleEye 形状的复用示例

以下是合成的 2×3 小端 FP32 数据，展示比较工具不依赖摄像头或具体模型；不是板端数据：

```sh
python -B tools/verify_tensor.py --actual examples/other_pass.f32 --golden examples/other_golden.f32 --shape 2x3 --require-exact
python -B tools/verify_tensor.py --actual examples/other_fail.f32 --golden examples/other_golden.f32 --shape 2x3 --require-exact
```

正例退出码 0；第二个例子改动了一个值，退出码 1。其他项目先确认输入来源一致、元素类型、布局、字节序和预先确定的容差；本工具不执行格式转换。用于 PYNQ 项目时，仅比较导出的数据，不负责 Overlay、缓存同步或 DMA。PYNQ 板端适配尚未验证。

## 用法

```powershell
python tools/verify_tensor.py --actual board_tensor.f32 --golden captured.golden.f32 --shape 3x320x320 --require-exact
python -m unittest discover -s tests -v
```

先用零帧、全值帧、不同通道帧、边缘帧和真实采集帧运行完整 RTL 仿真，再在板上对**同一帧**跑 CPU 与 FPGA。只要 FPGA 输出与 CPU 双线性参考使用同一坐标、归一化和舍入规则，便可要求逐位相等；换工具链或算法契约时先定义容差，不能事后放宽阈值掩盖错误。

板端顺序：确认 HWH 地址/时钟与驱动一致；锁定输入帧槽；CPU 写过输入时先 clean cache；启动一条 MM2S 和三条 S2MM；等待三条输出、检查 DMA/IP 状态与超时；invalidate 输出 cache；导出张量比对。不得把摄像头仍在写入的槽当作固定输入。

测速要分别记录 IP `cycles`、DMA/DDR/cache 的端到端 `preprocess_ms`、纯 PS 参考 `preprocess_ms` 和检测结果。**IP 流接口仿真时延不是板端实测。**已收到当前版本的 [CPU/FPGA A/B 原始日志](../../report/preproc_ab_20261028_zh.md)：两组使用相同模型与线程，但 CPU 为双三次、FPGA 为双线性且实时帧不同；只能报告两种配置的计时比，不能称为同算法加速比或数值一致。

## 已验证效果与边界

仓库内 [Icarus 仿真记录](../../sim/preproc_iverilog_20261027.log)显示五类帧各 307200 个值与参考逐位一致；采集帧的三路仿真输出和独立复核方法见 [仿真数据说明](../../sim/preproc_captured_dma_20261027/readme.md)。这证明指定测试台和输入下的离线结果，不证明实板 DMA 与缓存行为。

A 已确认复现 BIT 上板功能正常，登录后应用输出已收到；后续[独立板端固定输入测试](../../report/preproc_board_golden_20261029_zh.md)中 307,200 个输出与 Golden 逐位一致，BIT 哈希未核实。本 Skill 不自动锁帧、操作 DMA 或维护缓存；板端采集必须由匹配系统的驱动和流程完成。

## 失效条件

- 原 EagleEye 板端流程的输入、输出形状或 R/G/B 顺序改变时，必须重新定义预处理参考及传输契约；通用文件比较器仍可用于其他明确契约的小端 FP32 张量。
- DMA 尚未全部完成、帧槽被覆盖、cache 未维护、出现 AXI 错误/超时。
- 仅拿 IP cycle 或一次 FPS 作为正确性证明；应停止并补完整张量比对。
- 比较器只接受小端 FP32 原始文件；其他数据类型、字节序或张量布局应先按已确认的契约转换。它只判断两个文件是否符合给定容差，不能证明参考本身正确、输入帧一致或 DMA 已完成。

参考来源：仓库 [预处理源码](../../src/ip_repo/preproc/)、[仿真与验证说明](../../sim/verification_zh.md)及其中引用的历史交付记录。移植时应以自己的工程输入、张量和板端记录为准。

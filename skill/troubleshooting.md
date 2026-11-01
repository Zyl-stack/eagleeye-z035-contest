# FPGA 视频与模型部署踩坑清单

本清单汇总 EagleEye 开发中的问题与排查方法，适用于视频流、预处理、嵌入式推理及工程复现。迁移时先确认目标板卡、工具版本、接口和测试输入。经验来源与证据见[经验记录](evidence.md)；历史实验、软件模拟与当前板端结果分别标注。

| 现象与触发条件 | 排查步骤 | 依据与验证范围 |
|---|---|---|
| **Vivado 启动即退出**：程序尚未进入 Tcl 就报错或退出。 | 先检查 Vivado 版本、环境变量、依赖和启动日志；区分工具本身无法启动与工程构建失败。工具可启动后再检查 BD 和 IP。 | [干净构建案例](../report/ai_collaboration_record.md)记录曾出现 `0xC0000135`，并检查了启动环境；成因以现场日志为准。 |
| **BD 校验或生成失败**：工程复制后缺 IP 配置，或生成路径指向旧目录。 | 恢复保留大小写的完整快照；检查 BD 关联的 XCI、IP Repository 和路径，再运行 BD Validation 与 Output Products。 | [构建记录](../build/a_reproduce_zh.md)记录了遗漏 XCI 与相对路径问题；[复现指南](../report/reproduction_zh.md)提供恢复及构建入口。 |
| **像素错位、丢样或颜色混乱**：AXI 流发生反压或多通道不同步。 | 检查 `valid && ready` 接收事件、反压期间数据稳定、SOF/EOL、RGB 通道对齐和坐标位宽；用独立 Golden 验证边缘、连续帧和反压。 | [AXI 视频流回归](01-axi-video-regression/skill.md)记录位宽与通道同步经验；实际尺寸和时钟须重新测试。 |
| **预处理输出与参考不一致**：张量布局、通道顺序、插值或缓存状态不同。 | 固定同一输入，核对 RGB/BGR、NCHW、尺寸、缩放和归一化契约；确认 DMA 已完成，再对完整 FP32 张量逐值比较，排查长度、非有限值和误差。 | [预处理 Golden Skill](preproc-fp32-golden/readme.md)含正反例；[板端固定输入测试](../report/preproc_board_golden_20261029_zh.md)记录 307,200 值逐位一致，独立 BIT 身份待核对。 |
| **帧撕裂、重复或内容过旧**：采集端和消费者同时访问环形帧缓冲。 | 核对帧槽所有权、frame-done、stride、保留内存和 cache；使用单调帧序号、所有权协议或经测量的安全时间界限，检查复制期间是否发生回绕覆盖。 | [VDMA 帧快照](05-vdma-frame-snapshot/skill.md)说明条件；[应用告警](../report/warning_review_zh.md)记录 VDMA 错误位和 `freshness=unverified`，原因及帧新鲜度仍待验证。 |
| **暂停切换模式后黑屏，或鼠标无反应**：模式切换与帧状态冲突，或鼠标事件未进入应用。 | 按显示状态、帧边界、模式状态机排查黑屏；鼠标先检查 Linux 输入设备枚举和 USB 绑定，再检查事件与 ROI 接管。对照板卡的 XSA/HWH 和接口配置。 | [ROI 控制 Skill](roi-review-control/readme.md)和[协作案例](../report/ai_collaboration_record.md)记录现场反馈与排查；真实交互按目标板现场测试。 |
| **ARM 程序无法启动或加载库**：交叉编译 ABI、运行库或 sysroot 与目标不匹配。 | 检查 ELF 架构、hard-float、动态依赖及 GLIBC 版本；使用匹配的 ARMv7 工具链、sysroot 和推理库，先在 QEMU 检查，再由目标板验证。 | [ARMv7 部署](03-armv7-inference-deploy/skill.md)记录历史 `GLIBC_2.27` 依赖问题；ELF 检查不等于板端运行。 |
| **XSA、BIT 或系统工具链版本不匹配**：混用不同硬件导出或工具代际。 | 记录器件、工具版本和文件哈希；比对 XSA 内嵌与外部 BIT，核对设备树及工具链，再构建。保留原始退出码、日志和版本信息。 | [PetaLinux/XSA 重建](04-petalinux-xsa-rebuild/skill.md)及[复现指南](../report/reproduction_zh.md)；文件相同不单独证明板端配置正确。 |
| **WNS 为正但仍有时序问题**：外部接口缺约束，或存在 CDC、时钟告警。 | 查看 unconstrained paths、I/O 延迟、时钟树、CDC、DRC 和 Methodology 报告；按板级时序资料核对，不用猜测约束或宽泛例外掩盖。 | [工程告警](../report/warning_review_zh.md)保留缺约束、CDC 和方法学状态；正 WNS 仅覆盖已约束路径。 |
| **局部 FPGA 运算正确但整网更慢**：搬运、数据转换和调度成本超过节省的计算时间。 | 在相同输入、模型和线程下分别测完整 Predictor、打包、等待、回传和解码；按实际净耗时判断卸载价值，避免重复累加包含关系的计时。 | [历史 V2/R3 独立实测](../report/board_evidence_20261027.md)记录 Golden 通过但整网变慢；它不是当前 Eyes 视频系统的卷积加速结果。 |
| **普通 PC 环境可运行，板端 BusyBox 工具选项缺失**：同名命令的编译选项不同。 | 先检查现场实际工具及参数；在不扩展权限的前提下测试可用的受限读取方式，并记录缺失命令和失败原因。不要把主机夹具结果当作板端通过。 | [AI 协作案例 7](../report/ai_collaboration_record.md)记录 `head -c` 不支持后改用 `dd` 并增加负向夹具。 |
| **PYNQ DMA 等待不返回或结果错误**：长度、TLAST、缓存或设备契约不匹配。 | 先核对 BIT/HWH、IP 名称、传输方向、长度、TLAST 和缓存维护；状态不明时停止新任务，按目标平台的隔离与恢复方案处理。 | [PYNQ 示例](pynq-overlay-dma/readme.md)列出诊断方法；当前只完成 PC 模拟，示例不提供可保证的硬超时。 |

## 如何复用

1. 先记录目标板、工具版本、输入和触发条件，再从最早的错误或异常状态定位。
2. 保留命令、退出码、版本和原始日志；按各 Skill 的失效条件区分主机、仿真和板端结果。
3. 修改后使用相同输入验证相关行为；涉及 DMA、时序或物理外设时，依据目标系统的操作与恢复流程实施。

更多适用范围、示例和校验工具见 [Skill 目录](readme.md)。

# 嵌赛项目可复用技能包

> 本文件保留早期整理时的项目说明；当前仓库已将全部技能合并，完整入口见 [技能包目录](readme.md)。仓库内展开文件使用小写 `skill.md`，如需安装到要求 `SKILL.md` 的工具，请在独立副本中恢复文件名。

本包从“嵌赛”项目的 10 个可访问历史聊天（93 个回合）及当前聊天提炼，结合本地源码、测试、训练报告和交接材料编写。整理日期：2026-10-29。覆盖清单及证据见 [EVIDENCE.md](evidence.md)。无法读取的原始附件、未保存的串口记录不作为已验证事实。

技能包保留的是可迁移的工作方法、提示词、检查步骤和校验工具。另一支队伍可替换器件、模型、分辨率、地址和接口表后使用；本项目参数仅作案例。模型、数据集、Paddle-Lite 库和完整工程不随本包重复分发。

## 技能索引

每个目录内的 `SKILL.md` 都包含适用范围、执行方法、失效条件、效果和经验来源。下列编号仅用于阅读排序。

| 目录 | 能解决的问题 | 可以直接复用的内容 | 已有依据与边界 |
|---|---|---|---|
| [01-axi-video-regression](01-axi-video-regression/skill.md) | AXI 视频缩放错行、RGB 错位、反压后丢像素 | 握手不变量、独立 Golden、连续帧回归提示词 | 单通道及 RGB 小帧、目标行宽测试；不等于全系统上板 |
| [02-tensor-preprocess-golden](02-tensor-preprocess-golden/skill.md) | FPGA 预处理与模型输入数值不一致 | FP32 比较脚本、半像素/布局/归一化契约 | 五类完整帧历史比对；DDR 和检测变化未测 |
| [03-armv7-inference-deploy](03-armv7-inference-deploy/skill.md) | 模型可在 PC 跑，ARM 板上加载失败 | ELF 检查脚本、分阶段移植与 QEMU 验证流程 | Cortex-A9 QEMU 已跑通；速度须板测 |
| [04-petalinux-xsa-rebuild](04-petalinux-xsa-rebuild/skill.md) | XSA、bitstream、系统镜像版本混用 | XSA/外部 bitstream 校验脚本、环境预检清单 | 硬件匹配与环境阻塞有记录；不声称镜像已构建 |
| [05-vdma-frame-snapshot](05-vdma-frame-snapshot/skill.md) | 推理期间摄像头覆盖 DDR，出现撕裂或旧帧 | 缓冲所有权、时间界限、cache 排查与时延拆分 | 实现及离线运行有记录；真实 DMA 所有权须实测 |
| [06-detection-stream-contract](06-detection-stream-contract/skill.md) | B→C 坐标、速度、帧率和 ID 语义不一致 | 独立流校验器、旧流显式尺寸覆盖、时间基准模板 | STREAM_INFO 与实际视频 metrics 可核验 |
| [07-small-object-tracking-regression](07-small-object-tracking-regression/skill.md) | 小球漏检、目标预算挤掉球、遮挡后 ID 变化 | 两类数据审计、场景回归矩阵、分块检测评价 | 训练/真实视频 PC 测试；非板上实时指标 |
| [08-roi-ablation-evaluation](08-roi-ablation-evaluation/skill.md) | 无法证明插值方式对识别的真实影响 | 配对实验模板、UNKNOWN 指标、负结果写法 | 400 个配对 ROI；Lanczos 并未全面优于双线性 |
| [09-roi-transaction-integration](09-roi-transaction-integration/skill.md) | 分类返回到错误目标、旧结果覆盖新请求 | 三 ID 匹配、语义适配、故障注入与交接模板 | 12 帧软件闭环及过期结果拒绝报告 |
| [10-hls-cross-platform-port](10-hls-cross-platform-port/skill.md) | 将上游 FPGA 后端误当可直接部署的 Zynq 工程 | 仓库职责图、寄存器/内存迁移表、器件支持预检 | Z100 在 set_part 阻塞；后续报告不虚填 |
| [11-reproducible-evidence-release](11-reproducible-evidence-release/skill.md) | 报告、模型、源码和演示程序无法绑定版本 | manifest 工具、发布清单模板、证据分层和授权核对 | 多条模型路线与 Grid/Attention 案例；现场版仍须证据 |

## 使用方法

1. 根据问题打开对应 `SKILL.md`，复制其“可直接使用的提示词”，填入自己的输入路径和目标。
2. 先冻结该项要求的契约，再运行工具或执行工程步骤。所有地址、尺寸、类别映射和版本都由本队确认。
3. 保存命令、工具版本、退出码、输入/产物 SHA-256 和原始报告，按技能中的通过条件判断。
4. 输出包含“已证实、仅历史记录、未验证”的状态，不能把本包案例结果当作自己工程的验收结果。

例如，在本包上级目录运行：

```sh
python skill/02-tensor-preprocess-golden/scripts/compare_f32.py golden.f32 actual.f32 --count 307200 --atol 0.000001
python skill/03-armv7-inference-deploy/scripts/audit_elf.py paddle_person
python skill/04-petalinux-xsa-rebuild/scripts/verify_xsa_bit.py system_wrapper.xsa system_wrapper.bit
python skill/06-detection-stream-contract/scripts/check_stream.py targets.txt
python skill/06-detection-stream-contract/scripts/check_stream.py legacy.txt --legacy-size 640 640
python skill/11-reproducible-evidence-release/scripts/manifest.py create deliverables manifest.json
python skill/11-reproducible-evidence-release/scripts/manifest.py check deliverables manifest.json
```

脚本仅依赖 Python 3.10+ 标准库，退出码 `0` 表示检查通过，`1` 表示内容不符合要求，`2` 表示输入/用法错误。ELF 脚本只检查 ARM32 ELF 及 hard-float 标记，ARMv7/NEON、动态库和目标系统兼容性还需 `readelf` 与实际运行验证。

## Codex 技能格式

每项 `SKILL.md` 使用 `name`、`description` YAML frontmatter，可将需要的单项目录复制到自己的技能目录后使用。也可直接给支持读取本地文件的大模型提供该文件。比赛提交时保留本 `skill/` 目录即可，无须安装到本机全局技能目录。

## 验证与复用边界

[VALIDATION.md](validation.md)记录本次新增校验工具的测试与本地数据核验结果。历史 Vivado、训练、QEMU 指标在 [EVIDENCE.md](evidence.md) 中单独标注，未在本次重新执行硬件构建或训练。

本包没有 PYNQ 框架实测记录，因此不申报“已验证的通用 PYNQ Skill”。AXI 握手、DMA/cache、张量校验和发布检查可迁移到 PYNQ，但换成 `Overlay`/`allocate`/DMA Python API 后，需重新验证缓冲生命周期、flush/invalidate、TLAST 和完成条件，才能形成 PYNQ 专项提交。

本包整理自本队对话与工程观察；不为引用的第三方代码、权重、图片、数据集或上游库重新授予许可。公开提交前，按第 11 项逐项记录适用授权；不明处填“待确认”。

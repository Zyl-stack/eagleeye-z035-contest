# EagleEye 可复用技能包

本目录汇集项目开发中形成的验证工具、工作流、提示词和案例模板。按需要选择对应项目，将器件、模型、尺寸和接口替换为自己的实际配置；各项 README 或 `skill.md` 中列有使用方法、经验来源和失效条件。

## 视频与硬件验证

| Skill | 作用 |
|---|---|
| [AXI 视频流回归](01-axi-video-regression/skill.md) | 检查握手、反压、RGB 锁步、帧边界与逐像素 Golden。 |
| [FP32 预处理 Golden](preproc-fp32-golden/readme.md) | 比较预处理张量，包含示例和板端验证记录。 |
| [张量预处理一致性](02-tensor-preprocess-golden/skill.md) | 检查 FP32 格式、张量布局、数值误差和固定输入契约。 |
| [VDMA 帧快照](05-vdma-frame-snapshot/skill.md) | 梳理帧缓冲、同步、缓存和新鲜度的检查方法。 |
| [ROI 复判控制](roi-review-control/readme.md) | 核对暂停、鼠标、ROI 及硬件接口合同。 |
| [PYNQ Overlay 与 DMA](pynq-overlay-dma/readme.md) | Overlay、寄存器访问、DMA 与缓存一致性示例；目前为模拟对象验证。 |

## 检测与决策

| Skill | 作用 |
|---|---|
| [检测流接口合同](06-detection-stream-contract/skill.md) | 检查目标框、坐标、ID、尺寸与时间基准。 |
| [小目标跟踪回归](07-small-object-tracking-regression/skill.md) | 设计遮挡、小目标与跟踪错误的回归评估。 |
| [ROI 消融评估](08-roi-ablation-evaluation/skill.md) | 使用配对实验比较不同 ROI 处理方式。 |
| [ROI 请求事务](09-roi-transaction-integration/skill.md) | 检查请求关联、目标身份和过期结果处理。 |

## 构建与交付

| Skill | 作用 |
|---|---|
| [ARMv7 推理部署](03-armv7-inference-deploy/skill.md) | 核查 ELF、运行依赖和交叉部署边界。 |
| [PetaLinux 与 XSA 重建](04-petalinux-xsa-rebuild/skill.md) | 核查工具环境、XSA 与 bitstream 身份和构建证据。 |
| [HLS 跨平台移植](10-hls-cross-platform-port/skill.md) | 检查器件、接口和内存模型是否适合目标平台。 |
| [可复现证据与发布](11-reproducible-evidence-release/skill.md) | 建立文件清单、版本绑定和发布核验流程。 |

## 使用与验证

每项都有适用范围和验证边界。部分工具功能相近，但输入契约和输出有所不同，按任务选用即可。足球检测、AX7020 和历史工程案例仅作为可复用经验，不代表当前 Z035 系统已完成相同板端功能。

从仓库根目录运行主要离线检查：

```sh
python -B -m unittest discover -s skill/tests -v
python -B -m unittest discover -s skill/preproc-fp32-golden/tests -v
python -B -m unittest discover -s skill/roi-review-control/tests -v
python -B -m unittest discover -s skill/pynq-overlay-dma/tests -v
```

已有测试和来源见 [验证记录](validation.md)、[整理与复核记录](validation_local_20261030.txt)、[经验来源](evidence.md) 和 [Skill 复用测试](../sim/skill_portability_20261028.txt)。PYNQ 示例尚无实板或真实跨项目验证，不能把模拟结果写成实板结果。

为符合比赛仓库命名要求，展开后的技能入口使用小写 `skill.md`。若需安装到要求大写入口 `SKILL.md` 的工具，请在自己的独立副本中改名，或从 [原始技能包](../original/b_skills_20261029_v1.zip)提取原始大小写版本。该归档保留了原始内容与清单，未在本次整理中删除。

# 第三方模型、软件与 IP 来源核对

本清单记录本地已有的来源及许可文件，**不是对全部组件公开分发、比赛提交或商用授权的最终法律结论**。模型权重、框架和厂商 IP 的上游权益仍应单独标注。原源码和版权声明均保持不变。

| 项目 | 仓库或交接证据 | 目前能确认的许可 / 待核对事项 |
|---|---|---|
| PicoDet 架构与当前 `model.nb` | [当前模型](../data/model.nb) SHA256 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`；[`src/ps/licenses/notice.txt`](../src/ps/licenses/notice.txt) 记录来自先前 AX7020 交接。团队反馈交付说明将其描述为官方 PicoDet-XS 320 COCO LCNet 导出后转 Paddle-Lite ARM 格式、未由团队训练或量化；[官方模型表](https://github.com/PaddlePaddle/PaddleDetection/blob/release/2.6/configs/picodet/README.md)列有该型号。 | 团队反馈中两份交接模型哈希与仓库一致；所引用的另一台机器 E 盘原文件本次不可访问，尚未独立核对转换过程及官方原始文件身份。上游代码的许可证或文件哈希**不能单独证明该权重文件的比赛使用和再分发授权**；具体许可仍待确认。 |
| Paddle-Lite ARM 静态库与头文件 | 现场记录中的 `original/a_repro_delta_20261027.zip` 包含 `eagleeye_z035/build/deps/paddle_lite_armv7_build_dep.tar.gz`，SHA256 `858832912944fa9c2e91eada484c1bc127a8ba0a553aa9363e2ef91c872219c2`；归档同目录提供 `PADDLE_LITE_LICENSE.txt`。 | 本地交接许可证文件为 [Apache License 2.0 文本](third_party/paddle_lite_license.txt)，按原始字节保存；与 [Paddle-Lite 上游许可](https://github.com/PaddlePaddle/Paddle-Lite/blob/develop/LICENSE)对照。仍需核对实际静态库版本、被打包的其他依赖及各自的 NOTICE/许可，不把一个文件概括为整个应用的全部许可。 |
| Xilinx Linux 源码 | [`reference/linux_z035/readme.md`](../reference/linux_z035/readme.md) 记录 `linux-xlnx` 固定提交 `19984dd147fa7fbb7cb14b17400263ad0925c189`，源码归档通过 Git LFS 保存。 | 归档顶层 [`COPYING`](third_party/linux_copying.txt) 标明 GPL-2.0 与 Linux syscall exception，并提示其他许可也可能适用；[上游固定提交文件](https://github.com/Xilinx/linux-xlnx/blob/19984dd147fa7fbb7cb14b17400263ad0925c189/COPYING)对应。不能因此把归档中所有文件都简单认定为相同许可；发布时须按各文件及适用规则处理。 |
| ALINX 摄像头 IP | [`ov5640_ip/component.xml`](../src/ip_repo/video/ov5640_ip/component.xml) 的 vendor 为 `www.alinx.com.cn`，源码位于 `src/ip_repo/video/ov5640_ip/`。 | 厂商来源标识可确认；所收录文件对应的具体再分发许可或团队授权材料本次未找到，须核对交接与厂商条款。 |
| Digilent / Xilinx 动态时钟 IP | [`axi_dynclk_v1_0/component.xml`](../src/ip_repo/video/axi_dynclk_v1_0/component.xml) 标注 Digilent；[`mmcme2_drp.v`](../src/ip_repo/video/axi_dynclk_v1_0/src/mmcme2_drp.v) 包含 Xilinx 与 Digilent 的来源和声明。 | 来源声明不能自动作为完整许可；保留原版权与免责声明，核对对应 IP 版本的发布条款及第三方授权。 |
| AMD/Xilinx Vivado 生成文件及 IP | [`system.bd`](../src/vivado/eagleeye_7035_eyes.srcs/sources_1/bd/system/system.bd) 包含 `xilinx.com` 的 VDMA、DMA、PS7 等 IP；工程工具为 Vivado 2022.2。 | 具体 IP 和生成物按其各自适用许可核对；本次不能从 BD 的 VLNV 判断所有 IP 可自由公开再分发。 |
| MS7210 配置 RTL | [`i2c_dri.v`](../src/vivado/eagleeye_7035_eyes.srcs/sources_1/imports/rtl/ms7210/i2c_dri.v) 文件头包含 正点原子版权标识。 | 本次未找到与所收录版本明确对应的再分发授权，需按原始来源核对；原文件头保留。 |
| 项目预处理 IP | [`src/ip_repo/preproc/readme.md`](../src/ip_repo/preproc/readme.md) 和 `rtl/` 为团队交接的预处理实现。 | 本次未在该目录找到明确的独立许可声明；由团队确认作者及交付、比赛展示和公开分发范围，不擅自套用上游许可证。 |

足球 YOLO 的权重和 ONNX 资料属于独立参考路线，不作为当前 PicoDet 模型声明；若未来另行使用，应单独核对该文件、训练数据及授权。原始素材、日志、模型与第三方 IP 在公开之前还应由团队检查包含的个人信息和适用许可。

团队另反馈有 PicoDet-S 416 量化训练记录，数据配置为 `dataset/newdata`、产物为 `picos416.nb`；型号、输入尺寸和文件名与当前 XS 320 的 `model.nb` 不同，不能据此称当前模型使用该训练数据或经过该次微调。该训练记录位于另一台机器，本次未独立读取。

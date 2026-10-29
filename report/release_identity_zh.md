# 当前 PicoDet 演示版本与实测身份

本页把已核验文件、A 的现场确认和不同程序的日志分开记录。当前团队确认使用 PicoDet 人体检测；原仓库的足球 YOLO 是独立参考路线，不作为当前上板模型。

## 已有实测组合

A 的上板交接及预处理 A/B 实测说明使用以下组合：

| 项目 | 身份与依据 |
|---|---|
| 开发器件和工具 | XC7Z035FFG900-2；完整 Eyes 构建使用 Vivado 2022.2，见 [干净复现记录](clean_reproduction_20261028_zh.md)。 |
| BIT | [`z100_top.bit`](../board/reproduced_20261028/z100_top.bit) SHA256 `b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15`；A 已确认该复现 BIT 的摄像头、人体检测、鼠标 ROI、按键、舵机和 HDMI 六项功能正常。 |
| 实验 BOOT | [`boot.bin`](../board/reproduced_20261028/boot.bin) SHA256 `816ebd03dc533f857399938e533994c1989f8af4437ce5b361329da6d461b039`；A 的交接说明记录使用该 BOOT，分区内容与交接 BIT 的配置数据对应。 |
| AttentionPolicy 应用归档 | `linux_camera_smooth_trial_v7_20261013.tar.gz`，SHA256 `b41241dd9db598af66a6cb46d52c744c9b53a6c6461633823c1f62fbecdfdc59`，位于 `original/a_repro_delta_20261027.zip` 的 `eagleeye_z035/board/attention/`。归档内 `bin/paddle_person_live` SHA256 `785948413759d4478d8b99578da3a406f7d5870304756e386518fb6b4d73c57f`；[A/B 测试的身份记录](../board/measurements/preproc_ab_20261028/identity.sha256)与之相同。 |
| PicoDet 模型 | [`data/model.nb`](../data/model.nb) SHA256 `7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`；与 Attention 应用归档内模型及 A/B 测试身份记录一致。 |

上述应用日志和 A/B 测试证明的范围见 [应用证据](board_app_evidence_20261028_zh.md)和 [A/B 计时](preproc_ab_20261028_zh.md)。这些记录不证明当前板端张量与 Golden 一致；该对比仍无结果。

## 其他已有版本，不能混用证据

- 原 GridPolicy 稳定应用归档的可执行文件 SHA256 为 `a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`，源码重编身份见 [`src/ps_as_built/readme_provenance_zh.md`](../src/ps_as_built/readme_provenance_zh.md)。原资料是该历史版本记录，不代表它是本次 A/B 测试程序。
- 独立 PicoDet+C 候选的本地交叉编译 ELF SHA256 为 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；A 已确认该候选上板功能正常。AttentionPolicy 的原始日志对应上表的另一个 ELF，不能混作 C 候选原始日志，见 [`src/picodet_c_candidate/readme.md`](../src/picodet_c_candidate/readme.md)。
- 2026-10-28 干净克隆复现的新 BIT SHA256 为 `7dd0f6ac95d9b8e5f3c62db6707ea8ddad0130bbd23ec0c9cb17faaf413ada70`；其配置数据与 A 测试的 BIT 一致、整文件哈希不同。该新文件没有单独上板记录，见 [复现记录](clean_reproduction_20261028_zh.md)。

**报告口径：**主实测表引用 AttentionPolicy A/B 组合；PicoDet+C 候选引用 A 的现场功能确认并独立标识。若最终提交演示选择的是其他应用或镜像，更新版本表和相应真实证据，不将不同应用的日志拼成一次运行。

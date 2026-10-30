# B 交接的可复用 Skill

本目录整理自 B 提供的 `eagleeye_skills_20261029_v1.zip`，原包 SHA256 为 `972f1caebdc25bdb9b2c0a9d4deaef41faea2b1cad1734ff5c998bf04f756d7a`；未改写的原始包见 [原始归档](../../original/b_skills_20261029_v1.zip)。原包的 27 项清单和各文件哈希已核验通过。

## 入口

- [B 的技能索引与使用说明](skill/readme.md)：11 项工作流及工具。
- [经验来源与证据](skill/evidence.md) · [B 的原验证说明](skill/validation.md)。
- [本次整理与测试记录](validation_local_20261030.txt)。

11 项分别涵盖 AXI 视频回归、FP32 张量比较、ARMv7 程序核验、XSA/bitstream 一致性、VDMA 快照、检测流合同、小目标跟踪回归、ROI 消融、ROI 请求事务、HLS 跨平台移植及可复现发布。其中足球检测与 AX7020 是独立参考案例，不能直接作为当前 Z035 PicoDet 板端结果；PYNQ 的相关边界以各自文档为准。

## 文件命名与使用

为匹配比赛对仓库路径的命名要求，展开副本的文件路径转为小写，并更新了 Markdown 中对应的本地文件链接；原包及原包清单保持不变。B 提供的源码和历史记录没有删除。

因此展开副本中的入口名为 `skill.md`。若要将某项安装为需要固定大写入口文件名 `SKILL.md` 的 Codex Skill，请在**自己的独立副本**中改回 `SKILL.md`；也可直接从保留原始大小写的归档中提取。比赛材料浏览不需要安装到全局技能目录。

在本目录下可检查整理后的文件清单并运行 B 的工具测试：

```sh
python -B skill/11-reproducible-evidence-release/scripts/manifest.py check skill skill-manifest.json
python -B -m unittest discover -s skill/tests -v
```

该清单针对本目录整理后的 `skill/`；原包清单位于原始 ZIP 内。主机测试、历史交接记录和真实板端结果按各自范围区分，不以 B 的历史 PC 或 QEMU 记录冒充 Z035 实板验证。

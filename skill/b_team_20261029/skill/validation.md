# 本次技能包验证

验证日期：2026-10-29。本次验证对象是新整理的技能格式、校验工具和已有保存产物；没有重新训练、运行 Vivado 构建或操作开发板。

## 技能格式与工具测试

- 11 个 `SKILL.md` 均通过 skill-creator 的 `quick_validate.py`。为运行该官方校验器使用了本机自带 PyYAML 的 Python；技能包中的五个工具及测试本身仅使用 Python 标准库。
- `python -B -m unittest discover -s skill/tests -v`：18 项测试通过。
- 故障测试覆盖：FP32 截断、数量不符、NaN/Inf、误差超限和有符号零；ARM64/soft-float/截断 ELF；XSA bitstream 不同与多成员歧义；缺源尺寸、错误范围、重复/保留 ID、类别变化、帧截断；manifest 的修改、缺失、新增、路径逃逸、自引用和覆盖拒绝；CLI 的 0/1/2 退出码。

完整记录：[test-tools.log](validation/test-tools.log)、[skill-format.json](validation/skill-format.json)。

## 已有本地产物复核

| 检查 | 结果 | 能证明的范围 |
|---|---|---|
| zero/full/distinct/edge/captured 五组保存 RTL 输出 | 各 307200/307200 个 FP32 位模式相同 | 保存的完整张量与对应 Golden 相同 |
| captured_dma、captured_xsim 两组保存输出 | 各 307200/307200 个 FP32 位模式相同 | 保存的适配器联测/XSim 输出与 Golden 相同 |
| JPEG 版 paddle_person ELF 头 | ARM32、小端、EABI5、hard-float 标记通过 | 不能由此证明 NEON、动态库或实际运行 |
| 旧静态球流，显式 640×640 | 24 帧、12 条目标记录通过 | 语法/范围检查；缺尺寸时按预期拒绝，不猜默认 |
| 真实视频流，声明 1920×1080 | 272 帧、1616 条记录通过；507 条 held、134 次观测到 invalid→valid | 跨类别总体统计；不等于球的恢复真值或 ID 准确率 |
| XSA 内嵌/外部 system_wrapper.bit | 相同，2037636 bytes | 文件身份一致，不证明板级配置或启动成功 |

bitstream SHA-256：`bc5d35a5a414006309d0057ad6b99277847bd2f9e25162e99c92ea38d9336ed0`。

完整结果：[current-checks.json](validation/current-checks.json)。54 个原始依据文件的大小与 SHA-256 见 [source-fingerprints.json](validation/source-fingerprints.json)，其中不包含原始权重、数据或完整源码内容。

## 接收方复核

解压后在包含 `skill/` 与 `skill-manifest.json` 的目录运行：

```sh
python -B skill/11-reproducible-evidence-release/scripts/manifest.py check skill skill-manifest.json
python -B -m unittest discover -s skill/tests -v
```

格式校验不证明工作流在新板卡上成功，保存的 Golden 不证明本次重新运行过 RTL。历史资源/时序、QEMU、训练和上板反馈的证据范围见 [EVIDENCE.md](evidence.md)。迁移到新模型、工具版本、器件、坐标系或 PYNQ API 后，需按相应技能重新验证。

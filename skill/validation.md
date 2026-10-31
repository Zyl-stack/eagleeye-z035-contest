# 本次技能包验证

验证日期：2026-10-29。本次验证对象是新整理的技能格式、校验工具和已有保存产物；验证方式为主机工具测试和保存产物复核，训练、Vivado 及板端测试按来源记录列示。

## 技能格式与工具测试

- 当前保留的 10 个编号技能在历史记录中均通过 skill-creator 的 `quick_validate.py`。为运行该官方校验器使用了本机自带 PyYAML 的 Python；技能包中的五个工具及测试本身仅使用 Python 标准库。
- `python -B -m unittest discover -s skill/tests -v`：18 项测试通过。
- 故障测试覆盖：FP32 截断、数量不符、NaN/Inf、误差超限和有符号零；ARM64/soft-float/截断 ELF；XSA bitstream 不同与多成员歧义；缺源尺寸、错误范围、重复/保留 ID、类别变化、帧截断；manifest 的修改、缺失、新增、路径逃逸、自引用和覆盖拒绝；CLI 的 0/1/2 退出码。

完整记录：[test-tools.log](validation/test-tools.log)、[skill-format.json](validation/skill-format.json)。

## 已有本地产物复核

| 检查 | 结果 | 能证明的范围 |
|---|---|---|
| zero/full/distinct/edge/captured 五组保存 RTL 输出 | 各 307200/307200 个 FP32 位模式相同 | 保存的完整张量与对应 Golden 相同 |
| captured_dma、captured_xsim 两组保存输出 | 各 307200/307200 个 FP32 位模式相同 | 保存的适配器联测/XSim 输出与 Golden 相同 |
| JPEG 版 paddle_person ELF 头 | ARM32、小端、EABI5、hard-float 标记通过 | ELF 头部验证；NEON、动态依赖和实际运行分别检查 |
| XSA 内嵌/外部 system_wrapper.bit | 相同，2037636 bytes | 内嵌与外部文件身份核验，板级配置及启动分别验证 |

bitstream SHA-256：`bc5d35a5a414006309d0057ad6b99277847bd2f9e25162e99c92ea38d9336ed0`。

完整结果：[current-checks.json](validation/current-checks.json)。保留的原始依据文件的大小与 SHA-256 见 [source-fingerprints.json](validation/source-fingerprints.json)，其中不包含原始权重、数据或完整源码内容。

## 接收方复核

技能目录合并后，从仓库根目录运行：

```sh
python -B tools/verify_submission.py
python -B -m unittest discover -s skill/tests -v
```

技能材料已按用途统一整理在当前目录；格式、工具行为及保存张量按本次检查结果记录，目标板运行及 RTL 重跑分别列示。历史资源/时序、QEMU、训练和上板反馈的证据范围见 [经验来源](evidence.md)。迁移到新模型、工具版本、器件、坐标系或 PYNQ API 后，需按相应技能重新验证。

# 新完整构建入口实际验证（2026-10-27）

- GitHub 仓库快照与 A 增量经 `tools/restore_a_repro.py` 还原，并通过 275 项哈希。
- 使用 Vivado 2022.2，运行 `build/build_full_vivado.tcl`，输入还原的 A 快照、独立新输出目录、4 个作业。
- 实际结果：BD Validation 与目标生成通过；`SYNTH_STATUS=synth_design Complete!`；`IMPL_STATUS=write_bitstream Complete!`；XSA 生成。
- `tools/verify_vivado_outputs.py`：独立 BIT 与 XSA 内 BIT 逐字节一致；HWH 存在。
- BIT SHA256：`b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15`；XSA SHA256：`18667ead62c25cde514e871cba67e927c5ffd32af80d3a0d5e0565a8a56a7e42`；HWH SHA256：`fd6938d2dae6bbcd363df5d176892c38e714fbc9734d0be80c4eaeb1086ae9fc`。未上传新 BIT 或 XSA，也没有用其上板。
- 资源：LUT 24986、FF 28246、BRAM Tile 303/500、DSP 49/900；已约束路径 WNS +0.608 ns、WHS +0.007 ns。
- `timing_summary.rpt` 仍列出 12 个输入、33 个输出无延迟约束；`methodology.rpt` 有 8 项 Critical Warning，`cdc.rpt` 有 Critical 分类。这些不因 BIT 生成而消失。
- 本次未对新 BIT 进行实板验证。该构建为 Eyes 视频/预处理工程，不等于卷积加速器或完整 FPS 验证。

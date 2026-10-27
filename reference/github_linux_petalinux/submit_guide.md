# 上交和队友使用说明

建议提交整个 EagleEye_C_Attention_Sim 文件夹，但不要提交 build、.obj 或运行生成物。

## 队友操作

1. 从 GitHub 下载或克隆仓库。
2. 双击打开 EagleEye_C_Attention_Sim.sln。
3. 选择 Debug 和 x64，按 F5 运行。
4. 查看 src/main.c 了解主流程，查看 src/attention.c 解释 Attention，查看 src/motion_predictor.c 解释预测 ROI。
5. 运行 run_tests_msvc.bat 验证事务和足球场景。

## 模块对接

B 侧将真实候选目标填入 EE_GlobalTargetList；C 侧调用 Attention/Scheduler 生成 EE_FoveaRequest；A 侧按 request 的 ROI、mode 和 deadline 处理局部增强；返回的结果必须使用 request_id、target_id、frame_id 三项匹配后才能更新状态。

当前只需替换 fake_data.c 的输入来源，不要修改冻结的接口字段、事务匹配或 AXI-Lite register map。

## 重要文档

- README.md：项目入口和运行方法。
- SOCCER_MIGRATION.md：选题迁移背景、复用和新增内容。
- INTERFACE_SPEC.md：接口字段。
- PROGRESS.md：阶段结果。
- VITIS_PORTING.md：Vitis 平台边界。
- VIVADO_VITIS_BRINGUP_STEPS.md：真实硬件前的操作步骤。
- SOCCER_HARDWARE_BRINGUP_CHECKLIST.md：足球版第一次上板清单。

# PicoDet 注意力独立候选

此目录是独立应用候选，未修改 `src/ps/` 或 `src/ps_as_built/`。PicoDet 人体框通过 `app/src/picodet_c_attention.cpp` 直接进入注意力模块的适配、注意力评分、调度和 ROI 平滑，再经原有 `RoiAnimator`/`FoveaControl` 产生 Fovea 操作。

- 临时 target_id 由进程按相邻推理结果匹配生成；目标消失后清空历史，不保证跨遮挡、交叉等情况下的稳定身份。PicoDet 不提供正式跟踪 ID。
- 推理 sequence 不是可证明的摄像头帧号；日志中 `c_request` 是软件生成的请求，不代表硬件完成，`hardware_commit=pending` 也不是实板成功。
- 空目标、非法输入或超过 16 个候选时不再追踪旧目标；Hold 会等待正在提交的操作完成，然后阻止后续旧目标移动。已发出的硬件操作不能撤回。
- 本候选不包含足球检测或已验证的完整足球关注/控球链；只适配 PERSON 人体框。

## 构建和离线测试

先按 `report/a_supplement_results_20261027.md` 恢复工程快照，准备 PetaLinux 2022.2 的 ARM 工具链/sysroot，以及 已有的 Paddle-Lite 库。设置 `PROJECT` 和 `PADDLE_LITE_DIR` 后运行：

```sh
bash src/picodet_c_candidate/app/build_arm.sh
bash src/picodet_c_candidate/test_host.sh
bash src/picodet_c_candidate/test_hold.sh
```

交叉编译所生成的 ELF 在本机 SHA256 为 `1421b51bb95c448fe8ed244a21aff300d13156ecbf02ef9bde4b7b4a5c9a19f5`；这不是板端哈希读回。测试使用 mock 设备，不能证明硬件操作。QEMU 的 `--c-attention-dry-run` 仅允许离线 RGB、CPU 预处理且不启用硬件 ROI；一次输入 7 人框与原程序一致，并产生注意力 ROI。`sim/picodet_c_candidate/` 保留摘要。QEMU 不代表开发板 FPS。

## 板端反馈与证据边界

2026-10-27，现场已确认上板使用这次新 PicoDet 注意力候选且功能正常，按现场确认记录。该次原始串口及量化性能数据未随包提供；不能将后来 AttentionPolicy 版本的日志当作该候选的运行日志，也不额外推断量化指标。现有资料确认暂无独立稳定 target_id。

此候选保留原硬件访问方式，不提供覆盖稳定卡或自动部署步骤。需要现场验证时，按团队实际已核准的方案进行。

# AX7020 OV5640 Linux 摄像头 + PicoDet + 平滑 ROI v7 试验包

此包对应已在实板使用的 EagleEye `BOOT.BIN`/`image.ub` 和
`0x1e000000` 三帧 reserved-memory。无需 Vivado、XSA、bitstream、设备树或
PetaLinux 镜像变更。请保留当前启动卡上的旧程序，以便回退。

## 一条命令测试

把 `linux_camera_smooth_trial_v7_20261013.tar.gz` 复制到 SD 卡 FAT 启动分区。
断电后插卡，进入 PetaLinux，使用 `sudo -i` 切换至 root，再运行：

```sh
mkdir -p /tmp/eagleeye-v7
tar -xzf /run/media/mmcblk0p1/linux_camera_smooth_trial_v7_20261013.tar.gz -C /tmp/eagleeye-v7
cd /tmp/eagleeye-v7/linux_camera_smooth_trial_v7_20261013
FOVEA_ROI=1 ITERATIONS=100 sh ./run_board.sh
```

试验命令运行 100 次 PicoDet。测试时让 OV5640 拍摄有人活动的场景，观察 HDMI
是否保持实时画面。终端会显示 `Results: /tmp/eagleeye_v7_results/run_...`。
该目录内保留 `board.log`、`summary.txt`、`exit_code.txt`、
`first_frame.ppm`、`result.json`、前后 VDMA 状态和 dmesg。
结果在 `/tmp`，关机前可打包复制回 SD 卡：

```sh
tar -czf /run/media/mmcblk0p1/eagleeye_v7_results.tar.gz -C /tmp eagleeye_v7_results
sync
```

`FOVEA_ROI=0` 是默认值，可用 `sh ./run_board.sh` 保持与 v4 相同的推理行为。
第二次运行时，如果视频仍在
持续写入 DDR，脚本直接接入，不重置摄像头、时钟或 VDMA。冷启动时，它只
调用一次此前已经在板上跑出实时画面的初始化程序，并等待采样的 DDR
内容出现非零变化，然后再推理。超时或发现不同的 VDMA 地址/尺寸会停止，
并在日志中说明原因；不要在黑屏状态下反复运行旧版初始化程序。

`VIDEO_POLICY=attach` 可要求只接入现有视频，不执行冷启动初始化。
`READY_TIMEOUT_MS=45000` 是默认就绪等待上限；如果场景完全静止或全黑，
内容变化可能观测不到，请移动镜头或改善照明。`THREADS=1|2`、
`THRESHOLD=0..1` 和 `PIXEL_ORDER=rgb|bgr` 也可设置。运行需要 root、
`/dev/mem`、`/dev/i2c-0`，并且不能同时运行其他控制视频链的程序。

## 已完成与尚未完成

- 2026-10-12 实板基线：分步初始化/接入时，100 次推理全部完成，
  99 次有目标，共输出 761 个 PERSON 框，HDMI 全程动态，约 1.1 FPS。
  这是**上一版实板结果**，不是 v4 的实板结果。
- v5 ARMv7 hard-float 静态编译、QEMU Cortex-A9 离线推理、与旧版
  15 个框逐项比较、宿主状态/取帧测试、BusyBox shell 脚本验证已完成。
- v4 已由用户实板运行两轮，100 次和再次接入 20 次成功。v5 的
  Fovea 联动实板运行两轮，各20次、退出码0、mode=2、HDMI持续动态。
  v6 的九宫格选择已由用户实板观察。v7 的平滑更新尚未实板验收。
- `SNAPSHOT inference_sequence` 是**推理次数**，不是摄像头 `frame_id`。
  `content_changed` 只比较图像内容，静态场景可以不变。当前双次复制
  降低撕裂风险，但无法证明“最新完整帧”；需要后续可靠帧完成事件。
- v7 将九宫格选择与显示移动分离。`GRID ... target_roi=` 每次推理给出目标；
  独立循环每 50ms 最多移动 12 像素，并在每次 Fovea 接受后再发送下一步。
  `FOVEA request=... roi=...` 是实际提交位置。它只平滑画面，不预测球员运动。
- v7 尚未在 AX7020 实板验证。若出现硬件拒绝、黑屏或检测异常，
  立即停止试验并回到保留的 v6 包。
- 已有板端 S2MM `0x15810` 含内部/帧尺寸错误锁存位。v4 保留其状态并
  写入日志，没有清除或宣称已修复。即使本次识别稳定，也要复核流的
  SOF/TLAST、实际帧高和该错误是否持续发生。

## 软件来源和构建

`bin/video_bootstrap_verified` 与 2026-10-12 曾在实板成功启动 HDMI 的
`camera_display_probe_static` 字节相同。`bin/paddle_person_live` 使用相同的
PicoDet 模型和预处理；修改集中在视频接入、读帧检查和日志。

源码在 `src/`，宿主测试在 `tests/`，离线证据在 `evidence/`。若需重建，
使用本机 PetaLinux 2022.2 已完成的 AX7020 Yocto 工程，并设置
`PADDLE_LITE_DIR` 指向此前交付的 ARMv7 Paddle-Lite include/lib 目录，
执行 `bash build_arm.sh`。该第三方静态库不在此运行包中。

# 板端 USB 鼠标 ROI 应用

此目录配套 `EagleEye_7035_Mouse` bitstream。默认同时启用 PicoDet 自动 ROI 与 USB 鼠标暂停接管：

```sh
sh ./scripts/check_mouse.sh
sh ./run_board.sh
```

鼠标插入 Z100 底板任一 PS USB-A Host 口 J16-J19。实时播放时自动跟踪；KEY1 暂停后鼠标移动 ROI；右键恢复暂停瞬间位置；中键居中；滚轮调整灵敏度；KEY1 恢复后继续自动跟踪。KEY0 的缩放/边缘模式切换保持不变。

可选环境变量：`MOUSE_ROI=0|1`、`MOUSE_DEVICE=auto|/dev/input/eventX`、`MOUSE_SENSITIVITY=1..8`、`FOVEA_ROI=0|1`。鼠标缺失或断开不会终止推理，程序会等待并自动重试。

本应用不能与旧 bitstream 混用：构造函数会检查 Fovea 签名，但旧硬件没有暂停状态和手动命令语义。上板时必须使用 Mouse 工程新生成的 BIT/XSA/BOOT.BIN，并保留稳定版本用于回退。

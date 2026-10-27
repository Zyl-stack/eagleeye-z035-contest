# Mouse ROI SD 启动包进度（2026-10-20）

状态：**Mouse USB PHY 修正版已上板，USB 鼠标识别及暂停后手动 ROI 主功能已成功。** 2026-10-20 再次插卡时 E: 为 Dirty；完整备份后运行 `chkdsk E: /f`，写入后 E: 为 Healthy / NOT Dirty。

本机完整备份：`D:\EagleEye_FPGA_Competition\Z035_Camera_PicoDet_Preboard\mouse_roi_20261020\sd_backup_before_mouse_20261020`。卡内旧启动文件：`E:\BOOT_PRE_MOUSE_20261020.BIN`，SHA256 `CD138CC612DFF2AEF6DDBBE4DBBA7D34C5713CA383C61402865428B1E74E6E39`。

## 来源与校验

- 工程：`D:\EagleEye_FPGA_Competition\Vivado_EagleEye_7035_Mouse\EagleEye_7035_Mouse\EagleEye_7035_Mouse.xpr`
- 新 XSA：2026-10-20 14:46 导出，内含 bitstream 与 `system.bit` SHA256 一致。
- 完整实现：WNS `+0.438 ns`、TNS `0`。PS 初始化与已成功启动的 Z035 Preproc 基线 XSA 字节一致。
- `BOOT.BIN`：Bootgen 2022.2 生成，`-read` 已解析四个分区：FSBL、bitstream、U-Boot、DTB。Bootgen 输出了两条分区地址重叠 Warning；没有实板启动证据。
- `image.ub`、`boot.scr`：沿用已成功启动的基线文件，未重建。
- `mouse_roi_app_20261020.tar.gz`：已交叉编译的 ARM 鼠标/PicoDet 应用与 `run_board.sh`、校验清单。包内默认 `FOVEA_ROI=1`、`MOUSE_ROI=1`。

| 文件 | SHA256 |
|---|---|
| `BOOT.BIN` | `4C056D6973EFC3E2C2A14D5DA5908B8A2491E91A6D97C64B8FE498B65FC90626` |
| `image.ub` | `3C1CFD3C61296EDD45C2E1B2928FA737C2E36458479E61AFF5F25652591369DF` |
| `boot.scr` | `7013E14923FBE461C4ECD745214FCAFCB0FFADDB10865F9135CD8624637359F4` |
| `system.bit` | `4ADF509880D02E246193EDA398CD17FDD10B0A06540E81081D6868CEA67EC914` |
| `z100_top.xsa` | `EE330832E0593BBC127024CBE9E6B721C1D9B29319899ED03165A3BA2BB9C60E` |
| `mouse_roi_app_20261020.tar.gz` | `A65F363BA0ED172930353DFF000D9F74F744460B21A79A187048C4E061A835F7` |

## 写卡结果

1. 已写入 `E:\BOOT.BIN` 和 `E:\mouse_roi_app_20261020.tar.gz`，读回 SHA256 与上表一致。
2. `E:\image.ub` 和 `E:\boot.scr` 与已成功启动的基线文件哈希一致，保持不动。
3. 旧 `BOOT.BIN` 同时保存在本机完整备份和卡内 `BOOT_PRE_MOUSE_20261020.BIN`。
4. 已将新鼠标应用按现有自启动 worker 期待的顶层目录名重新打包，53 个内部文件的 SHA256 均通过。卡上自启动路径 `E:\linux_camera_smooth_trial_v7_20261013.tar.gz` 现在是新鼠标应用，SHA256 `3A6C372696C3AEE29FF3A5DB315D6135A013C34BA4A40417AAE48D8AEC756374`。`run_board.sh` 默认启用 `MOUSE_ROI=1`。
5. 原自启动应用在卡内另名保留为 `E:\linux_camera_smooth_trial_v7_20261013_PRE_MOUSE.tar.gz`，SHA256 `03A9E295B119678BA78BEA5F5CF961B071CEA552A815FF742CE7BD377297F692`；本机完整备份中也有原包。第一次 Mouse 写卡时 `image.ub` 和 `boot.scr` 未修改；随后的 USB Host 修复只更新了 `image.ub`，详见下节。

## USB Host 设备树修复（2026-10-20）

- 首次 Mouse 上板时应用已启动并读到 KEY1 暂停，但 Linux 的 `/proc/bus/input/devices` 为空；USB0 平台节点存在，ChipIdea 驱动已注册，却没有绑定到 USB0。
- 原 FIT 内嵌设备树的 `/axi/usb@e0002000` 缺少 `usb-phy` 引用和 `dr_mode`。按 Xilinx 2022.2 驱动及 USB3320/ULPI 板卡资料，添加根节点 `phy0` 和 USB0 Host 引用。
- 在 `usb_phy_fix_20261020` 中从旧 FIT 提取内核、设备树及 ramdisk，使用设备树 overlay 修改后重打包。新旧 FIT 的内核与 ramdisk SHA256 分别完全一致；新设备树仅增加 USB PHY 节点、Host 模式与引用。
- 卡上现用 `E:\image.ub` SHA256 `4DABCB81B69988E77BB1D0E550CBA4B7DE4419DEC3E107C24AAAC4960E21A7AD`。旧镜像保留为 `E:\image_PRE_USB_PHY_20261020.ub`，SHA256 `3C1CFD3C61296EDD45C2E1B2928FA737C2E36458479E61AFF5F25652591369DF`。
- 本机完整 SD 备份：`D:\EagleEye_FPGA_Competition\Z035_Camera_PicoDet_Preboard\mouse_roi_20261020\sd_backup_before_usb_phy_20261020`。
- `BOOT.BIN`、`boot.scr`、自启动鼠标应用包均未再修改。实板 Linux 枚举出 `USB Optical Mouse`，输入节点为 `event0`；用户确认鼠标 ROI 主功能成功。右键复位、中键居中及恢复自动跟踪尚未逐项反馈。

## 上板首测

鼠标插入 PS USB-A Host 口。新鼠标应用应随 SD 卡自动启动；先在串口检查 `dmesg | grep -Ei 'ulpi|ci_hdrc|usb|hid' | tail -40`、`cat /proc/bus/input/devices` 和 `/tmp/eagleeye-autostart.log`。看到 USB Host 与鼠标 event 设备后，按 KEY1 暂停并移动鼠标，确认 ROI 与右侧放大区跟随。任何黑屏、启动失败或鼠标异常先保留串口日志；仅回退本次 USB 修复需将卡内 `image_PRE_USB_PHY_20261020.ub` 恢复为 `image.ub`。如要完整回到 Mouse 之前，还需同时恢复旧 BOOT 和旧应用包。

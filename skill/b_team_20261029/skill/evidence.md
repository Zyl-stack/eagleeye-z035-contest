# 经验来源与证据覆盖

本文件让评审能追溯经验来源。历史聊天中的结论是线索，现存文件能支持的部分优先采用文件证据。原始文件不随本包重复分发；下方路径以原工作区 `E:/Lanczos` 为基准，移交后可用发布清单绑定其实际位置和 SHA-256。

## 对话覆盖

已逐页读取以下 10 个聊天，所有分页均到 `hasMore=false`，合计 93 回合。标题按工具返回原文记录。当前聊天补充了 model.nb 来源、训练路线与授权核对。此覆盖不等于读取了账户中不可访问或已删除的对话。

| 聊天标题 | 聊天 ID | 回合数 | 提炼主题 |
|---|---|---:|---|
| 回答项目复现问题 | 01a17d4f-c3e8-76d2-88f9-98aa430ba122 | 3 | 权重/ONNX 区分、640×640 旧流、Grid/Attention、日志证据 |
| 实现 PicoDet FPGA 图像预处理 IP | 01a14925-beb8-7d33-a67a-317beea43911 | 2 | 完整张量、FP32 Golden、源码重建、DDR 延迟边界 |
| 梳理仓库并综合 XC7Z100 | 01a1384e-da44-7412-877b-d32951847fb4 | 10 | 仓库分工、HLS 迁移、器件与工具授权预检 |
| 构建 PetaLinux 2022.2 镜像 | 01a11905-f068-7a22-bccd-aef23950cca8 | 5 | 宿主环境、ext4、工具版本、阻塞日志 |
| 实现 AX7020 实时 PicoDet 推理 | 01a11192-e05f-7542-bab7-882dfe2da4f6 | 5 | VDMA 快照、硬件接管、QEMU 与板端区分 |
| 准备 AX7020 Paddle-Lite 人体检测 | 01a104f9-ae8d-7f92-8bfa-254815010327 | 2 | ARMv7hf、glibc、静态链接、模型转换 |
| 搭建 AX7020 ONNX 单图推理 | 01a0ec5b-52fe-7d81-9dd7-608083c5be46 | 1 | 单图移植边界、letterbox、NMS、缺失产物 |
| 梳理持球队员跟踪任务 | 01a0a436-6e99-7893-bfab-7c11cb61735d | 36 | 数据审计、小目标训练、真实视频、源尺寸/时间基准 |
| 指导 GitHub 协作流程 | 01a08969-85e7-7eb2-8ad6-0a974caefa89 | 17 | 复用旧工程、字段换算、B/C 闭环与分支交接 |
| 优化车载危险识别比赛方案 | 01a06618-1b7d-76b2-8064-1f871d716470 | 12 | 技术因果链、ROI 配对消融、训练与领域差异 |

## 证据案例

### E01：视频流的正确性不能只看静态画面

来源：`Code/scaler_single_channel_fixed/README.md`、`Code/scaler_rgb_fixed/README.md`、`Code/lanczos_rgb_wrapper_latest/README.md` 及其 testbench。

现存说明记录：单通道 16×12 连续两帧 1536 个像素、640×8 目标行宽测试 20480 个像素零错误；RGB 连续两帧 1536 个像素零错误。检查覆盖 clamp、Q10 数值、SOF/EOL、反压保持、通道锁步。旧版本坐标总线宽度错误，后改为 `$clog2(2*IMG_WIDTH)` 等按数值范围计算。全分辨率长时上板稳定性不由这些测试证明。

### E02：预处理必须验证整个张量

来源：`Qiansai/picodet_preproc_ip/README.md`、`vectors/*.golden.f32`、`tools/run_verification.py`、`build/vivado_impl/*.rpt`。

历史记录：五类 640×360 RGB888 输入，每帧输出 307200 个 FP32，与 CPU 双线性 Golden 逐位一致。Vivado 2024.2 独立 IP 后布线报告：709 LUT、333 FF、25.5 BRAM36 等效 tile、1 DSP、100 MHz WNS +3.519 ns。流接口仿真 7.88884 ms；真实 DDR 端到端延迟、模型框变化和 2022.2 重跑未验证。该固定尺寸的插值结果落在 1/32 像素值网格，归一化 ROM 按 n/32 编址，Golden 直接以双精度插值后归一化再转 FP32。改变尺寸后不能直接沿用这张 ROM 和相位表。

### E03：ARM 指令集正确仍可能因系统库失败

来源：准备 AX7020 Paddle-Lite 人体检测对话；`Qiansai/交付文件/paddle_person_ax7020/README.md`、`expected_result.json`、`SHA256SUMS`。

历史失败：早期动态链接二进制要求 `GLIBC_2.27`，旧板端系统兼容性未满足，后改为完全静态链接。Cortex-A9 QEMU 运行 JPEG 检出 8 个 PERSON，最高置信度 0.8298。实时 raw-frame 程序的另一份 QEMU 日志检出 6 个 PERSON，两份输入/程序不同，结果不能混用。QEMU 不能提供板卡 FPS。

### E04：镜像重建前先识别工具代际

来源：构建 PetaLinux 2022.2 镜像对话；`team_handoff_20261005_v1/DELIVERY_README.md` 及 `integration/linux_ov5640_picodet/scripts/`。

观察：XSA 标示 2022.2，旧 2017.4 流程期待 HDF；现有 Ubuntu 24.04/26.04 环境与本次推荐宿主版本不匹配，且未安装所需 PetaLinux。历史请求的 BOOT.BIN/image.ub 未因脚本已写好而视为完成。宿主支持表和工具命令随版本变化，复用时应查目标版本官方文档。

### E05：VDMA 私有快照保护推理输入

来源：`team_handoff_20261005_v1/integration/linux_ov5640_picodet/README.md`、`paddle_person_ax7020/src/paddle_person.cpp`、`deliverables/logs/qemu_offline_validation.log`。

代码选择最近完成槽、复制到私有内存并复查写指针。该策略依赖复制期间写者未回绕覆盖所选槽。仅前后两次环形指针相同不能排除多次回绕；复用必须增加可证明的时间界限、单调帧序号或显式缓冲所有权。本包将此作为验收缺口说明，不把离线运行称为真实 DMA 并发验证。

### E06：源坐标系曾被旧说明误判

来源：持球队员跟踪任务及回答项目复现问题对话；`Qiansai/EagleEye_C_Attention_Sim_git/B_DETECTION_INTERFACE.md`、`build/quality_cases/ball_static_b_target_stream.txt`、`build/real_match/real_match_metrics.json`。

旧对话曾依据 PC demo 默认约定和框落点，误把静态流解释为 640×360；2026-10-27 后续复核其生成图像为 640×640。框落在某个范围内不足以确定源分辨率。新流增加 `STREAM_INFO,width,height`；`imgsz=1280` 与源尺寸无等价关系。真实视频 metrics 记录 source_fps=25、frame_stride=2、frames_processed=272、detector_fps=0.817；12.5 Hz 是源时间下的采样频率，不是推理吞吐。

### E07：小球回归需要保留失败案例

来源：持球队员跟踪任务对话；`VISION_DETECTION.md`、`B_TO_C_YOLO_HANDOFF.md`、`tests_py/`、`build/real_match/`。

历史记录：两类数据合并 6111 张图片，训练集 12228 个 PLAYER 框、1011 个 BALL 框；CPU 1 epoch、1% 数据冒烟 mAP50=0.00063，只证明链路能运行。队友正式训练报告指标为 P=96.1%、R=91.4%、mAP50=95.8%、mAP50-95=69.9%，不能外推真实视频/板端性能。真实视频分块检测仍有低置信假球；目标数上限内增加 BALL 保留策略，遮挡时保留 valid=0 的短时轨迹。上述数值是历史记录，本次未重训。

### E08：负消融结果同样是可复用结论

来源：`Qiansai/B_Perception/RESULTS_V1.md`、`results/ablation_v1/SUMMARY.md`、`results/refine_v1/training_report.json`。

VOC2007 代理数据，训练 1200 ROI、验证 400 ROI、同一 TinyFoveaCNN。Closed-set Accuracy：Nearest 49.00%、Bilinear 53.25%、Lanczos 49.75%。Operational Accuracy：19.50%、14.25%、20.50%。后者包含 UNKNOWN 拒识条件，不能与闭集准确率混写。小目标仅 23 个样本，不足以支撑强结论。Pillow Lanczos 也不等于 FPGA 定点实现。

### E09：字段同名不代表语义兼容

来源：`Qiansai/B_C_Integration/README.md`、`build/closed_loop_report.json` 及指导 GitHub 协作流程对话。

旧 B 置信度 0..255，C 为 0..100；速度 Q8.8 与整数不同；OBSTACLE 不可直接转为 BICYCLE；区域枚举语义不同。现存 12 帧软件闭环报告为 PASS，正常结果接受、过期结果拒绝、request ID 唯一。输入为合成视频、软件 Lanczos 和 VOC 模型，未证明 FPGA ROI/板端闭环。

### E10：综合阻塞要定位在真实阶段

来源：`Qiansai/Accelerator/xc7z100_hls/README.md`、`XC7Z100_加速器交接说明_20261014.md`。

三个仓库分别负责训练、推理运行时和 HLS 算子；无 IK 的 final_compare 与 http_backend 卷积文件哈希一致。代码只覆盖特定融合算子，存在输入尺寸/通道上限及 Intel FPGA 的运行时假设。Z100 尝试停止于 `set_part`，未生成资源/时序结果。器件文件、工具版别和可用授权是不同检查项。

### E11：发布身份必须细化到文件

来源：回答项目复现问题与当前聊天；PicoDet 交付 README、训练 README、各 SHA256SUMS。

官方 XS-320 model.nb 与本地 S-416 QAT picos416.nb 记录属不同路线；YOLO 1280 导出 ONNX 与 ARM 512 ONNX 也不是同一文件。`runs/`、`build/` 被忽略时，分支名不能交付产物。2026-10-27 复现包的历史核对记录区分 GridPolicy as-built 与后续 AttentionPolicy，并注明用户反馈上板通过但无原始当次日志。该附件当前未重新读取，因此本包不把它作为本次独立确认的现场状态。GitHub 最新仓库未登录访问返回 404；不能据此认定仓库不存在。权重/数据及比赛用途授权不明处为“待确认”。

## 如何复核这些依据

拿到原工程后，先生成文件 SHA-256 清单，再查看报告生成工具、输入和时间。要复跑哪一项，就按对应技能的工作流建立新的结果目录。不要覆盖历史日志或用模板输出替代测量输出。本次新增脚本的结果见 [VALIDATION.md](validation.md)。

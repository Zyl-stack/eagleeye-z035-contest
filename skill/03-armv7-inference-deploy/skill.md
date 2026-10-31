---
name: 03-armv7-inference-deploy
description: 将视觉模型分阶段移植到 Zynq Cortex-A9 ARMv7 Linux，排查 ABI、指令集、运行库和模型转换版本问题，建立 QEMU 与板端验收。
---

# Cortex-A9 推理移植与 ABI 验收

## 适用范围

目标为 ARM32 Linux CPU 推理，典型平台为 Zynq-7000 Cortex-A9。根据 SoC 和实际系统资料确认指令集、驱动及后端。模型训练、PL 卷积迁移和实时摄像头链路分别验收。

## 工作流

1. 获取板卡完整 SoC、CPU/NEON、ARM32 endian、hard/soft-float、内核、glibc、RAM 和可用存储。记录实际 rootfs 信息；不要把开发主机的 `uname` 当成板端信息。
2. 固定模型 SHA-256、输入输出 shape、类别表、预处理和后处理。先完成单图加载/推理，再接 raw-frame，最后接实时采集。YOLO 导出需检查是否有内置 NMS、输出 layout 和 letterbox 逆变换。
3. 选择确实支持目标 ABI 的运行时、交叉工具链和模型优化器，固定 commit/版本及 flags。Paddle-Lite `.nb` 应绑定优化器和运行库；不应随意混用不同分支生成物。
4. 用 `scripts/audit_elf.py` 排除 x86/aarch64/soft-float 标记问题，再用工具链的 `readelf -h -A -d`、`objdump -T` 检查架构属性、NEON、解释器、动态库及 GLIBC/GLIBCXX 符号版本。ELF 标记通过只是一项必要条件。
5. 若旧板端 glibc 不满足要求，优先使用匹配 SDK/sysroot；可在运行时和许可允许、功能支持时评估静态链接。静态链接不消除内核系统调用、驱动及 CPU 指令要求。
6. 用相同 ARM 二进制和模型在 `qemu-arm -cpu cortex-a9` 上执行确定输入，保存结果 JSON、完整日志和哈希。对比可信 CPU 参考，检查有限输出、类别和源坐标；不能仅以进程返回 0 证明结果正确。
7. 到板端再验证启动、模型加载、真实输入、连续推理和分阶段耗时。记录 `warmup/repeats/threads`，只把板端采集结果称为板端性能。

## 使用方法

```sh
python scripts/audit_elf.py ./paddle_person
arm-linux-gnueabihf-readelf -h -A -d ./paddle_person
arm-linux-gnueabihf-objdump -T ./paddle_person
qemu-arm -cpu cortex-a9 ./paddle_person <实际支持的单图参数>
```

最后一行按实际程序帮助替换单图参数后执行。

## 失效条件

只有 x86 零张量推理、只有头文件语法编译、只有模型 checker、缺测试图或 ARM 依赖库时，不能交付为板端成品。QEMU 耗时不能换算成板卡 FPS。地址、驱动与缓存错误也无法由 QEMU 用户态推理排除。

## 已验证效果与失败来源

[E03](../evidence.md#e03arm-指令集正确仍可能因系统库失败)：曾出现 ELF 架构正确而数学函数依赖 GLIBC_2.27，后以静态链接解决本次库依赖。历史 Cortex-A9 QEMU JPEG 测试检出 8 PERSON。另一路 ORT 单图包因缺目标 SDK/图片/镜像而未完成板端验收，避免将源码准备误写成部署成功。

## 可直接使用的提示词

> 请将这个模型移植到给定 ARM32 Linux，先核对目标 ABI/sysroot、模型输入输出与运行时版本，再验证单图 ARM 二进制。交付 ELF 属性、依赖、模型/程序哈希和确定输入结果；分别列出主机、QEMU 和真实板端完成了哪些测试，保留失败日志及未生成的产物清单。

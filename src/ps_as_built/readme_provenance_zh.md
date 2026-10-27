# 已上板 PicoDet 应用源码身份

本目录的 37 个源文件、脚本、测试文件逐字节提取自 `board/sd_boot/linux_camera_smooth_trial_v7_20261013.tar.gz`（SHA256 `3a6c372696c3aee29ff3a5db315d6135a013c34ba4a40417aae48d8aec756374`）。逐文件哈希见 `SOURCE_SHA256SUMS.txt`。

- 实际交付的 ROI 策略为 `GridPolicy`（九宫格）；`src/ps/` 中的 `AttentionPolicy` 是后续拟替换源码，尚未上板验证，不能用它解释旧 BOOT/应用包的演示结果。
- 应用包内 `bin/paddle_person_live` SHA256：`a16025dacfb8f02cee60a82b8c236e32319f724514baa50a0898b2c189f18aea`。
- 模型 `model.nb` SHA256：`7c8a381bb5ba4a6a3c5c2b3277fd2cfb6ba1f802ddffd854c73bf75e747bfff4`，与提交目录 `data/model.nb` 一致。
- 2026-10-27 已用本机 PetaLinux 2022.2 交叉工具链、GCC 11.2.0 及 `build/deps/paddle_lite_armv7_build_dep.tar.gz` 中的静态库，从本目录源码重新编译。生成的 `paddle_person_live` SHA256 与应用包中的二进制 **完全一致**。这验证了软件源码与已上板二进制的对应关系；不等于注意力版本已经上板。

重建步骤、依赖哈希和板级工程边界见 `../../build/REPRODUCE_ZH.md`。

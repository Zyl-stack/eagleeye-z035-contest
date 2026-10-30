# 注意力参考子集

此目录仅包含 `attention.c`、`attention_scheduler.c` 和对应的三个头文件，用于解释后续注意力 ROI 方案；不是完整注意力仿真仓库，也不是已上板 PicoDet 程序。来源参考仓库 `EagleEye_C_Attention_Sim` 的 `c-attention` 分支，当前核对提交为 `25bd7a35ab1a072f004e75cad20331a3d128c166`。所用子集与当前分支的全部文件/构建目标并不等价。

在 Linux/WSL 下运行 `make` 可编译该目录实际包含的两个 `.c` 为静态库 `build/libeagleeye_attention_reference.a`；2026-10-27 已在独立副本中通过 GCC 编译。完整 PC 仿真应另取该 Git 分支并以其自身的测试、Makefile 和提交号核验。不要把该 PC 仿真或足球 PLAYER/BALL 检测描述为 Z035 已上板功能。

# 视频、检测、注意力与 Linux 参考源码

本目录保留视频、注意力和 Linux 的参考代码，与 `src/` 中的 Z035 Eyes 主工程独立管理。表中提交号说明来源；当前参考子集按 PicoDet 提交范围整理，对应 `original/` ZIP 保留原始文件名并与展开目录逐字节核验。整理前快照另存本地及 Git 历史。

| 目录 | 来源分支与提交 | 对应关系和限制 |
|---|---|---|
| `github_a_fovea/` | `a-fovea`：`729a8d05a01ecac1b97351dfc5d1d6654294f304` | 视频、ROI 和 Vivado/Vitis 参考；其中 XPR 以 XC7Z020 为目标，Z035 主工程另行构建 |
| `github_c_attention/` | `c-attention`：`e91d713a5b42153d29f7b80ad53ae6709c656716` | 注意力、目标选择和调度；本地 `sim/github_c_*` 是历史主机编译/模拟，不是板端结果 |
| `github_linux_petalinux/` | `linux-petalinux`：`9fada9e03e19e22c14ff23f25fb63c3782ea0491` | PetaLinux 配置、设备树片段和应用源码，目标为 AX7020；不含完整 Linux 内核源码，也不是已绑定 Z035 的镜像 |

`sim/github_c_build.log`、`sim/github_c_simulation.log`、`sim/github_linux_reproduce.log` 和 `sim/github_a_inspection.txt` 是先前整理过程留下的记录。这些记录覆盖各参考模块的主机测试及静态检查，主工程板端联调见正式报告的现场资料。使用这些材料前请核对器件、提交版本、接口、模型和测试环境。

比赛是否允许归档内使用原始大小写、各第三方源码和模型能否公开分发，仍需以组委会规则和权利人授权为准。

本地 Z035 稳定系统配置、固定提交的完整 Linux 源码归档及独立 eeiso22 实验内核补丁另见 [linux_z035](linux_z035/readme.md)。完整源码通过 Git LFS 分发；与现场镜像的二进制对应关系仍待核验。

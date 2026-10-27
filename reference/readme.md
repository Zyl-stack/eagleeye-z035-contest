# A、B、C 和 Linux 参考源码

本目录提供本地已有的 GitHub 分支快照，供比赛材料查阅；与 `src/` 中的 Z035 Eyes 板级主工程保持独立，未自动拼接。原始大小写及全部 Git 跟踪文件分别保存在 `original/` 下对应 ZIP。

| 目录 | 来源分支与提交 | 对应关系和限制 |
|---|---|---|
| `github_a_fovea/` | `a-fovea`：`729a8d05a01ecac1b97351dfc5d1d6654294f304` | 视频、ROI 和 Vivado/Vitis 参考；其中 XPR 目标为 XC7Z020，不能直接替代 Z035 工程 |
| `github_b_detection/` | `b-detection`：`a4d9226b401c878c6a83a6e520966f997262b492` | 足球 PLAYER/BALL 检测与跟踪；与主工程 PicoDet 人体检测不是同一模型，未证明在 Eyes 上联调 |
| `github_c_attention/` | `c-attention`：`e91d713a5b42153d29f7b80ad53ae6709c656716` | 注意力、目标选择和调度；本地 `sim/github_c_*` 是历史主机编译/模拟，不是板端结果 |
| `github_linux_petalinux/` | `linux-petalinux`：`9fada9e03e19e22c14ff23f25fb63c3782ea0491` | PetaLinux 配置、设备树片段和应用源码，目标为 AX7020；不含完整 Linux 内核源码，也不是已绑定 Z035 的镜像 |

`sim/github_b_tests.log`、`sim/github_c_build.log`、`sim/github_c_simulation.log`、`sim/github_linux_reproduce.log` 和 `sim/github_a_inspection.txt` 是先前整理过程留下的记录。B/C/Linux 的主机测试与 A 的静态检查均不表示本仓库主版本完成了板端联调。使用这些材料前请核对器件、提交版本、接口、模型和测试环境。

比赛是否允许归档内使用原始大小写、各第三方源码和模型能否公开分发，仍需以组委会规则和权利人授权为准。

本地 Z035 稳定系统配置、固定提交的完整 Linux 源码归档及独立 eeiso22 实验内核补丁另见 [linux_z035](linux_z035/readme.md)。完整源码通过 Git LFS 分发；它不是已经证明与现场镜像逐字节一致的构建。

# PYNQ Overlay、寄存器与 DMA 接入示例

## 问题来源和适用范围

EagleEye 开发中曾出现硬件描述与实际工程不匹配、数据搬运和缓存状态难以区分的问题。本 Skill 将可复用的文件身份核对、寄存器访问边界、简单 AXI DMA 和缓存维护顺序拆成小例子，适用于**自行核实过接口契约的 PYNQ 项目**。代码不依赖 EagleEye 的 IP 名称、地址、图像大小或模型。

**当前只有 Python 模拟测试，没有 PYNQ 实板或真实外部工程验证。** 示例不是完整设备驱动、隔离机制、硬件安全准入或可直接部署的 Overlay；不提供实际 BIT/HWH。哈希核对只验证文件与给定值一致，不能证明来源可信、BIT 与 HWH 真实配套或适用于某块板。

## 脱离工程运行模拟示例

将本目录复制到空目录，使用 Python 3.10+，无需安装 PYNQ 或 NumPy：

```sh
python -B examples/mock_demo.py
python -B -m unittest discover -s tests -v
```

第一条只打印带 `MOCK` 的缓存和传输顺序，没有产生 FPGA 事务或数值结果；第二条使用 Python 假对象检查成功、身份不符、未授权寄存器写、DMA 异常、长度错误及中断。验证记录见 [本仓库记录](../../sim/pynq_skill_mock_20261028.txt)。复制到别的项目时只复制本目录即可，仓库记录链接不属于运行依赖。

## 硬件使用前的契约

1. 确认 PYNQ、设备和板卡支持目标 BIT；BIT 与同名 HWH 来自同一构建；输入 SHA256 从可信交付渠道获得。装载 Overlay 会重配置 PL，可能影响当前其他任务；在独立实验环境并获得现场许可后才执行。
2. 确认所需 IP 实例名、寄存器宽度与偏移、读写副作用、流方向、字节数、DMA 地址宽度、对齐、DRE、最大传输长度、TLAST、复位/时钟、设备和缓冲区的唯一所有权。检查 HWH 是元数据核验，不是现场硬件身份证明。
3. 下面 DMA 例子仅适用于**简单模式、明确接收和发送长度、约定帧格式匹配**的 AXI DMA 流设计。不能直接用于 scatter-gather、VDMA、多主设备争用或未审查的任意 Overlay。部分 HLS IP 还需要按自己的协议设置寄存器和启动；本示例不猜测这些动作。
4. 任一错误、通道状态不明或超时，停止新任务；不能声称 DMA 已排空，不能立刻释放或重用缓冲区。按目标工程已验证的方式隔离和恢复；本 Skill 不实现故障恢复或全局独占。

## Overlay 与寄存器

在本 Skill 根目录启动 Python 或 Notebook；以下显式加入 `tools/` 搜索路径。两个 SHA256 和 IP 名必须替换为从可信交付资料取得的实际值：

```python
import sys
sys.path.insert(0, './tools')
from pynq_examples import prepare_overlay, download_overlay, read32, write32

# 此调用核验本地文件并用 download=False 解析，不自动重配置 PL。
ov = prepare_overlay('/tmp/demo.bit', '/tmp/demo.hwh',
                     '<bit_sha256>', '<hwh_sha256>', ['my_ip_0', 'axi_dma_0'])
# 仅在确认设备、其他任务、恢复方法及现场许可后执行：
download_overlay(ov, approved=True)
ip = getattr(ov, 'my_ip_0')
# 仅使用已确认允许读取的寄存器偏移；读取本身也可能有副作用。
value = read32(ip, 0x10, allowed_offsets={0x10})
# 写操作默认拒绝。仅在已核实寄存器语义并获准时：
# write32(ip, 0x14, 1, allowed_offsets={0x14}, approved=True)
```

`register_map` 可用于查看 HWH 描述，例如 `ip.register_map`；不同 IP 和 PYNQ 版本支持情况不同，显示或读取寄存器可能触发只读清除等副作用，须先查设备手册。若只希望离线核对 HWH 内模块、端口及基地址，可使用仓库中的 `roi-review-control/tools/check_hw_contract.py --hwh ...`，无需加载 Overlay。

## DMA 与缓存示例

针对已核实满足上述契约的环回或处理流水线，且输入、输出缓冲的整个字节长度与协议长度一致：

```python
import numpy as np
from pynq import allocate
from pynq_examples import SimpleDmaSession

# ov 已经按上节核验和获准加载；长度、类型、实例由自己的设计确定。
input_buffer = allocate(shape=(1024,), dtype=np.uint8)
output_buffer = allocate(shape=(1024,), dtype=np.uint8)
input_buffer[:] = np.arange(1024, dtype=np.uint8)
session = SimpleDmaSession(ov.axi_dma_0, input_buffer, output_buffer)
result = session.run()  # 完成后才允许读取；按实际协议对照 Golden
```

`run()` 按输入和输出缓冲 `flush`、先提交接收再提交发送、等待发送及接收、检查通道错误和实际字节数、输出 `invalidate` 执行。PYNQ 驱动在某些版本也会进行缓存操作；重复维护的行为和代价需在目标系统确认。接收前清理输出缓冲的脏缓存行；运行期间 CPU 不得读写正在由设备使用的缓冲。`allocate` 返回的缓冲是 DMA 目标地址对应的连续内存，普通 NumPy 数组不能直接当作 PYNQ DMA 缓冲。

**等待限制：**PYNQ 不同版本的同步 `wait()` 接口和实现不同，某些版本可能无限等待；本例没有可保证的硬超时。设备读、传输提交及缓存操作也可能阻塞。不得把外部线程超时或 Ctrl-C 当作停止 DMA、排空事务或释放内存的证明。需由目标平台提供经过验证的超时、隔离和冷恢复方案；不具备时不要运行硬件示例。

异常或中断时 session 标为 `faulted`，持有输入/输出对象并拒绝重试，不做自动 reset、stop 或 free。**对象引用不能保证进程退出后 DMA 内存安全**；不得在事务状态不明时退出、释放或复用，应先按本机经过核验的恢复流程处理。成功 `wait` 或 `idle` 也不单独证明其他 AXI master 已排空。仅当目标系统确认相关设备停止且缓冲无其他使用者后，才按 PYNQ 对应版本释放缓冲。

## 常见问题与诊断

- 文件哈希或必需 IP 名称不匹配：停止；检查 BIT/HWH 版本和来源，不能靠修改预期值掩盖。
- 寄存器读写不符合白名单：停止；核对地址、宽度和只读清除行为，不能盲写控制寄存器。
- DMA 不完成：先检查流方向、TLAST、时钟复位、长度和驱动版本；DMA `wait()` 可能阻塞，不要假设已有超时或复位自动安全。
- 数据不一致：核查输入、dtype、对齐、完整接收字节数、缓存和 Golden；不要跳过 flush/invalidate 或随意放宽阈值。
- 视频系统共用资源：需要系统级所有权和实际排空证据，本例不负责仲裁。

## 官方资料与验证边界

参考 [PYNQ allocate 与缓存 API](https://pynq.readthedocs.io/en/latest/pynq_libraries/allocate.html)、[PYNQ 2.7 DMA 实现](https://pynq.readthedocs.io/en/v2.7.0/_modules/pynq/lib/dma.html) 和 [PYNQ Overlay 接口](https://pynq.readthedocs.io/en/v3.0.0/_modules/pynq/overlay.html)。实际使用须对照所装版本。当前仅在 PC 的模拟对象上测试控制流，未验证 Overlay 重配置、寄存器、DMA、缓存一致性、目标板资源隔离或跨板复用；不能以此申请“实板已验证”结论。

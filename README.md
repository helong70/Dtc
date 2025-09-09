# DTC（Diagnostic Trouble Code）模块

## 概述
本仓库实现了一个简单的 DTC（故障诊断代码）功能模块，用于基于阈值和迟滞判断信号的异常（过压/欠压等），并支持计数确认与清除逻辑以避免抖动误判。代码以纯 C 语言实现，适用于嵌入式或控制领域的轻量级故障监测。

## 目标读者
- 嵌入式工程师、固件开发者
- 需要将简单阈值/迟滞式诊断逻辑集成到项目中的开发者

## 文件列表与说明
- `Dtc_cfg.h`
  - DTC 配置结构和通道类型定义（`DtcChannel_t`、阈值/迟滞配置等）。
  - 定义了故障/状态枚举、配置联合体及外部配置数组 `extern DtcChannel_t DtcChannelCfg[DTC_NUM];`。

- `Dtc_cfg.c`
  - 模块的配置实现示例，包含 `DtcChannelCfg` 数组的初始化示例（一个通道）。
  - 提供了 `Dtc_OutOfRange(DtcChannel_t* chan)` 函数：根据配置的范围类型（上限/下限/区间）和迟滞判断当前值是否为故障，并维护确认/清除计数与状态转换。

- `Dtc.h`
  - 定义诊断上报使用的简单状态枚举 `DtcFault_t`（`DTC_NO_FAULT`, `DTC_FAULT_DETECTED`）。

- `Dtc.c`
  - DTC 模块运行逻辑：包含 `Dtc_Init`、`Dtc_Clear`、`Dtc_Report`、`Dtc_Main`。
  - `Dtc_Init`：将 `DtcChannelCfg` 中的通道类型复制到 `DtcStatus`（目前实现是把 faultType 赋值给 DtcStatus，注意这与上报含义可能不同，见“注意事项”）。
  - `Dtc_Main`：对每个通道根据 `configType` 分发至对应的处理函数（目前只有 `DTC_CONFIG_OUT_OF_RANGE`），随后调用 `Dtc_Report` 更新 `DtcStatus`。

## 主要数据结构与接口契约（简明）
- DtcChannel_t (主要字段)
  - faultType: 故障类型枚举
  - configType: 配置类型（例如 `DTC_CONFIG_OUT_OF_RANGE`）
  - config.outOfRangeConfig: 包含 `pvalue`（指向被监测的实际值）、阈值与迟滞、rangeType（上限/下限/区间）
  - detectCountToConfirm / clearCountToClear: 确认/清除阈值计数
  - detectCounter / clearCounter: 运行时计数器
  - state: 动态状态（`DTC_STATE_NO_FAULT` / `PENDING` / `CONFIRMED`）

- 重要函数
  - void Dtc_Init(void)
    - 输入：无（使用全局 `DtcChannelCfg`）
    - 输出：初始化 `DtcStatus` 数组
    - 错误模式：当前实现将 `chan[i].faultType` 直接赋值给 `DtcStatus[i]`，可能不是预期的故障/无故障布尔上报（参见注意）。

  - void Dtc_Main(void)
    - 输入：全局配置 `DtcChannelCfg`（包含 `pvalue` 指针，应指向实时测量值）
    - 输出：更新 `DtcChannelCfg[].state` 与 `DtcStatus[]`
    - 调用频率：应在定时任务/循环中周期性调用

  - void Dtc_OutOfRange(DtcChannel_t *chan)
    - 输入：单通道配置
    - 输出：更新通道的计数器与 `state`
    - 行为：基于 rangeType 和阈值与迟滞来判断是否发生故障，并做计数确认/清除

  - void Dtc_Clear(DtcFaultType_t dtcId)
    - 清除指定通道计数并复位状态为 `DTC_STATE_NO_FAULT`。

  - void Dtc_Report(DtcFaultType_t dtcId)
    - 将通道 state 映射为 `DtcStatus[dtcId]`（已实现：CONFIRMED -> DTC_FAULT_DETECTED，否则 DTC_NO_FAULT）

## 使用方式示例
1. 在系统初始化阶段，确保为每个 `DtcChannelCfg[i].config.outOfRangeConfig.pvalue` 指向合法的监测变量地址（例如电压采样变量）。
2. 调用 `Dtc_Init()` 初始化状态。
3. 在周期性任务（例如 10ms 或按需）中调用 `Dtc_Main()`，模块会更新通道内部状态并填充 `DtcStatus[]`。
4. 读取 `DtcStatus[i]` 得到是否检测到故障。
5. 可通过 `Dtc_Clear(i)` 清除指定故障通道的状态计数。

示例伪代码：

```c
// ...existing code...
uint32_t measuredVoltage = ReadVoltage();
DtcChannelCfg[0].config.outOfRangeConfig.pvalue = &measuredVoltage;
Dtc_Init();
while (1) {
    measuredVoltage = ReadVoltage();
    // pvalue 指向的地址应实时更新对应变量
    Dtc_Main();
    if (DtcStatus[0] == DTC_FAULT_DETECTED) {
        // 执行故障处理
    }
}
```

## 注意事项与建议改进
- `Dtc_cfg.c` 的 `Dtc_OutOfRange` 从 `pvalue` 读取值时使用 `uint32_t value = *(chan->config.outOfRangeConfig.pvalue);`，确保 `pvalue` 不为 NULL 并且指向正确的字宽（uint32_t）。
- `Dtc.c` 中 `Dtc_Init` 目前把 `chan[i].faultType` 赋给 `DtcStatus[i]`，这看起来像是把故障类型覆盖为状态值，建议调整为：初始化 `DtcStatus[i] = DTC_NO_FAULT;` 或基于 `chan[i].state` 映射。
- `Dtc.h` 包含了 `Dtc_cfg.h`，而 `Dtc_cfg.h` 又包含 `Dtc.h`，存在头文件互相包含的潜在循环依赖风险。当前代码通过包含顺序可编译，但建议调整：将共用类型拆分到一个独立的头文件，或使用前向声明减少耦合。
- 对于多字节或浮点测量值，若使用不同数据宽度，请修改配置结构以支持不同类型或使用 void* 并添加类型字段。
- 当前实现中只有“越限”配置，若需添加更多诊断类型（如短路/断路），可扩展 `DtcConfig_t` 和处理分发逻辑。

## 验证与测试建议
- 单元测试：为 `Dtc_OutOfRange` 编写单元测试，覆盖上行越限、下行越限、区间越限、迟滞边界、计数确认/清除等场景。
- 集成测试：在目标板或仿真中，模拟测量值变化并观察 `DtcStatus` 的变化与去抖效果。

## 已知限制
- 代码示例中只配置了一个通道（数组大小受 `DTC_NUM` 控制）。
- 对线程安全/中断上下文的保护未实现，若在中断与主循环间共享 `pvalue` 或 `DtcStatus`，请添加适当的互斥/原子操作。

## 联系与贡献
如需改进此模块或提交 issue/PR，请在仓库中创建相应条目或联系维护者。

---
说明：本 README 基于当前仓库中的 `Dtc_cfg.c`、`Dtc_cfg.h`、`Dtc.c`、`Dtc.h` 文件内容生成。

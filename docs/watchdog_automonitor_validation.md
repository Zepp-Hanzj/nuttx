# watchdog_automonitor 验证归档

本文档只记录已经执行的验证和可复核证据。没有日志或截图支持的项目统一标为“待验证”。

## 1. 环境

- NuttX：13.0.0
- MCU：STM32F407ZG（STM32F407ZG-P1 自定义板）
- 调试器：J-Link，SWD
- 实板配置：`stm32f407zg-p1:watchdog-notifier`
- 自动监控方式：`CONFIG_WATCHDOG_AUTOMONITOR_BY_CAPTURE=y`
- WWDG 超时：40 ms
- notifier：`CONFIG_WATCHDOG_TIMEOUT_NOTIFIER=y`
- simulator 配置：`sim:watchdog-notifier`

后续证据统一建议保存到：

```text
docs/validation-evidence/watchdog/
```

命名规则：`<编号>-<测试名>-<日期>.log`、`<编号>-<测试名>-<日期>.png`。

## 2. 已完成并归档的验证

### 2.1 STM32 capture、自动喂狗和 notifier

实板 UART 已观察到连续输出：

```text
WWDG capture=2640 monitor=2640 timeout=2640
WWDG capture=2666 monitor=2666 timeout=2666
WWDG capture=2692 monitor=2692 timeout=2692
...
WWDG capture=3443 monitor=3443 timeout=3443
```

结论：

- capture 计数持续递增；
- monitor 计数与 capture 一致；
- timeout notifier 计数与 capture 一致；
- 长时间运行未出现 HardFault 或异常复位；
- 已实际经过 WWDG EWI、capture、硬件 keepalive、`watchdog_automonitor_timeout()` 和 notifier 回调。

当前证据来自此前 UART 会话；正式提交前应将完整连续日志另存为：
`01-stm32-capture-notifier-<日期>.log`，并截图保存起始、稳定运行和停止前三个时间点：
`01-stm32-capture-start-<日期>.png`、`01-stm32-capture-stable-<日期>.png`、
`01-stm32-capture-end-<日期>.png`。

### 2.2 simulator notifier 单元测试

构建确认只注册：

```text
Register: cmocka_driver_watchdog
```

运行结果：

```text
nsh> cmocka_driver_watchdog
[==========] tests: Running 1 test(s).
[ RUN      ] drivertest_watchdog_notifier
[       OK ] drivertest_watchdog_notifier
[==========] 1 test(s) run.
[  PASSED  ] 1 test(s).
```

覆盖内容：优先级顺序、重复注册、多次 timeout、注销、action 和 `data == NULL`。

正式证据应保存为：

- `02-simulator-build-<日期>.log`
- `02-simulator-notifier-<日期>.log`
- `02-simulator-notifier-<日期>.png`

### 2.3 STM32 WWDG 硬件复位源

通过 J-Link 关闭 EWI、启动 WWDG 且不执行 keepalive，随后读取：

```text
RCC_CSR = 0x5E000000
WWDGRSTF = 1
```

结论：已确认 MCU 由 WWDG 触发复位。

该证据仍缺少完整原始 J-Link 会话归档，应保存为：
`03-stm32-wwdg-reset-jlink-<日期>.log`，并截图保存复位前寄存器、复位后
`RCC_CSR` 两个画面：`03-stm32-wwdg-reset-before-<日期>.png`、
`03-stm32-wwdg-reset-after-<日期>.png`。

### 2.4 simulator automonitor 后端矩阵

在独立 simulator 配置中分别选择后端，并运行同一个
`cmocka_driver_watchdog` notifier 测试：

| 后端 | 预期 action | 编译 | 测试 |
|---|---:|---|---|
| `BY_CAPTURE` | 4 | 通过 | 通过 |
| `BY_WDOG` | 2 | 通过 | 通过 |
| `BY_WORKER` | 3 | 通过 | 通过 |
| `BY_ONESHOT` | 0 | 通过 | 通过 |
| `BY_TIMER` | 1 | 通过 | 通过 |
| `BY_IDLE` | 5 | 通过 | 通过 |

测试内部根据编译选项断言 action，因此每种模式的通过结果同时验证了
action 选择和 `data == NULL`。`BY_TIMER` 配置额外启用了 `TIMER`，`BY_IDLE`
配置额外启用了 `PM`；两者均在 simulator 中完成编译和运行测试。

### 2.5 simulator notifier 生命周期竞态

新增 `drivertest_watchdog_notifier_race`，由并发 pthread 反复注册、触发
timeout、注销 notifier，同时主测试任务持续触发 timeout。结果：

```text
[==========] tests: 2 test(s) run.
[  PASSED  ] 2 test(s).
```

完整输出保存在 `06-simulator-notifier-race-0714.log`。该测试覆盖 notifier
链并发交错和 callback 生命周期，但不等价于真实硬件 capture IRQ 挂起测试。

### 2.6 STM32 WWDG 生命周期边界

实板尝试通过 `WDIOC_STOP` 停止已启动的 STM32 WWDG，结果为失败；capture 和
notifier 计数继续递增，系统没有异常复位。该结果符合 STM32 WWDG 的硬件限制：
WWDG 启动后不能像普通可停止 watchdog 一样关闭。因此不能用 WWDG 的
`STOP -> START` 作为 automonitor 通用生命周期测试，相关临时测试代码已移除。
该限制和返回值应在 PR 说明中明确，通用生命周期竞态继续由 simulator notifier
压力测试覆盖。

### 2.7 capture stop-race 注入测试

使用临时验证入口在 simulator 中清除 capture upper-half 关联后注入一次
`arg == NULL` 的 capture 回调。回调安全返回 0，notifier 并发测试同时通过：

```text
[==========] tests: 2 test(s) run.
[  PASSED  ] 2 test(s).
```

证据保存在 `08-simulator-capture-stop-race-0714.log`。临时注入入口已移除，
正式代码仅保留空关联保护。

### 2.8 capture 上下文实例限制

NuttX watchdog upper-half 本身支持注册多个 watchdog 实例。但 capture 回调的
lower-half API 没有统一的实例参数；如果 lower-half 将 `arg` 原样传给回调，
当前实现按回调参数获得 upper-half，多个实例是安全的。对于 STM32 WWDG 这类
始终以 `arg == NULL` 调用回调的 lower-half，只能使用一个活动的 capture
automonitor 实例，代码中的全局指针仅作为该兼容路径的 fallback。该限制必须
在 PR 描述中明确，不能声称 NULL-argument lower-half 支持多个并发实例。

## 3. 已修复且已编译验证的问题

1. 避免测试任务调用 `WDIOC_START` 后关闭 automonitor。
2. 兼容 STM32 WWDG IRQ 传入 `arg == NULL` 的 capture 回调。
3. stop 与挂起 capture IRQ 竞态下增加空指针保护。
4. notifier 回调使用固定 action 和 `data == NULL`。
5. STM32 配置恢复后重新生成 `nuttx.bin`，构建成功。

## 4. 尚未完成的验证

以下项目不能写成“已通过”，必须逐项执行并保存证据。

### 4.1 完整 UART 复位闭环

需要在关闭 automonitor 的独立配置下记录：启动 NSH → 启动 watchdog → 不喂狗 →
重新出现 NSH banner。请保存完整 UART 原始日志和复位前后截图：

- `04-stm32-reset-cycle-<日期>.log`
- `04-stm32-reset-cycle-before-<日期>.png`
- `04-stm32-reset-cycle-after-<日期>.png`

### 4.2 notifier 中断上下文约束

当前 cmocka 测试在普通任务上下文调用 notifier，不能证明复杂 callback 在 ISR 中安全。
需要增加一个只执行原子计数的实板 callback，并证明计数递增且无阻塞、无 HardFault。
保存：`05-stm32-notifier-isr-<日期>.log` 和对应截图。

### 4.3 其他 automonitor 后端

尚未分别验证 `BY_WDOG`、`BY_WORKER`、`BY_ONESHOT`、`BY_TIMER`、`BY_IDLE` 的
action 和 notifier 时序。至少需要 simulator 构建检查，并对可运行后端执行一次测试。
每种后端独立保存日志，不能混用 capture 日志。

### 4.4 start/stop 和 notifier 生命周期竞态

尚未验证 stop 与挂起中断、start/stop 快速交替、callback 执行期间注销 notifier。
需要增加压力测试，保存完整日志；若出现复位、HardFault 或死锁，必须保留现场截图。

### 4.5 多 watchdog 实例

当前 capture 兼容逻辑使用单个 upper-half 关联指针。尚未验证两个 watchdog 同时启用
automonitor 的行为，提交 PR 前必须明确这是设计限制，或补充实例化实现和测试。

## 5. PR 前清理项

- 移除 `CONFIG_STM32F407ZG_P1_WWDG_TEST` 专用计数器和板级临时日志，或改为正式通用测试接口；
- 将 apps 中 `O_RDONLY` 兼容修正与 watchdog 测试改动分开评估；
- 对 Makefile 和 CMake 构建路径都执行一次“只注册 watchdog”检查；
- 完成第 4 节证据后，再更新本文档结论。

## 6. 当前结论

主路径已经有真实硬件和 simulator 证据，但完整 UART 复位闭环、ISR 约束、其他后端、竞态和多实例行为仍未完成。因此当前状态是“核心路径验证通过，PR 完整验证进行中”。

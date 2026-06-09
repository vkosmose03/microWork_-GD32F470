# Architecture

The firmware is layered so that all signal-processing logic is portable and
unit-testable, while everything device-specific is isolated in one place. This
directly addresses requirement #5 (modular, independent head/measurement/
calculation modules) and the optional unit-test requirement.

## Layers and dependency rules

```
            +-----------------------------------------------+
  app/      |  head module: orchestration only              |
            |  acquire -> convert -> analyse -> store -> log |
            +----------------------+------------------------+
                                   | (calls down only)
        +-------------+------------+------------+-------------+
calc/   | measure/    | config/    |            |             |
rms     | counts ->   | persisted  |            |             |
freq    | volts       | params     |            |             |
dfdt    |             | + CRC      |            |             |
harmonic|             |            |            |             |
        +------+------+-----+------+            |             |
               |            | (hal_nvm)         |             |
            +--v------------v-------------------v-------------v--+
  hal/      |  hal_adc   hal_uart   hal_nvm   hal_time          |
            |  abstract interfaces, no vendor dependencies      |
            +--------------------------+------------------------+
                                       | (implemented by)
            +--------------------------v------------------------+
  bsp/      |  gd32f470: TIMER+ADC+DMA, USART, flash EEPROM,     |
            |  SysTick, startup, linker, GD32F4xx library        |
            +---------------------------------------------------+
```

**Rules**

- `calc/`, `measure/`, `config/` are **pure C99** and include only standard
  headers and the abstract `hal/` headers. They never include vendor code, so
  they compile identically for the host and the target.
- `app/` depends on the modules and on `hal/` interfaces, never on `bsp/`.
- `bsp/gd32f470/` is the **only** place the GigaDevice firmware library is used.
- Dependencies point **downward** only; lower layers know nothing of upper ones.

## Why this enables testing

Because the calculation and configuration logic has no hardware dependency, the
host build links those modules into Unity test executables. The configuration
test substitutes a RAM-backed fake for `hal_nvm`. No emulator or hardware is
needed for CI.

## Build targets

| Target | When | Contents |
|---|---|---|
| `acmeter_core` | always | calc + measure + config (portable) |
| `test_*` | host build | Unity test executables (ctest) |
| `gd32_vendor` | cross build | GD32F4xx library + startup (non-strict warnings) |
| `gd32_bsp` | cross build | HAL implementations + syscalls (`-Werror`) |
| `firmware` | cross build | `app` + core + bsp → `.elf/.bin/.hex` |

The split is selected automatically: configuring with the
`cmake/gcc-arm-none-eabi.cmake` toolchain file enables the firmware targets;
otherwise the host/test build is configured.

## Data flow at runtime

1. `TIMER1` raises an update event at the sample rate; its TRGO triggers `ADC0`.
2. `DMA1` writes each conversion into a circular buffer holding two blocks.
3. On half/full-transfer, the DMA ISR hands the finished half-block to `app`.
4. `app` converts counts→volts (`measure`), runs `signal_calc`, and reports the
   results over UART. The other half-block fills concurrently.

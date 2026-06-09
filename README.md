# GD32F470 AC-Voltage Measurement Firmware

[![CI](https://github.com/OWNER/REPO/actions/workflows/ci.yml/badge.svg)](https://github.com/OWNER/REPO/actions/workflows/ci.yml)
[![Standard](https://img.shields.io/badge/C-C99-blue.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Embedded firmware for the **GigaDevice GD32F470xx** (Arm Cortex-M4F) that
measures an AC voltage in the **40–55 Hz** range on an ADC input (sampling
≤ 19200 Hz) and computes:

- **RMS** (СКЗ) of the AC component
- **Frequency** (over ≤ 3 periods)
- **Rate of frequency change, dF/dt** (over ≤ 6 periods)
- **Amplitude of the first harmonic** (Goertzel) — *optional requirement*

Results are reported over **UART**; runtime parameters are persisted in
**non-volatile memory**. The acquisition path is fully hardware-driven
(**Timer → ADC → DMA**) so the CPU only wakes once per sample block.

> Test assignment for АО «ПО Элтехника» — *Инженер-программист ВПО*.

---

## Highlights

| Requirement | How it is met |
|---|---|
| C99 | Enforced via `CMAKE_C_STANDARD 99` + `-Wpedantic`. |
| 40–55 Hz, Fs ≤ 19200 Hz | Timer-triggered ADC at a configurable rate (default 19200 Hz). |
| RMS / frequency / dF/dt | `src/calc/` — pure, unit-tested algorithms. |
| Max hardware offload | Timer TRGO → ADC → DMA double buffer; CPU processes whole blocks. |
| Modular, independent modules | `app` / `measure` / `calc` / `config` behind a thin `hal`. |
| Builds with **zero** warnings | `-Wall -Wextra -Wpedantic -Werror …` on all first-party code. |
| Git history | Small, conventional commits. |
| Unit tests* | Host `ctest` suites (Unity) for every algorithm. |
| 1st-harmonic amplitude* | Generalized Goertzel in `src/calc/harmonic.c`. |
| Config in NVM* | CRC-protected record in flash-emulated EEPROM. |
| UART output* | `hal_uart` + formatted report in the app. |

---

## Architecture

A platform-independent core sits behind a thin hardware-abstraction layer; the
vendor library is confined to the board-support package. This makes the
calculation/measurement logic portable and unit-testable on a host PC.

```
app (head)  ──orchestration: acquire → convert → analyse → store → report
   │
   ├── measure   raw ADC counts → calibrated volts        (portable)
   ├── calc      rms · freq · dfdt · harmonic · aggregate (portable, tested)
   ├── config    persisted, CRC-checked parameters        (portable, tested)
   │
   └── hal/      hal_adc · hal_uart · hal_nvm · hal_time   (abstract interfaces)
          │
          └── bsp/gd32f470   Timer+ADC+DMA · USART+DMA · flash EEPROM · startup
                             (the ONLY place the GD32F4xx library is used)
```

See [docs/architecture.md](docs/architecture.md), and the algorithm derivations
in [docs/algorithms.md](docs/algorithms.md).

---

## Building

### Host unit tests

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

### Firmware (GD32F470 target)

Requires the GNU Arm Embedded toolchain (`arm-none-eabi-gcc`).

```bash
cmake -S . -B build-fw -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cmake --build build-fw
# -> build-fw/firmware.elf, firmware.bin, firmware.hex (+ size report)
```

---

## Continuous integration

GitHub Actions ([.github/workflows/ci.yml](.github/workflows/ci.yml)) runs on
every push/PR:

1. **Host unit tests** + gcovr coverage.
2. **Firmware build** with `arm-none-eabi-gcc` (`-Werror`), uploads `.elf/.bin/.hex`.
3. **clang-format** style check.
4. **cppcheck** static analysis.

> Replace `OWNER/REPO` in the badge URLs once the repository is pushed, and
> enable branch protection so CI must pass before merging to `main`.

---

## Configuration

Parameters live in a CRC-protected record in NVM (defaults used if absent/invalid):

| Parameter | Default | Range |
|---|---|---|
| `sample_rate_hz` | 19200 | 1 … 19200 |
| `freq_periods` | 3 | 1 … 3 |
| `dfdt_periods` | 6 | 2 … 6 |
| `uart_baud` | 115200 | > 0 |
| `adc_scale` / `adc_offset` | 3.3 V / 4095, 1.65 V | calibration |

---

## License

MIT — see [LICENSE](LICENSE). The vendored GigaDevice/CMSIS sources keep their
own licenses.

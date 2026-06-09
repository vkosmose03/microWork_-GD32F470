# Hardware notes (GD32F470)

This describes the peripheral configuration in `bsp/gd32f470/`. The target is a
GD32F470ZGT6 (Cortex-M4F, 1024 KB flash, 192 KB SRAM); other GD32F470 variants
work with adjustments to the linker script.

## Clock tree

`SystemInit()` (from the GD32F4xx library) configures the PLL from the external
crystal before `main()`. The drivers do not assume a fixed core frequency:

- The sample timer reads `rcu_clock_freq_get(CK_APB1)` and uses `2 × PCLK1` as
  the timer clock (the standard GD32F4 timer clocking when the APB1 prescaler is
  not 1).
- `SysTick` is derived from `SystemCoreClock` for a 1 ms tick.

If your board uses a crystal other than the library default, set `HXTAL_VALUE`
accordingly (e.g. as a compile definition) so `SystemInit` produces the intended
clock.

## Pin / peripheral map

| Function | Peripheral | Pin(s) | Notes |
|---|---|---|---|
| Analog input | ADC0 channel 0 | PA0 | channels 0–7 → PA0–PA7 |
| Sample trigger | TIMER1 | — | update event → TRGO |
| Sample transfer | DMA1 ch0, sub-periph 0 | — | circular, 16-bit |
| UART out | USART0 | PA9 (TX), PA10 (RX) | AF7, default 115200 8N1 |
| Config storage | FMC | sector 11 @ 0x080E0000 | flash-emulated EEPROM |
| Time base | SysTick | — | 1 kHz |

## Acquisition chain (hardware-offloaded)

```
TIMER1 update @ Fs ──TRGO──▶ ADC0 (1 conversion) ──▶ DMA1 ch0 ──▶ circular buffer
                                                                   │ HTF / FTF IRQ
                                                                   ▼
                                                      app processes the half-block
```

- ADC: 12-bit, right-aligned, single routine channel, sample time 15 cycles,
  external rising-edge trigger from `TIMER1_TRGO`. ADC clock = PCLK2/8 to stay
  in spec.
- DMA: single-data mode, peripheral→memory, 16-bit, circular over `2 × block`
  samples; half- and full-transfer interrupts deliver each block.
- The CPU runs only during block processing; acquisition is autonomous.

## Configuration in NVM

The configuration record (magic + version + size + params + CRC32) is stored at
the start of flash **sector 11** (0x080E0000, 128 KB). `hal_nvm_erase()` erases
the sector; `hal_nvm_write()` programs 32-bit words. On boot, `config_load()`
validates magic/version/size/CRC and falls back to defaults if anything fails.

> Note: linking the configuration sector means application code must stay below
> 0x080E0000 (the default image is ~13 KB, so there is ample room).

## UART output format

One line per processed block:

```
RMS=1.414 V  F=50.01 Hz  dF/dt=0.12 Hz/s  A1=2.001 V
```

Floats are formatted with a small fixed-point helper, so floating-point
`printf` is not linked. Invalid/unavailable fields print `---`.

## Adapting to another board

- Change the analog channel via `APP_ADC_CHANNEL` (and the matching GPIO) in
  `src/app/app.c` / `bsp/gd32f470/hal_adc_gd32.c`.
- Adjust flash/RAM sizes in `bsp/gd32f470/gd32f470zg_flash.ld`.
- Calibrate `adc_scale` / `adc_offset` (defaults: 3.3 V / 4095, 1.65 V bias),
  which are stored in NVM.

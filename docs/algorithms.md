# Algorithms

All estimators operate on a block of `N` float samples acquired at sample rate
`Fs`. They are implemented in `src/calc/` and validated by the host unit tests
against synthetic 40/50/55 Hz signals.

## Sampling

The signal of interest is 40–55 Hz; the sample rate must not exceed 19200 Hz
(requirement #2). The default is `Fs = 19200 Hz`, which gives:

| Signal | Samples per period |
|---|---|
| 40 Hz | 480 |
| 50 Hz | 384 |
| 55 Hz | ~349 |

One processing block is `3840` samples (0.2 s at 19200 Hz). That is ≥ 6 periods
even at the lowest frequency (6 × 480 = 2880), so a single block is enough for
the dF/dt estimate over 6 periods.

## RMS (СКЗ) — `rms.c`

The ADC front-end biases the AC signal to mid-scale, so the DC component is
removed before computing the RMS of the AC part:

```
mean   = (1/N) Σ x[i]
rms_ac = sqrt( (1/N) Σ x[i]² − mean² )
```

Computed in a single pass. For best accuracy the block should span an integer
number of periods; the small leakage over 0.2 s of mains-like signal is
negligible.

## Frequency — `freq.c`

Upward (negative→positive) zero crossings of the DC-removed signal are located.
Each crossing time is refined to sub-sample resolution by linear interpolation
between the two straddling samples `x[i-1] < 0 ≤ x[i]`:

```
t_cross = (i-1) + ( −x[i-1] / (x[i] − x[i-1]) )      [in samples]
```

With `M` crossings spanning `M−1` periods (capped at `max_periods`, ≤ 3 per
requirement #3.2):

```
period_samples = (t_cross[M-1] − t_cross[0]) / (M−1)
frequency      = Fs / period_samples
```

Zero-crossing timing is robust and cheap for a clean sinusoid and needs no FFT.

## Rate of frequency change dF/dt — `dfdt.c`

Over a window of up to `max_periods` periods (≤ 6, requirement #3.3) the
per-period instantaneous frequency is computed from consecutive crossings:

```
f_k = Fs / (t_cross[k+1] − t_cross[k])      at time t_k = ½(t_cross[k]+t_cross[k+1])/Fs
```

A linear least-squares line is fitted to the `(t_k, f_k)` points; its slope is
dF/dt:

```
slope = ( P·Σ t·f − Σt·Σf ) / ( P·Σ t² − (Σt)² )
```

where `P` is the number of period points. The fitted line also yields the
start/end frequencies of the window. Using a regression over several periods is
more noise-tolerant than differencing two endpoint estimates.

## First-harmonic amplitude — `harmonic.c`

The amplitude of the fundamental (optional requirement #2) is found with the
**generalized Goertzel** algorithm, a single-bin DFT evaluated at the measured
fundamental `f0` (not restricted to an FFT bin):

```
ω      = 2π·f0 / Fs
coeff  = 2·cos ω
s[n]   = x[n] + coeff·s[n-1] − s[n-2]          (DC removed first)
Re     = s[N-1] − s[N-2]·cos ω
Im     = s[N-2]·sin ω
A1     = 2·√(Re² + Im²) / N                     (peak amplitude)
```

This costs one multiply-add per sample — far cheaper than a full FFT — and is
accurate even when `f0` lies between bins.

## Aggregation — `signal_calc.c`

`signal_calc_process()` runs the four estimators over one block and returns a
single result struct with per-field validity flags, so a partially analysable
block (e.g. too few crossings for dF/dt) still reports what it can.

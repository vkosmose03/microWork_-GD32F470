/**
 * @file  rms.h
 * @brief Root-mean-square (СКЗ) of a sampled signal.
 *
 * Pure C99, no hardware or vendor dependencies, so it is fully unit testable
 * on the host. All functions operate on a buffer of @p n float samples.
 */
#ifndef CALC_RMS_H
#define CALC_RMS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Arithmetic mean (DC component) of the buffer. Returns 0 if n == 0. */
float calc_mean(const float *x, size_t n);

/** @brief True RMS including any DC component: sqrt(mean(x^2)). */
float calc_rms(const float *x, size_t n);

/**
 * @brief RMS of the AC component only (DC removed): sqrt(mean(x^2) - mean(x)^2).
 *
 * This is the quantity wanted for an AC voltage measurement, since the ADC
 * front-end typically adds a DC bias to centre the waveform in its input range.
 * For best accuracy @p n should span an integer number of signal periods.
 */
float calc_rms_ac(const float *x, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* CALC_RMS_H */

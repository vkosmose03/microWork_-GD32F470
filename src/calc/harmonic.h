/**
 * @file  harmonic.h
 * @brief Amplitude of the fundamental (first harmonic) via the Goertzel filter.
 *
 * The generalized Goertzel algorithm evaluates a single DFT bin at an arbitrary
 * (non-integer) frequency, so it gives an accurate amplitude even when the
 * fundamental does not align with an FFT bin. It costs one multiply-add per
 * sample, far cheaper than a full FFT (req* #2: amplitude of the first
 * harmonic).
 *
 * Pure C99, host-testable.
 */
#ifndef CALC_HARMONIC_H
#define CALC_HARMONIC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float amplitude;  /**< Peak amplitude of the fundamental.        */
    float rms;        /**< RMS of the fundamental (= amplitude/√2).   */
    int valid;        /**< Non-zero if the estimate is meaningful.    */
} harmonic_result_t;

/**
 * @brief Amplitude of the component at @p f0_hz over @p n samples.
 *
 * @param x      Input samples (DC is removed internally).
 * @param n      Number of samples; should span an integer number of periods.
 * @param fs_hz  Sample rate in Hz.
 * @param f0_hz  Fundamental frequency in Hz (e.g. from calc_frequency).
 */
harmonic_result_t calc_harmonic1(const float *x, size_t n, float fs_hz, float f0_hz);

#ifdef __cplusplus
}
#endif

#endif /* CALC_HARMONIC_H */

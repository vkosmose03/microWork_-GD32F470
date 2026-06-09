/**
 * @file  freq.h
 * @brief Fundamental frequency estimation by zero-crossing detection.
 *
 * The signal mean (DC bias) is removed internally, then upward
 * (negative-to-positive) zero crossings are located with linear interpolation
 * for sub-sample resolution. The frequency is derived from the average spacing
 * of those crossings over a bounded number of periods.
 *
 * Pure C99, host-testable.
 */
#ifndef CALC_FREQ_H
#define CALC_FREQ_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float frequency_hz;    /**< Estimated fundamental frequency, 0 if invalid. */
    unsigned periods_used; /**< Number of whole periods the estimate spans.    */
    int valid;             /**< Non-zero if a frequency could be determined.   */
} freq_result_t;

/**
 * @brief Locate upward zero crossings of the DC-removed signal.
 *
 * @param x         Input samples.
 * @param n         Number of samples.
 * @param out_times Output array of crossing positions in fractional samples.
 * @param max_out   Capacity of @p out_times.
 * @return Number of crossings written (<= max_out).
 */
size_t calc_upward_crossings(const float *x, size_t n, float *out_times, size_t max_out);

/**
 * @brief Estimate frequency over at most @p max_periods periods (req #3.2: <= 3).
 *
 * @param x           Input samples.
 * @param n           Number of samples.
 * @param fs_hz       Sample rate in Hz.
 * @param max_periods Upper bound on periods used (e.g. 3).
 */
freq_result_t calc_frequency(const float *x, size_t n, float fs_hz, unsigned max_periods);

#ifdef __cplusplus
}
#endif

#endif /* CALC_FREQ_H */

/**
 * @file  dfdt.h
 * @brief Rate of frequency change (dF/dt) of a sampled signal.
 *
 * Upward zero crossings are located over a bounded number of periods; the
 * instantaneous frequency of each period is computed and a linear least-squares
 * line is fitted to frequency-versus-time. The slope of that line is dF/dt.
 *
 * Pure C99, host-testable.
 */
#ifndef CALC_DFDT_H
#define CALC_DFDT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float dfdt_hz_per_s;   /**< Slope of frequency vs time (Hz/s).            */
    float f_start_hz;      /**< Fitted frequency at the first period.         */
    float f_end_hz;        /**< Fitted frequency at the last period.          */
    unsigned periods_used; /**< Number of per-period frequency points fitted. */
    int valid;             /**< Non-zero if a slope could be determined.      */
} dfdt_result_t;

/**
 * @brief Estimate dF/dt over at most @p max_periods periods (req #3.3: <= 6).
 *
 * @param x           Input samples.
 * @param n           Number of samples.
 * @param fs_hz       Sample rate in Hz.
 * @param max_periods Upper bound on periods used (e.g. 6).
 */
dfdt_result_t calc_dfdt(const float *x, size_t n, float fs_hz, unsigned max_periods);

#ifdef __cplusplus
}
#endif

#endif /* CALC_DFDT_H */

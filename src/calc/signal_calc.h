/**
 * @file  signal_calc.h
 * @brief Aggregated signal analysis: RMS, frequency, dF/dt and 1st harmonic.
 *
 * This is the public entry point of the calculation module. It runs all the
 * individual estimators over one sample block and returns a single result
 * structure. It is deliberately free of any hardware dependency so the whole
 * analysis pipeline can be exercised by the host unit tests.
 */
#ifndef CALC_SIGNAL_CALC_H
#define CALC_SIGNAL_CALC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Tunable analysis parameters (mirrors the persisted configuration). */
typedef struct {
    float fs_hz;           /**< Sample rate of the block (Hz).               */
    unsigned freq_periods; /**< Periods for frequency estimation (<= 3).      */
    unsigned dfdt_periods; /**< Periods for dF/dt estimation (<= 6).          */
} signal_calc_params_t;

/** Combined analysis result for one sample block. */
typedef struct {
    float rms;                 /**< AC RMS (СКЗ).                            */
    float frequency_hz;        /**< Fundamental frequency.                   */
    float dfdt_hz_per_s;       /**< Rate of frequency change.                */
    float harmonic1_amplitude; /**< Peak amplitude of the fundamental.       */

    int rms_valid;
    int freq_valid;
    int dfdt_valid;
    int harmonic_valid;
} signal_result_t;

/**
 * @brief Run the full analysis pipeline over a block of float samples.
 *
 * @param x Samples in physical units (e.g. volts). DC bias is handled inside.
 * @param n Number of samples.
 * @param p Analysis parameters (must not be NULL).
 */
signal_result_t signal_calc_process(const float *x, size_t n, const signal_calc_params_t *p);

#ifdef __cplusplus
}
#endif

#endif /* CALC_SIGNAL_CALC_H */

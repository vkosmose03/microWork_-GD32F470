/**
 * @file  measure.h
 * @brief Measurement module: turn raw ADC blocks into calibrated samples.
 *
 * The measurement module is intentionally independent of the acquisition
 * hardware (req #5). The board layer fills a buffer of raw ADC counts via DMA
 * and hands it here; this module applies the calibration (counts -> volts) and
 * produces a float buffer ready for the calculation module. Because it touches
 * no vendor code, it is fully host-testable.
 */
#ifndef MEASURE_H
#define MEASURE_H

#include "hal/hal_adc.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ADC calibration: physical value = (count - offset_counts) * scale. */
typedef struct {
    float scale;         /**< Volts per ADC least-significant bit.     */
    float offset_counts; /**< ADC count corresponding to 0 V (DC bias). */
} measure_calib_t;

/** Convert a single raw ADC count to a calibrated value (volts). */
float measure_sample_to_volts(adc_sample_t count, const measure_calib_t *cal);

/**
 * @brief Convert a block of raw ADC counts to calibrated volts.
 *
 * @param raw  Input buffer of @p n raw ADC counts.
 * @param n    Number of samples.
 * @param cal  Calibration parameters.
 * @param out  Output buffer of at least @p n floats (volts).
 */
void measure_convert_block(const adc_sample_t *raw, size_t n, const measure_calib_t *cal,
                           float *out);

#ifdef __cplusplus
}
#endif

#endif /* MEASURE_H */

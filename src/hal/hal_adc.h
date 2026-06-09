/**
 * @file  hal_adc.h
 * @brief Abstract ADC acquisition interface.
 *
 * The ADC is driven entirely by hardware: a timer triggers conversions at a
 * fixed sample rate and DMA streams the results into a buffer. The CPU is only
 * notified, through @ref hal_adc_block_cb_t, once a full block of samples is
 * ready. This keeps the CPU load minimal (task requirement #4).
 *
 * This header is platform independent: it contains no vendor dependencies so
 * that the application and measurement logic can be compiled and unit tested
 * on the host.
 */
#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Raw ADC sample: 12-bit value, right-aligned in a 16-bit word. */
typedef uint16_t adc_sample_t;

/**
 * @brief Block-ready callback, invoked from DMA interrupt context.
 * @param samples Pointer to the freshly filled half of the DMA buffer.
 * @param count   Number of samples in the block.
 * @param ctx     User context passed in @ref hal_adc_config_t.
 */
typedef void (*hal_adc_block_cb_t)(const adc_sample_t *samples, size_t count, void *ctx);

typedef struct {
    uint32_t sample_rate_hz;     /**< Requested sampling frequency (must be <= 19200). */
    size_t block_samples;        /**< Samples per block delivered to the callback.     */
    uint8_t channel;             /**< ADC input channel index.                          */
    hal_adc_block_cb_t on_block; /**< Called once per filled block (may be NULL).        */
    void *ctx;                   /**< Opaque user context for the callback.             */
} hal_adc_config_t;

/** @return 0 on success, negative on error. */
int hal_adc_init(const hal_adc_config_t *cfg);
int hal_adc_start(void);
int hal_adc_stop(void);

/**
 * @brief Actual sample rate after the timer prescaler/period rounding.
 *
 * The realizable rate may differ slightly from the requested one; the
 * calculation modules use this value as their time base.
 */
uint32_t hal_adc_actual_rate_hz(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ADC_H */

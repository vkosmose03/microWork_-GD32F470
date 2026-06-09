/**
 * @file  config.h
 * @brief Persistent, validated runtime configuration (req* #3).
 *
 * Configurable parameters (sampling rate, number of periods used by the
 * estimators, UART baud rate, ADC calibration) are stored in non-volatile
 * memory through the @ref hal_nvm.h interface. A record header with a magic
 * number, version and CRC32 protects against corrupted or uninitialised
 * storage; if validation fails the safe defaults are used.
 *
 * The module is platform independent: on the host the NVM is a RAM stub, which
 * lets the load/save/validate logic be unit tested.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Hard limits derived from the task requirements. */
#define CONFIG_MAX_SAMPLE_RATE_HZ 19200u /**< req #2: Fs <= 19200 Hz.        */
#define CONFIG_MAX_FREQ_PERIODS   3u     /**< req #3.2: frequency over <= 3.  */
#define CONFIG_MAX_DFDT_PERIODS   6u     /**< req #3.3: dF/dt over <= 6.      */
#define CONFIG_MIN_DFDT_PERIODS   2u

typedef struct {
    uint32_t sample_rate_hz; /**< Sampling frequency, <= CONFIG_MAX_SAMPLE_RATE_HZ. */
    uint16_t freq_periods;   /**< Periods used for frequency estimation.            */
    uint16_t dfdt_periods;   /**< Periods used for dF/dt estimation.                */
    uint32_t uart_baud;      /**< UART baud rate for result output.                 */
    float adc_scale;         /**< Volts per ADC least-significant bit.              */
    float adc_offset;        /**< DC offset in volts (subtracted before analysis).  */
} config_params_t;

/** Populate @p p with built-in safe defaults. */
void config_set_defaults(config_params_t *p);

/**
 * @brief Clamp out-of-range fields to valid values.
 * @return 0 if @p p was already valid, non-zero if any field was clamped.
 */
int config_validate(config_params_t *p);

/**
 * @brief Load configuration from NVM.
 *
 * On a CRC/magic/version mismatch, @p p is filled with defaults instead.
 * @return 0 on a successful, validated load; negative if defaults were used.
 */
int config_load(config_params_t *p);

/**
 * @brief Validate and persist configuration to NVM.
 * @return 0 on success, negative on an NVM error.
 */
int config_save(const config_params_t *p);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

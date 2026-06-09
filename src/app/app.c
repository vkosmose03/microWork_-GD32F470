#include "app/app.h"

#include "calc/signal_calc.h"
#include "config/config.h"
#include "hal/hal_adc.h"
#include "hal/hal_uart.h"
#include "measure/measure.h"

#include <stddef.h>
#include <stdint.h>

/* One processing block. Must cover >= 6 periods at the lowest frequency (40 Hz):
 * 6 * (19200 / 40) = 2880 samples. 3840 (0.2 s at 19200 Hz) leaves margin. */
#define APP_BLOCK_SAMPLES 3840u
#define APP_ADC_CHANNEL   0u /* ADC0 channel 0 -> PA0 */

static float s_volts[APP_BLOCK_SAMPLES];

/* Shared with the ADC block callback (ISR context). */
static volatile int s_block_ready;
static const adc_sample_t *volatile s_block_ptr;
static volatile size_t s_block_len;

static void on_adc_block(const adc_sample_t *samples, size_t count, void *ctx) {
    (void) ctx;
    /* Keep the ISR short: hand the buffer to the main loop. The other half of
     * the double buffer keeps filling while the main loop processes this one. */
    s_block_ptr = samples;
    s_block_len = count;
    s_block_ready = 1;
}

/* Minimal fixed-point float formatter (avoids pulling in floating-point printf). */
static void format_fixed(char *dst, size_t cap, float value, unsigned decimals) {
    if (cap == 0u) {
        return;
    }
    size_t pos = 0u;
    if (value < 0.0f) {
        if (pos < (cap - 1u)) {
            dst[pos++] = '-';
        }
        value = -value;
    }

    float scale = 1.0f;
    for (unsigned i = 0u; i < decimals; ++i) {
        scale *= 10.0f;
    }
    const uint32_t scaled = (uint32_t) ((value * scale) + 0.5f);

    uint32_t modulus = 1u;
    for (unsigned i = 0u; i < decimals; ++i) {
        modulus *= 10u;
    }
    uint32_t int_part = scaled / modulus;
    const uint32_t frac_part = scaled % modulus;

    char tmp[12];
    int t = 0;
    if (int_part == 0u) {
        tmp[t++] = '0';
    } else {
        while (int_part > 0u && t < (int) sizeof(tmp)) {
            tmp[t++] = (char) ('0' + (int_part % 10u));
            int_part /= 10u;
        }
    }
    while (t > 0 && pos < (cap - 1u)) {
        dst[pos++] = tmp[--t];
    }

    if (decimals > 0u) {
        if (pos < (cap - 1u)) {
            dst[pos++] = '.';
        }
        uint32_t divisor = 1u;
        for (unsigned i = 0u; (i + 1u) < decimals; ++i) {
            divisor *= 10u;
        }
        for (unsigned i = 0u; i < decimals; ++i) {
            const uint32_t digit = (divisor != 0u) ? ((frac_part / divisor) % 10u) : 0u;
            if (pos < (cap - 1u)) {
                dst[pos++] = (char) ('0' + digit);
            }
            divisor /= 10u;
        }
    }
    dst[pos] = '\0';
}

static void emit_value(const char *label, int valid, float value, unsigned decimals,
                       const char *unit) {
    char num[24];
    hal_uart_write_str(label);
    if (valid) {
        format_fixed(num, sizeof(num), value, decimals);
        hal_uart_write_str(num);
    } else {
        hal_uart_write_str("---");
    }
    hal_uart_write_str(unit);
}

static void report(const signal_result_t *r) {
    emit_value("RMS=", r->rms_valid, r->rms, 3u, " V  ");
    emit_value("F=", r->freq_valid, r->frequency_hz, 2u, " Hz  ");
    emit_value("dF/dt=", r->dfdt_valid, r->dfdt_hz_per_s, 2u, " Hz/s  ");
    emit_value("A1=", r->harmonic_valid, r->harmonic1_amplitude, 3u, " V");
    hal_uart_write_str("\r\n");
}

void app_run(void) {
    config_params_t cfg;
    (void) config_load(&cfg); /* defaults applied on first boot / corruption */

    hal_uart_init(cfg.uart_baud);
    hal_uart_write_str("GD32F470 AC meter\r\n");

    const measure_calib_t calib = {cfg.adc_scale, cfg.adc_offset / cfg.adc_scale};

    const hal_adc_config_t adc_cfg = {
        cfg.sample_rate_hz, APP_BLOCK_SAMPLES, APP_ADC_CHANNEL, on_adc_block, NULL};
    if (hal_adc_init(&adc_cfg) != 0) {
        hal_uart_write_str("ADC init failed\r\n");
        for (;;) {
        }
    }

    signal_calc_params_t params;
    params.fs_hz = (float) hal_adc_actual_rate_hz();
    params.freq_periods = cfg.freq_periods;
    params.dfdt_periods = cfg.dfdt_periods;

    hal_adc_start();

    for (;;) {
        if (s_block_ready) {
            s_block_ready = 0;
            const adc_sample_t *block = s_block_ptr;
            const size_t len = s_block_len;

            measure_convert_block(block, len, &calib, s_volts);
            const signal_result_t r = signal_calc_process(s_volts, len, &params);
            report(&r);
        }
    }
}

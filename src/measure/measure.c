#include "measure/measure.h"

#include <stddef.h>

float measure_sample_to_volts(adc_sample_t count, const measure_calib_t *cal) {
    if (cal == NULL) {
        return 0.0f;
    }
    return ((float) count - cal->offset_counts) * cal->scale;
}

void measure_convert_block(const adc_sample_t *raw, size_t n, const measure_calib_t *cal,
                           float *out) {
    if (raw == NULL || cal == NULL || out == NULL) {
        return;
    }
    for (size_t i = 0u; i < n; ++i) {
        out[i] = ((float) raw[i] - cal->offset_counts) * cal->scale;
    }
}

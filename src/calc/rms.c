#include "calc/rms.h"

#include <math.h>

float calc_mean(const float *x, size_t n) {
    if (x == NULL || n == 0u) {
        return 0.0f;
    }
    float sum = 0.0f;
    for (size_t i = 0u; i < n; ++i) {
        sum += x[i];
    }
    return sum / (float) n;
}

float calc_rms(const float *x, size_t n) {
    if (x == NULL || n == 0u) {
        return 0.0f;
    }
    float sum_sq = 0.0f;
    for (size_t i = 0u; i < n; ++i) {
        sum_sq += x[i] * x[i];
    }
    return sqrtf(sum_sq / (float) n);
}

float calc_rms_ac(const float *x, size_t n) {
    if (x == NULL || n == 0u) {
        return 0.0f;
    }
    float sum = 0.0f;
    float sum_sq = 0.0f;
    for (size_t i = 0u; i < n; ++i) {
        sum += x[i];
        sum_sq += x[i] * x[i];
    }
    const float inv_n = 1.0f / (float) n;
    const float mean = sum * inv_n;
    float variance = (sum_sq * inv_n) - (mean * mean);
    /* Guard against a tiny negative value from floating-point round-off. */
    if (variance < 0.0f) {
        variance = 0.0f;
    }
    return sqrtf(variance);
}

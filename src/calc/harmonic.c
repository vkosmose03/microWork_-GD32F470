#include "calc/harmonic.h"
#include "calc/rms.h"

#include <math.h>

#define CALC_PI 3.14159265358979323846f
#define CALC_SQRT2 1.41421356237309504880f

harmonic_result_t calc_harmonic1(const float *x, size_t n, float fs_hz, float f0_hz) {
    harmonic_result_t r = {0.0f, 0.0f, 0};

    if (x == NULL || n < 2u || fs_hz <= 0.0f || f0_hz <= 0.0f || f0_hz >= (0.5f * fs_hz)) {
        return r;
    }

    const float mean = calc_mean(x, n);
    const float omega = (2.0f * CALC_PI * f0_hz) / fs_hz;
    const float cos_w = cosf(omega);
    const float sin_w = sinf(omega);
    const float coeff = 2.0f * cos_w;

    float s1 = 0.0f;
    float s2 = 0.0f;
    for (size_t i = 0u; i < n; ++i) {
        const float s0 = (x[i] - mean) + (coeff * s1) - s2;
        s2 = s1;
        s1 = s0;
    }

    const float real = s1 - (s2 * cos_w);
    const float imag = s2 * sin_w;
    const float magnitude = sqrtf((real * real) + (imag * imag));

    r.amplitude = (2.0f * magnitude) / (float) n;
    r.rms = r.amplitude / CALC_SQRT2;
    r.valid = 1;
    return r;
}

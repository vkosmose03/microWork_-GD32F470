#include "calc/freq.h"
#include "calc/rms.h"

#include <stddef.h>

size_t calc_upward_crossings(const float *x, size_t n, float *out_times, size_t max_out) {
    if (x == NULL || out_times == NULL || n < 2u || max_out == 0u) {
        return 0u;
    }

    const float mean = calc_mean(x, n);
    size_t found = 0u;
    float prev = x[0] - mean;

    for (size_t i = 1u; i < n && found < max_out; ++i) {
        const float cur = x[i] - mean;
        /* Upward crossing: previous strictly below zero, current at/above zero. */
        if (prev < 0.0f && cur >= 0.0f) {
            const float delta = cur - prev;
            /* delta > 0 here, so the division is well defined. */
            const float frac = -prev / delta;
            out_times[found] = (float) (i - 1u) + frac;
            ++found;
        }
        prev = cur;
    }
    return found;
}

freq_result_t calc_frequency(const float *x, size_t n, float fs_hz, unsigned max_periods) {
    freq_result_t r = {0.0f, 0u, 0};

    if (x == NULL || n < 2u || fs_hz <= 0.0f || max_periods == 0u) {
        return r;
    }

    /* We need at most (max_periods + 1) crossings to bound max_periods periods. */
    const size_t want = (size_t) max_periods + 1u;
    /* Cap the local buffer; 3-6 periods is all the task requires. */
    enum { MAX_CROSSINGS = 16 };
    float times[MAX_CROSSINGS];
    const size_t cap = (want < (size_t) MAX_CROSSINGS) ? want : (size_t) MAX_CROSSINGS;

    const size_t got = calc_upward_crossings(x, n, times, cap);
    if (got < 2u) {
        return r;
    }

    const unsigned periods = (unsigned) (got - 1u);
    const float span_samples = times[got - 1u] - times[0];
    if (span_samples <= 0.0f) {
        return r;
    }

    const float period_samples = span_samples / (float) periods;
    r.frequency_hz = fs_hz / period_samples;
    r.periods_used = periods;
    r.valid = 1;
    return r;
}

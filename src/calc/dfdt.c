#include "calc/dfdt.h"
#include "calc/freq.h"

#include <stddef.h>

dfdt_result_t calc_dfdt(const float *x, size_t n, float fs_hz, unsigned max_periods) {
    dfdt_result_t r = {0.0f, 0.0f, 0.0f, 0u, 0};

    if (x == NULL || n < 2u || fs_hz <= 0.0f || max_periods < 2u) {
        return r;
    }

    /* Need (max_periods + 1) crossings to bound max_periods periods. */
    enum { MAX_CROSSINGS = 16 };
    float times[MAX_CROSSINGS];
    size_t want = (size_t) max_periods + 1u;
    if (want > (size_t) MAX_CROSSINGS) {
        want = (size_t) MAX_CROSSINGS;
    }

    const size_t got = calc_upward_crossings(x, n, times, want);
    if (got < 3u) {
        /* Fewer than two full periods: cannot estimate a slope. */
        return r;
    }

    const unsigned points = (unsigned) (got - 1u);
    const float inv_fs = 1.0f / fs_hz;

    /* Accumulate sums for a linear least-squares fit of f(t). */
    float sum_t = 0.0f;
    float sum_f = 0.0f;
    float sum_tt = 0.0f;
    float sum_tf = 0.0f;

    for (unsigned k = 0u; k < points; ++k) {
        const float period_samples = times[k + 1u] - times[k];
        if (period_samples <= 0.0f) {
            return r;
        }
        const float f_k = fs_hz / period_samples;
        /* Time stamp at the midpoint of the period, in seconds. */
        const float t_k = 0.5f * (times[k] + times[k + 1u]) * inv_fs;

        sum_t += t_k;
        sum_f += f_k;
        sum_tt += t_k * t_k;
        sum_tf += t_k * f_k;
    }

    const float np = (float) points;
    const float denom = (np * sum_tt) - (sum_t * sum_t);
    if (denom == 0.0f) {
        return r;
    }

    const float slope = ((np * sum_tf) - (sum_t * sum_f)) / denom;
    const float mean_t = sum_t / np;
    const float mean_f = sum_f / np;
    const float intercept = mean_f - (slope * mean_t);

    /* Times of the first and last fitted points. */
    const float t_first = 0.5f * (times[0] + times[1]) * inv_fs;
    const float t_last = 0.5f * (times[got - 2u] + times[got - 1u]) * inv_fs;

    r.dfdt_hz_per_s = slope;
    r.f_start_hz = intercept + (slope * t_first);
    r.f_end_hz = intercept + (slope * t_last);
    r.periods_used = points;
    r.valid = 1;
    return r;
}

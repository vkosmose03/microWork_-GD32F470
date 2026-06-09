#include "calc/signal_calc.h"

#include "calc/dfdt.h"
#include "calc/freq.h"
#include "calc/harmonic.h"
#include "calc/rms.h"

#include <stddef.h>

signal_result_t signal_calc_process(const float *x, size_t n, const signal_calc_params_t *p) {
    signal_result_t out = {0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0};

    if (x == NULL || n == 0u || p == NULL || p->fs_hz <= 0.0f) {
        return out;
    }

    /* RMS of the AC component is always computable from the raw block. */
    out.rms = calc_rms_ac(x, n);
    out.rms_valid = 1;

    const freq_result_t f = calc_frequency(x, n, p->fs_hz, p->freq_periods);
    if (f.valid) {
        out.frequency_hz = f.frequency_hz;
        out.freq_valid = 1;

        /* The harmonic amplitude needs a fundamental frequency to lock onto. */
        const harmonic_result_t h = calc_harmonic1(x, n, p->fs_hz, f.frequency_hz);
        if (h.valid) {
            out.harmonic1_amplitude = h.amplitude;
            out.harmonic_valid = 1;
        }
    }

    const dfdt_result_t d = calc_dfdt(x, n, p->fs_hz, p->dfdt_periods);
    if (d.valid) {
        out.dfdt_hz_per_s = d.dfdt_hz_per_s;
        out.dfdt_valid = 1;
    }

    return out;
}

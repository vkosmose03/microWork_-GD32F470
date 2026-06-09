/**
 * @file  test_util.h
 * @brief Synthetic signal generators shared by the unit tests.
 */
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <math.h>
#include <stddef.h>

#ifndef TEST_TWO_PI
#define TEST_TWO_PI 6.283185307179586
#endif

/** Fill @p buf with a pure sine: offset + amp*sin(2*pi*f*t + phase). */
static inline void gen_sine(float *buf, size_t n, double fs, double freq, double amp,
                            double offset, double phase) {
    for (size_t i = 0u; i < n; ++i) {
        const double t = (double) i / fs;
        buf[i] = (float) (offset + amp * sin((TEST_TWO_PI * freq * t) + phase));
    }
}

/**
 * @brief Fill @p buf with a linear chirp whose instantaneous frequency sweeps
 *        from @p f0 to @p f1 over the whole buffer. dF/dt = (f1 - f0) / T.
 * @return The constant rate of frequency change (Hz/s).
 */
static inline double gen_chirp(float *buf, size_t n, double fs, double f0, double f1,
                               double amp, double offset) {
    const double duration = (double) n / fs;
    const double k = (f1 - f0) / duration; /* Hz per second */
    for (size_t i = 0u; i < n; ++i) {
        const double t = (double) i / fs;
        const double phase = TEST_TWO_PI * ((f0 * t) + (0.5 * k * t * t));
        buf[i] = (float) (offset + amp * sin(phase));
    }
    return k;
}

#endif /* TEST_UTIL_H */

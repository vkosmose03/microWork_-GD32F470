#include "calc/harmonic.h"
#include "test_util.h"
#include "unity.h"

#include <math.h>

void setUp(void) {
}
void tearDown(void) {
}

enum { N = 3840 };
static float buf[N];

static void test_harmonic_extracts_fundamental(void) {
    /* 2.0 V fundamental at 50 Hz plus a 0.5 V third harmonic and DC bias. */
    const double fs = 19200.0;
    for (size_t i = 0u; i < (size_t) N; ++i) {
        const double t = (double) i / fs;
        buf[i] =
            (float) (1.65 + 2.0 * sin(TEST_TWO_PI * 50.0 * t) + 0.5 * sin(TEST_TWO_PI * 150.0 * t));
    }
    const harmonic_result_t r = calc_harmonic1(buf, N, 19200.0f, 50.0f);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.03f, 2.0f, r.amplitude);
    TEST_ASSERT_FLOAT_WITHIN(0.03f, 2.0f / 1.41421356f, r.rms);
}

static void test_harmonic_rejects_bad_args(void) {
    gen_sine(buf, N, 19200.0, 50.0, 2.0, 1.65, 0.0);
    /* f0 above Nyquist must be rejected. */
    const harmonic_result_t r = calc_harmonic1(buf, N, 19200.0f, 20000.0f);
    TEST_ASSERT_FALSE(r.valid);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_harmonic_extracts_fundamental);
    RUN_TEST(test_harmonic_rejects_bad_args);
    return UNITY_END();
}

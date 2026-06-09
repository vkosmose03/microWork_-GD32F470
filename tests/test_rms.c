#include "calc/rms.h"
#include "test_util.h"
#include "unity.h"

void setUp(void) {
}
void tearDown(void) {
}

/* 10 whole periods of 50 Hz at 19200 Hz = exactly 3840 samples. */
enum { N = 3840 };
static float buf[N];

static void test_rms_ac_of_sine(void) {
    gen_sine(buf, N, 19200.0, 50.0, 2.0, 1.65, 0.0);
    /* RMS of a 2 V peak sine is 2/sqrt(2) ~= 1.41421. */
    TEST_ASSERT_FLOAT_WITHIN(0.005f, 1.41421356f, calc_rms_ac(buf, N));
}

static void test_rms_ac_rejects_dc_bias(void) {
    gen_sine(buf, N, 19200.0, 50.0, 2.0, 0.0, 0.0);
    const float no_bias = calc_rms_ac(buf, N);
    gen_sine(buf, N, 19200.0, 50.0, 2.0, 3.30, 0.0);
    const float with_bias = calc_rms_ac(buf, N);
    TEST_ASSERT_FLOAT_WITHIN(0.005f, no_bias, with_bias);
}

static void test_true_rms_includes_dc(void) {
    /* Constant 2.0 signal: true RMS = 2.0, AC RMS = 0. */
    for (size_t i = 0u; i < (size_t) N; ++i) {
        buf[i] = 2.0f;
    }
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, calc_rms(buf, N));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, calc_rms_ac(buf, N));
}

static void test_rms_handles_empty(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc_rms(NULL, 0u));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc_rms_ac(buf, 0u));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calc_mean(NULL, 0u));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_rms_ac_of_sine);
    RUN_TEST(test_rms_ac_rejects_dc_bias);
    RUN_TEST(test_true_rms_includes_dc);
    RUN_TEST(test_rms_handles_empty);
    return UNITY_END();
}

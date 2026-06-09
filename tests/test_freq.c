#include "calc/freq.h"
#include "test_util.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

enum { N = 3840 };
static float buf[N];

static void check_frequency(double f) {
    gen_sine(buf, N, 19200.0, f, 2.0, 1.65, 0.3);
    const freq_result_t r = calc_frequency(buf, N, 19200.0f, 3u);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_TRUE(r.periods_used >= 1u && r.periods_used <= 3u);
    TEST_ASSERT_FLOAT_WITHIN(0.15f, (float) f, r.frequency_hz);
}

static void test_freq_40hz(void) { check_frequency(40.0); }
static void test_freq_50hz(void) { check_frequency(50.0); }
static void test_freq_55hz(void) { check_frequency(55.0); }

static void test_freq_caps_periods(void) {
    gen_sine(buf, N, 19200.0, 50.0, 2.0, 1.65, 0.0);
    const freq_result_t r = calc_frequency(buf, N, 19200.0f, 3u);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_EQUAL_UINT(3u, r.periods_used);
}

static void test_freq_rejects_flat_signal(void) {
    for (size_t i = 0u; i < (size_t) N; ++i) {
        buf[i] = 1.65f;
    }
    const freq_result_t r = calc_frequency(buf, N, 19200.0f, 3u);
    TEST_ASSERT_FALSE(r.valid);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_freq_40hz);
    RUN_TEST(test_freq_50hz);
    RUN_TEST(test_freq_55hz);
    RUN_TEST(test_freq_caps_periods);
    RUN_TEST(test_freq_rejects_flat_signal);
    return UNITY_END();
}

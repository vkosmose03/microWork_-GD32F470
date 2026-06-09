#include "calc/dfdt.h"
#include "test_util.h"
#include "unity.h"

void setUp(void) {
}
void tearDown(void) {
}

enum { N = 3840 };
static float buf[N];

static void test_dfdt_positive_chirp(void) {
    /* Sweep 45 -> 55 Hz over 0.2 s => dF/dt = 50 Hz/s. */
    const double k = gen_chirp(buf, N, 19200.0, 45.0, 55.0, 2.0, 1.65);
    const dfdt_result_t r = calc_dfdt(buf, N, 19200.0f, 6u);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_TRUE(r.periods_used >= 2u && r.periods_used <= 6u);
    TEST_ASSERT_FLOAT_WITHIN(8.0f, (float) k, r.dfdt_hz_per_s);
    TEST_ASSERT_TRUE(r.f_end_hz > r.f_start_hz);
}

static void test_dfdt_steady_is_near_zero(void) {
    gen_sine(buf, N, 19200.0, 50.0, 2.0, 1.65, 0.0);
    const dfdt_result_t r = calc_dfdt(buf, N, 19200.0f, 6u);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 0.0f, r.dfdt_hz_per_s);
}

static void test_dfdt_needs_two_periods(void) {
    /* Less than two periods of 50 Hz (< 768 samples). */
    gen_sine(buf, 500u, 19200.0, 50.0, 2.0, 1.65, 0.0);
    const dfdt_result_t r = calc_dfdt(buf, 500u, 19200.0f, 6u);
    TEST_ASSERT_FALSE(r.valid);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dfdt_positive_chirp);
    RUN_TEST(test_dfdt_steady_is_near_zero);
    RUN_TEST(test_dfdt_needs_two_periods);
    return UNITY_END();
}

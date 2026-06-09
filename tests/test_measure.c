#include "measure/measure.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

static void test_sample_to_volts(void) {
    const measure_calib_t cal = {3.3f / 4095.0f, 2048.0f};
    /* Mid-scale code maps to ~0 V (within one LSB). */
    TEST_ASSERT_FLOAT_WITHIN(cal.scale, 0.0f, measure_sample_to_volts(2048u, &cal));
    /* Full scale code 4095 maps to (4095-2048)*scale. */
    const float expected = (4095.0f - 2048.0f) * cal.scale;
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected, measure_sample_to_volts(4095u, &cal));
}

static void test_convert_block(void) {
    const measure_calib_t cal = {0.001f, 1000.0f};
    const adc_sample_t raw[4] = {1000u, 2000u, 0u, 1500u};
    float out[4];
    measure_convert_block(raw, 4u, &cal, out);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.0f, out[0]);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 1.0f, out[1]);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, -1.0f, out[2]);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.5f, out[3]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sample_to_volts);
    RUN_TEST(test_convert_block);
    return UNITY_END();
}

#include "config/config.h"
#include "hal/hal_nvm.h"
#include "unity.h"

/* Provided by fake_hal_nvm.c. */
extern void fake_nvm_corrupt(uint32_t offset);

void setUp(void) {
    (void) hal_nvm_erase();
}
void tearDown(void) {
}

static void test_defaults_are_valid(void) {
    config_params_t p;
    config_set_defaults(&p);
    TEST_ASSERT_EQUAL_UINT32(CONFIG_MAX_SAMPLE_RATE_HZ, p.sample_rate_hz);
    TEST_ASSERT_EQUAL_INT(0, config_validate(&p)); /* nothing to clamp */
}

static void test_validate_clamps_out_of_range(void) {
    config_params_t p;
    config_set_defaults(&p);
    p.sample_rate_hz = 50000u; /* above max */
    p.freq_periods = 10u;      /* above max */
    p.dfdt_periods = 1u;       /* below min */
    p.adc_scale = -1.0f;       /* invalid */
    TEST_ASSERT_NOT_EQUAL(0, config_validate(&p));
    TEST_ASSERT_EQUAL_UINT32(CONFIG_MAX_SAMPLE_RATE_HZ, p.sample_rate_hz);
    TEST_ASSERT_EQUAL_UINT16(CONFIG_MAX_FREQ_PERIODS, p.freq_periods);
    TEST_ASSERT_EQUAL_UINT16(CONFIG_MAX_DFDT_PERIODS, p.dfdt_periods);
    TEST_ASSERT_TRUE(p.adc_scale > 0.0f);
}

static void test_save_then_load_roundtrip(void) {
    config_params_t saved;
    config_set_defaults(&saved);
    saved.sample_rate_hz = 9600u;
    saved.freq_periods = 2u;
    saved.dfdt_periods = 4u;
    saved.uart_baud = 57600u;
    TEST_ASSERT_EQUAL_INT(0, config_save(&saved));

    config_params_t loaded;
    TEST_ASSERT_EQUAL_INT(0, config_load(&loaded));
    TEST_ASSERT_EQUAL_UINT32(9600u, loaded.sample_rate_hz);
    TEST_ASSERT_EQUAL_UINT16(2u, loaded.freq_periods);
    TEST_ASSERT_EQUAL_UINT16(4u, loaded.dfdt_periods);
    TEST_ASSERT_EQUAL_UINT32(57600u, loaded.uart_baud);
}

static void test_load_blank_nvm_uses_defaults(void) {
    config_params_t loaded;
    /* NVM was just erased in setUp -> no valid record. */
    TEST_ASSERT_LESS_THAN_INT(0, config_load(&loaded));
    TEST_ASSERT_EQUAL_UINT32(CONFIG_MAX_SAMPLE_RATE_HZ, loaded.sample_rate_hz);
}

static void test_load_detects_corruption(void) {
    config_params_t saved;
    config_set_defaults(&saved);
    TEST_ASSERT_EQUAL_INT(0, config_save(&saved));

    /* Flip a byte inside the stored parameters; CRC must reject it. */
    fake_nvm_corrupt(8u);

    config_params_t loaded;
    TEST_ASSERT_LESS_THAN_INT(0, config_load(&loaded));
    TEST_ASSERT_EQUAL_UINT32(CONFIG_MAX_SAMPLE_RATE_HZ, loaded.sample_rate_hz);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_defaults_are_valid);
    RUN_TEST(test_validate_clamps_out_of_range);
    RUN_TEST(test_save_then_load_roundtrip);
    RUN_TEST(test_load_blank_nvm_uses_defaults);
    RUN_TEST(test_load_detects_corruption);
    return UNITY_END();
}

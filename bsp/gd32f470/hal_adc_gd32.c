/**
 * @file  hal_adc_gd32.c
 * @brief hal_adc implementation: TIMER1 -> ADC0 -> DMA1, fully hardware driven.
 *
 * A timer update event (TRGO) triggers one ADC conversion at the configured
 * sample rate; DMA streams each result into a circular double buffer. The CPU
 * is interrupted only at the half- and full-transfer points, where it forwards
 * the freshly completed half-block to the registered callback. Between blocks
 * the CPU is free (task requirement #4).
 *
 * Default analog input: ADC0 channel 0 on pin PA0 (channels 0..7 map to PA0..7).
 */
#include "hal/hal_adc.h"

#include "gd32f4xx.h"

#define ADC_DMA_PERIPH DMA1
#define ADC_DMA_CHANNEL DMA_CH0
#define ADC_MAX_BLOCK 4096u

/* Circular DMA buffer holds two blocks (double buffering). */
static adc_sample_t s_dma_buffer[2u * ADC_MAX_BLOCK];
static size_t s_block;
static hal_adc_block_cb_t s_callback;
static void *s_ctx;
static uint8_t s_channel;
static uint32_t s_actual_rate_hz;

/* Forward declaration for the DMA interrupt handler (weak in startup). */
void DMA1_Channel0_IRQHandler(void);

static void adc_gpio_config(uint8_t channel) {
    /* Channels 0..7 are exposed on PA0..PA7. */
    rcu_periph_clock_enable(RCU_GPIOA);
    const uint32_t pin = (uint32_t) (1u << channel);
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, pin);
}

static void adc_timer_config(uint32_t sample_rate_hz) {
    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1);

    /* On the default clock tree the APB1 timers run at 2 x PCLK1. */
    const uint32_t timer_clk = 2u * rcu_clock_freq_get(CK_APB1);
    uint32_t ticks = timer_clk / sample_rate_hz;
    if (ticks == 0u) {
        ticks = 1u;
    }
    s_actual_rate_hz = timer_clk / ticks;

    timer_parameter_struct tp;
    timer_struct_para_init(&tp);
    tp.prescaler = 0u;
    tp.alignedmode = TIMER_COUNTER_EDGE;
    tp.counterdirection = TIMER_COUNTER_UP;
    tp.period = ticks - 1u;
    tp.clockdivision = TIMER_CKDIV_DIV1;
    tp.repetitioncounter = 0u;
    timer_init(TIMER1, &tp);

    /* The update event becomes the TRGO that triggers the ADC. */
    timer_master_output_trigger_source_select(TIMER1, TIMER_TRI_OUT_SRC_UPDATE);
}

static void adc_dma_config(void) {
    rcu_periph_clock_enable(RCU_DMA1);
    dma_deinit(ADC_DMA_PERIPH, ADC_DMA_CHANNEL);

    dma_single_data_parameter_struct dp;
    dma_single_data_para_struct_init(&dp);
    dp.periph_addr = (uint32_t) (&ADC_RDATA(ADC0));
    dp.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dp.memory0_addr = (uint32_t) s_dma_buffer;
    dp.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dp.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
    dp.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    dp.direction = DMA_PERIPH_TO_MEMORY;
    dp.number = 2u * s_block;
    dp.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, &dp);

    /* ADC0 is mapped to DMA1 channel 0, sub-peripheral 0. */
    dma_channel_subperipheral_select(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, DMA_SUBPERI0);
    dma_circulation_enable(ADC_DMA_PERIPH, ADC_DMA_CHANNEL);
    dma_interrupt_enable(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, DMA_CHXCTL_HTFIE | DMA_CHXCTL_FTFIE);
    nvic_irq_enable(DMA1_Channel0_IRQn, 1u, 0u);
    dma_channel_enable(ADC_DMA_PERIPH, ADC_DMA_CHANNEL);
}

static void adc_core_config(uint8_t channel) {
    rcu_periph_clock_enable(RCU_ADC0);
    /* Keep the ADC clock within spec: PCLK2/8 (e.g. 100 MHz/8 = 12.5 MHz). */
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);

    adc_deinit();
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);

    adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 1u);
    adc_routine_channel_config(ADC0, 0u, channel, ADC_SAMPLETIME_15);

    adc_external_trigger_source_config(ADC0, ADC_ROUTINE_CHANNEL, ADC_EXTTRIG_ROUTINE_T1_TRGO);
    adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_RISING);

    adc_dma_mode_enable(ADC0);

    adc_enable(ADC0);
    for (volatile uint32_t i = 0u; i < 10000u; ++i) {
        /* short settling delay before calibration */
    }
    adc_calibration_enable(ADC0);
}

int hal_adc_init(const hal_adc_config_t *cfg) {
    if (cfg == NULL || cfg->on_block == NULL || cfg->sample_rate_hz == 0u ||
        cfg->block_samples == 0u || cfg->block_samples > ADC_MAX_BLOCK) {
        return -1;
    }

    s_block = cfg->block_samples;
    s_callback = cfg->on_block;
    s_ctx = cfg->ctx;
    s_channel = cfg->channel;

    adc_gpio_config(s_channel);
    adc_timer_config(cfg->sample_rate_hz);
    adc_dma_config();
    adc_core_config(s_channel);
    return 0;
}

int hal_adc_start(void) {
    if (s_callback == NULL) {
        return -1;
    }
    timer_enable(TIMER1); /* start generating triggers */
    return 0;
}

int hal_adc_stop(void) {
    timer_disable(TIMER1);
    return 0;
}

uint32_t hal_adc_actual_rate_hz(void) {
    return s_actual_rate_hz;
}

void DMA1_Channel0_IRQHandler(void) {
    if (dma_interrupt_flag_get(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, DMA_INT_FLAG_HTF) == SET) {
        dma_interrupt_flag_clear(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, DMA_INT_FLAG_HTF);
        /* First half is full. */
        s_callback(&s_dma_buffer[0], s_block, s_ctx);
    }
    if (dma_interrupt_flag_get(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, DMA_INT_FLAG_FTF) == SET) {
        dma_interrupt_flag_clear(ADC_DMA_PERIPH, ADC_DMA_CHANNEL, DMA_INT_FLAG_FTF);
        /* Second half is full. */
        s_callback(&s_dma_buffer[s_block], s_block, s_ctx);
    }
}

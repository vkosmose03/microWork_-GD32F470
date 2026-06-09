/**
 * @file  hal_time_gd32.c
 * @brief hal_time implementation using the Cortex-M SysTick at 1 kHz.
 */
#include "hal/hal_time.h"

#include "gd32f4xx.h"

static volatile uint32_t s_ticks_ms;

/* Forward declaration for the SysTick interrupt handler (weak in startup). */
void SysTick_Handler(void);

int hal_time_init(void) {
    s_ticks_ms = 0u;
    /* SystemCoreClock is configured by SystemInit before main(). */
    if (SysTick_Config(SystemCoreClock / 1000u) != 0u) {
        return -1;
    }
    return 0;
}

void SysTick_Handler(void) {
    ++s_ticks_ms;
}

uint32_t hal_time_now_ms(void) {
    return s_ticks_ms;
}

void hal_delay_ms(uint32_t ms) {
    const uint32_t start = s_ticks_ms;
    while ((s_ticks_ms - start) < ms) {
        __NOP();
    }
}

/**
 * @file  hal_uart_gd32.c
 * @brief hal_uart implementation on USART0 (PA9 = TX, PA10 = RX).
 */
#include "hal/hal_uart.h"

#include "gd32f4xx.h"

int hal_uart_init(uint32_t baudrate) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    /* PA9/PA10 -> USART0 (alternate function 7). */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9 | GPIO_PIN_10);

    usart_deinit(USART0);
    usart_baudrate_set(USART0, baudrate);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
    return 0;
}

int hal_uart_write(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *) data;
    if (p == NULL) {
        return -1;
    }
    for (size_t i = 0u; i < len; ++i) {
        usart_data_transmit(USART0, (uint16_t) p[i]);
        while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET) {
        }
    }
    return 0;
}

int hal_uart_write_str(const char *str) {
    size_t n = 0u;
    if (str == NULL) {
        return -1;
    }
    while (str[n] != '\0') {
        ++n;
    }
    return hal_uart_write(str, n);
}

void hal_uart_flush(void) {
    while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {
    }
}

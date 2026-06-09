/**
 * @file  hal_uart.h
 * @brief Abstract UART output interface for result reporting (req* #4).
 */
#ifndef HAL_UART_H
#define HAL_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @return 0 on success, negative on error. */
int hal_uart_init(uint32_t baudrate);

/** Transmit @p len bytes. Implementations may use DMA to offload the CPU. */
int hal_uart_write(const void *data, size_t len);

/** Convenience helper for NUL-terminated strings. */
int hal_uart_write_str(const char *str);

/** Block until all queued bytes have left the transmitter. */
void hal_uart_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */

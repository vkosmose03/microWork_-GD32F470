/**
 * @file  hal_time.h
 * @brief Abstract monotonic time base.
 */
#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the time base (e.g. SysTick). @return 0 on success. */
int hal_time_init(void);

/** @return Milliseconds elapsed since @ref hal_time_init. */
uint32_t hal_time_now_ms(void);

/** Busy/sleep delay for @p ms milliseconds. */
void hal_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIME_H */

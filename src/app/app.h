/**
 * @file  app.h
 * @brief Application head module.
 *
 * Orchestrates the independent modules — it wires acquisition (hal_adc) to the
 * measurement and calculation cores and to result reporting (hal_uart) and
 * persisted configuration (config). It contains no hardware-specific code and
 * talks to the board only through the HAL.
 */
#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise all modules and run the acquisition/analysis loop forever.
 *
 * Assumes the time base (hal_time_init) has already been started.
 */
void app_run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */

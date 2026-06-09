/**
 * @file  gd32f4xx_libopt.h
 * @brief Selects which GD32F4xx standard-peripheral modules are compiled in.
 *
 * Included at the bottom of <gd32f4xx.h>. We enable only the peripherals this
 * firmware actually uses, which keeps the build small.
 */
#ifndef GD32F4XX_LIBOPT_H
#define GD32F4XX_LIBOPT_H

#include "gd32f4xx_adc.h"
#include "gd32f4xx_dma.h"
#include "gd32f4xx_fmc.h"
#include "gd32f4xx_gpio.h"
#include "gd32f4xx_misc.h"
#include "gd32f4xx_pmu.h"
#include "gd32f4xx_rcu.h"
#include "gd32f4xx_syscfg.h"
#include "gd32f4xx_timer.h"
#include "gd32f4xx_usart.h"

#endif /* GD32F4XX_LIBOPT_H */

/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
// STM32f4
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_rng.h>
#include "misc.h"
#include "system_stm32f4xx.h"
// FreeRTOS
#include <FreeRTOS.h>
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "timers.h"
// Misc
#include "io.h"
#include "timer.h"

/**
 * CONSTANTS
 */

#define         LED_ON                      1
#define         LED_OFF                     0
        
#define         LED_ERROR_FLASHES           5
#define         LED_FLASH_PERIOD_MS         2000
#define         SW_IRQ_PIN                  21

#define         TIMER_ID                    0

#define         AVERAGE_USAGE_INTERVAL_MS   5000

/**
 * PROTOTYPES
 */


#endif      // MAIN_H

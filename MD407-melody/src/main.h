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
#include <stm32f4xx_dac.h>
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
#include "usart.h"
#include "printf.h"
#include "ftos.h"

/**
 * CONSTANTS
 */

#define         AVERAGE_USAGE_INTERVAL_MS   5000
#define         BACKGROUND_TASK_SLEEP_US    1

/**
 * HELPER FUNCTIONS
 */
void handle_switched_in(int* pxCurrentTCB);
void handle_switched_out(int* pxCurrentTCB);
void task_cpu_usage(TimerHandle_t timer);

#endif      // MAIN_H

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
#include "usart.h"
#include "printf.h"
#include "timer.h"
#include "ftos.h"

/**
 * CONSTANTS
 */

#define         LED_ON                      1
#define         LED_OFF                     0

#define         SW_IRQ_PIN                  21
        
#define         LED_ERROR_FLASHES           5
#define         LED_FLASH_PERIOD_MS         2000

#define         TIMER_ID_LED_ON             0

#define         AVERAGE_USAGE_INTERVAL_MS   5000

#define         SHARED_SIZE                 8

#define         A_MATRIX_ROWS               8
#define         A_MATRIX_COLUMNS            SHARED_SIZE          
#define         B_MATRIX_ROWS               SHARED_SIZE
#define         B_MATRIX_COLUMNS            40   
#define         RESULT_MATRIX_ROWS          A_MATRIX_ROWS
#define         RESULT_MATRIX_COLUMNS       B_MATRIX_COLUMNS

/**
 * HELPER FUNCTIONS
 */
void print_result_matrix(float matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS]);
void print_a_matrix(float matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]);
void print_b_matrix(float matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]);
void handle_switched_in(int* pxCurrentTCB);
void handle_switched_out(int* pxCurrentTCB);
void task_cpu_usage(TimerHandle_t timer);

#endif      // MAIN_H

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


// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>
// Pico SDK
#include "pico/stdlib.h"            // Includes `hardware_gpio.h`
#include "pico/rand.h"
// C
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

#define         SHARED_SIZE                 4

#define         A_MATRIX_ROWS               4
#define         A_MATRIX_COLUMNS            SHARED_SIZE          
#define         B_MATRIX_ROWS               SHARED_SIZE
#define         B_MATRIX_COLUMNS            4       
#define         RESULT_MATRIX_ROWS          A_MATRIX_ROWS
#define         RESULT_MATRIX_COLUMNS       B_MATRIX_COLUMNS

/**
 * PROTOTYPES
 */
void setup();
void setup_led();
void setup_gpio();

void led_on();
void led_off();
void led_set(bool state);

/**
 * HELPER FUNCTIONS
 */
void print_result_matrix(float matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS]);
void print_a_matrix(float matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]);
void print_b_matrix(float matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]);
void handle_switched_in(int* pxCurrentTCB);
void handle_switched_out(int* pxCurrentTCB);
void task_cpu_usage(TimerHandle_t timer);
void log_debug(const char* msg);
void log_device_info(void);


#endif      // MAIN_H

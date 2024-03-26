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
#include <time.h>

/**
 * CONSTANTS
 */

#define         LED_ON                      1
#define         LED_OFF                     0
        
#define         LED_ERROR_FLASHES           5
#define         LED_FLASH_PERIOD_MS         2000
#define         SW_IRQ_PIN                  21

#define         TIMER_ID_LED_ON             0

#define         AVERAGE_USAGE_INTERVAL_MS   5000

/**
 * PROTOTYPES
 */
void setup();
void setup_led();
void setup_gpio();

void enable_irq(bool state);

void led_on();
void led_off();
void led_set(bool state);

void task_led_pico(void* unused_arg);

void enable_irq(bool state);
void gpio_isr(uint gpio, uint32_t events);

void task_sleep(void* unused_arg);


#endif      // MAIN_H

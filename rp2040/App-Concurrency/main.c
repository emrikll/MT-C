

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>
// Pico SDK
#include "hardware/timer.h"
#include "pico/stdlib.h"            // Includes `hardware_gpio.h`
#include "pico/rand.h"
// C
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FREQUENCY_HIGH 1 / portTICK_PERIOD_MS

#define MAX_VALUE 10000
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated.  6817583
*/
#define STACK_SIZE 70
#define REFERENCE

/* Structure that will hold the TCB of the tasks being created. */

#ifndef  REFERENCE
StaticTask_t xTaskBufferLow;
StackType_t xStackLow[ STACK_SIZE ];

StaticTask_t xTaskBufferHigh;
StackType_t xStackHigh[ STACK_SIZE ];

StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t shared_variable_lock;

#else

StaticTask_t xTaskBufferReference;
StackType_t xStackReference[ STACK_SIZE ];
#endif /* ifndef  REFERENCE */


uint32_t start_time = 0;

uint32_t shared_variable = 0;
uint8_t done = 0;


/*
 ** Tasks 
*/

uint32_t largest_stack = ~0;
uint32_t START_STACK = 0;

__attribute__( ( always_inline ) ) uint32_t __get_MSP(void) {
  register uint32_t result;

  __asm__ volatile ("MRS %0, msp\n" : "=r" (result) );
  return(result);
}

void tick() {
    uint32_t current = __get_MSP();
    if (current < largest_stack) {
        largest_stack = current;
    }
}



#ifdef REFERENCE
void reference_task(void *parameter) {
    while (1) {
        if (done) {
            vTaskSuspend(NULL);
        }
        shared_variable++;
        if(shared_variable == MAX_VALUE) {
            done = 1;
            uint32_t end_time = time_us_64();

            printf("Reached max value at after: %u\r\n", end_time - start_time);
        } 
    }
}

#else
void increment_shared() {
    if (done) {
        vTaskSuspend(NULL);
    }
    if (xSemaphoreTake(shared_variable_lock, portMAX_DELAY) != pdTRUE) {
        printf("Something went wrong\r\n");
    }
    shared_variable++;
    if(shared_variable == MAX_VALUE) {
        done = 1;
        uint32_t end_time = time_us_64();

        printf("Reached max value at after: %d\r\n", end_time - start_time);
    } 
    
    xSemaphoreGive(shared_variable_lock);
}

void low_priority_task(void *parameter) {
    while (1) {
        increment_shared();
    }
}

void high_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_HIGH);
    }
}
#endif

void log_debug(const char* msg) {

#ifdef DEBUG
    uint msg_length = 9 + strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "[DEBUG] %s\n", msg);
    printf("%s", sprintf_buffer);
    free(sprintf_buffer);
#endif
}


/**
 * @brief Show basic device info.
 */
void log_device_info(void) {

    printf("App: %s %s (%i)\n", APP_NAME, APP_VERSION, BUILD_NUM);
}

int main(void)
{
    timer_hw->dbgpause = 0;
    #ifdef DEBUG
    stdio_init_all();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    #endif
    
    // Log app info
    timer_hw->dbgpause = 1;

    printf("\n\rInit\n\r");

    #ifndef REFERENCE
    shared_variable_lock = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    configASSERT(shared_variable_lock != NULL);

    xTaskCreateStatic(
        high_priority_task, 
        "High priority", 
        STACK_SIZE, 
        NULL, 
        2, 
        xStackHigh,
        &xTaskBufferHigh
    );


    xTaskCreateStatic(
        low_priority_task, 
        "Low priority", 
        STACK_SIZE, 
        NULL, 
        1, 
        xStackLow,
        &xTaskBufferLow
    );

    #else

    xTaskCreateStatic(
        reference_task, 
        "Reference task", 
        STACK_SIZE, 
        NULL, 
        1, 
        xStackReference,
        &xTaskBufferReference
    );
    #endif


    printf("Starting scheduler...\n\r");
    start_time = time_us_64();
    vTaskStartScheduler();

}


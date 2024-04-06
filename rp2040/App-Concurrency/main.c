

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

#define CAPACITY_HIGH 3
#define CAPACITY_MEDIUM 3
#define CAPACITY_LOW 5

#define FREQUENCY_HIGH 5 / portTICK_PERIOD_MS
#define FREQUENCY_MEDIUM 10 / portTICK_PERIOD_MS
#define FREQUENCY_LOW 15 / portTICK_PERIOD_MS
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 200

/* Structure that will hold the TCB of the tasks being created. */
StaticTask_t xTaskBufferLow[CAPACITY_LOW];
StackType_t xStackLow[CAPACITY_LOW][ STACK_SIZE ];

StaticTask_t xTaskBufferMedium[CAPACITY_MEDIUM];
StackType_t xStackMedium[CAPACITY_MEDIUM][ STACK_SIZE ];

StaticTask_t xTaskBufferHigh[CAPACITY_HIGH];
StackType_t xStackHigh[CAPACITY_HIGH][ STACK_SIZE ];

UBaseType_t uxHighWaterMark;

#define MAX_VALUE 10000

uint64_t start_time = 0;

uint32_t shared_variable = 0;
uint8_t done = 0;
StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t shared_variable_lock;
/*
 ** Tasks 
*/

void increment_shared() {
    if (done) {
        vTaskSuspend(NULL);
    }
    if (xSemaphoreTake(shared_variable_lock, portMAX_DELAY) != pdTRUE) {
        printf("Something went wrong\r\n");
    }
    shared_variable++;
    
    xSemaphoreGive(shared_variable_lock);
    if(shared_variable == MAX_VALUE) {
        done = 1;
        uint64_t end_time = time_us_64();
        printf("Reached max value at after: %llu\n\r", end_time-start_time);
    } 
}

void low_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_LOW);
    }
}

void medium_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_MEDIUM);
    }
}

void high_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_HIGH);
    }
}

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
    log_debug("test");
    #endif
    
    // Log app info
    timer_hw->dbgpause = 1;

    printf("\n\rInit\n\r");

    shared_variable_lock = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    configASSERT(shared_variable_lock != NULL);

    for (int i = 0; i < CAPACITY_HIGH; i++) {
        xTaskCreateStatic(
            high_priority_task, 
            "High priority", 
            STACK_SIZE, 
            NULL, 
            3, 
            xStackHigh[i],
            &xTaskBufferHigh[i]
        );
    }

    for (int i = 0; i < CAPACITY_MEDIUM; i++) {
        xTaskCreateStatic(
            medium_priority_task, 
            "Medium priority", 
            STACK_SIZE, 
            NULL, 
            2, 
            xStackMedium[i],
            &xTaskBufferMedium[i]
        );
    }

    for (int i = 0; i < CAPACITY_LOW; i++) {
        xTaskCreateStatic(
            low_priority_task, 
            "Low priority", 
            STACK_SIZE, 
            NULL, 
            1, 
            xStackLow[i],
            &xTaskBufferLow[i]
        );
    }


    printf("Starting scheduler...\n\r");
    printf("test %u",3);
    start_time = time_us_64();
    vTaskStartScheduler();

}


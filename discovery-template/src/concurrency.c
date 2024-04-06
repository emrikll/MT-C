
#include <stdint.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "timer.h"
#include "timers.h"
#include "rng.h"
#include "semphr.h"
#include "printf.h"
#include "usart.h"

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
#define STACK_SIZE 70

/* Structure that will hold the TCB of the tasks being created. */
StaticTask_t xTaskBufferLow[CAPACITY_LOW];
StackType_t xStackLow[CAPACITY_LOW][ STACK_SIZE ];

StaticTask_t xTaskBufferMedium[CAPACITY_MEDIUM];
StackType_t xStackMedium[CAPACITY_MEDIUM][ STACK_SIZE ];

StaticTask_t xTaskBufferHigh[CAPACITY_HIGH];
StackType_t xStackHigh[CAPACITY_HIGH][ STACK_SIZE ];

UBaseType_t uxHighWaterMark;

#define MAX_VALUE 10000

uint32_t start_time = 0;

uint32_t shared_variable = 0;
uint8_t done = 0;

UBaseType_t uxHighWaterMark;
UBaseType_t uxMediumWaterMark;
UBaseType_t uxLowWaterMark;
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
        printf_("Something went wrong\r\n");
    }
    shared_variable++;
    if(shared_variable == MAX_VALUE) {
        done = 1;
        uint32_t end_time = time_us();
        printf_("Reached max value at after: %d\r\n", end_time - start_time);
        printf_("Watermark high: %ld\r\n", uxHighWaterMark);
        printf_("Watermark medium: %ld\r\n", uxMediumWaterMark);
        printf_("Watermark low: %ld\r\n", uxLowWaterMark);
    } 
    
    xSemaphoreGive(shared_variable_lock);
}

void low_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_LOW);
        uxLowWaterMark = uxTaskGetStackHighWaterMark( NULL );
    }
}

void medium_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_MEDIUM);
        uxMediumWaterMark = uxTaskGetStackHighWaterMark( NULL );
    }
}

void high_priority_task(void *parameter) {
    while (1) {
        increment_shared();
        vTaskDelay(FREQUENCY_HIGH);
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    }
}

int main(void)
{
    enable_timer();
    enable_usart();
    printf_("\n\rInit\n\r");

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


    printf_("Starting scheduler...\n\r");
    start_time = time_us();
    vTaskStartScheduler();

}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
 
    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
 
    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
 
    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

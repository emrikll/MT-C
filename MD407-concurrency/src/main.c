

#include <stdint.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "timer.h"
#include "timers.h"
#include "semphr.h"
#include "usart.h"
#include "printf.h"

#define FREQUENCY_HIGH 1 / portTICK_PERIOD_MS

#define MAX_VALUE 10000
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated.  6817583
*/
#define STACK_SIZE 70

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




#ifdef REFERENCE
void reference_task(void *parameter) {
    while (1) {
        if (done) {
            vTaskSuspend(NULL);
        }
        shared_variable++;
        if(shared_variable == MAX_VALUE) {
            done = 1;
            uint32_t end_time = time_us();

            printf_("Reached max value at after: %d\r\n", end_time - start_time);
        } 
    }
}

#else
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

int main(void)
{
    enable_usart();
    printf_("\n\rInit\n\r");

    
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

    enable_timer();

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

/*
* PERFORMANCE FUNCTIONS 
*/
/**
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int* pxCurrentTCB) {
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int* pxCurrentTCB) {
}


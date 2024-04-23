
#include <stdint.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "core_cmFunc.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "timer.h"
#include "timers.h"
#include "rng.h"
#include "semphr.h"
#include "printf.h"
#include "usart.h"

#define FREQUENCY_HIGH 1 / portTICK_PERIOD_MS

#define MAX_VALUE 100000
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated.  6817583
*/
#define STACK_SIZE 100
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

volatile uint32_t shared_variable = 0;
uint8_t done = 0;

int START_STACK = 0;
uint32_t largest_stack = ~0; 
void tick() {
    uint32_t current = __get_MSP();
    if (current < largest_stack) {
        largest_stack = current;
    }
}
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

            printf_("%u\n", end_time-start_time);
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
    //tick();
    shared_variable++;
    if(shared_variable == MAX_VALUE) {
        done = 1;
        uint32_t end_time = time_us();

        //printf_("%08x\n", largest_stack);
        printf_("%u\n", end_time-start_time);
    } 
    //tick();
    
    xSemaphoreGive(shared_variable_lock);
}

void low_priority_task(void *parameter) {
    while (1) {
        //tick();
        increment_shared();
        //tick();
    }
}

void high_priority_task(void *parameter) {
    while (1) {
        //tick();
        increment_shared();
        //tick();
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

int main(void)
{
    START_STACK = __get_MSP();
    
    enable_usart();

    tick();

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


    //printf_("Starting scheduler...\n\r");
    enable_timer();
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

// FreeRTOS
#include <FreeRTOS.h>
#include "core_cmFunc.h"
#include "portmacro.h"
#include "task.h"
#include "timers.h"
// Misc
#include "usart.h"
#include "printf.h"
#include "timer.h"
#include <stdint.h>
//STM32
#include "rng.h"



#include "FreeRTOSConfig.h"

/**
 * CONSTANTS
 */
#define         AVERAGE_USAGE_INTERVAL_MS   5000

#define         SHARED_SIZE                 4

#define         A_MATRIX_ROWS               3
#define         A_MATRIX_COLUMNS            SHARED_SIZE          
#define         B_MATRIX_ROWS               SHARED_SIZE
#define         B_MATRIX_COLUMNS            7    
#define         RESULT_MATRIX_ROWS          A_MATRIX_ROWS
#define         RESULT_MATRIX_COLUMNS       B_MATRIX_COLUMNS


/**
 * HELPER FUNCTIONS
 */
void print_result_matrix(double matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS]);
void print_a_matrix(double matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]);
void print_b_matrix(double matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]);
void task_cpu_usage(TimerHandle_t timer);

/*
 * GLOBALS
 */

uint32_t start_time;

// Matrix
double a_matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS] = {7986.45, 1292.79, 8583.79, 2072.98, 2161.08, 7137.87, 8844.89542, 1241.16699, 9333.09941, 1046.33654, 1766.28663, 227.57371};
double b_matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS] = {2089.46, 484.29, 4070.86, 6388.14, 5878.41, 7796.02, 4174.04910, 3779.38097, 1819.78879, 392.12370, 5173.20243, 2525.39435, 4430.77499, 3700.96781, 3312.82425, 2487.50948, 1680.72726, 1257.68497, 5224.09199, 5027.58651, 4620.30426, 7821.20553, 5898.87661, 3104.60453, 4500.93917, 3847.383526, 1115.46655, 1150.36475};
double result_matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS];


// Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO;

// Capacity
int capacity_task_i_row = 0;
int CAPACITY = 250;

int START_STACK = 0;
uint32_t largest_stack = ~0; 

// Static allocation task_i_row

/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_I_ROW 200

//#define REFERENCE

#ifndef REFERENCE
/* Structure that will hold the TCB of the task being created. */
StaticTask_t task_buffer_i_row[RESULT_MATRIX_ROWS];
StackType_t stack_i_row[RESULT_MATRIX_ROWS][ STACK_SIZE_I_ROW ];
#else
StaticTask_t task_buffer_reference;
StackType_t stack_reference[ STACK_SIZE_I_ROW ];
#endif

void tick() {
    uint32_t current = __get_MSP();
    if (current < largest_stack) {
        largest_stack = current;
    }
}

uint32_t task0start = 0;

#ifdef REFERENCE
void reference_task(void *parameters) {
    task0start = time_us();
    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            double tmp = 0.0;
            for (int k = 0; k < A_MATRIX_COLUMNS; k++) {
                tmp = tmp + (a_matrix[(i * A_MATRIX_COLUMNS) + k] * b_matrix[(k * B_MATRIX_COLUMNS) + j]);
            }
            result_matrix[(i * RESULT_MATRIX_COLUMNS) + j] = tmp;
        }

    }
    uint32_t end_time = time_us();
    //TickType_t end = xTaskGetTickCount();
    //printf_("End_time FreeRTOS: %u\n\r", end);
    //printf_("End_time: %u\n\r", end_time - start_time);
    tick();
    printf_("Stack usage: %u\r\n", START_STACK - largest_stack);
    printf_("%u\n", end_time - task0start);
    //print_result_matrix(result_matrix);
    vTaskEndScheduler();
 
}

#else
void task_i_row(void *parameter) {
    tick();
    int i;
    i = (int) parameter;
    //printf_("Task %d\n\r", i);
    if (i == 0) {
        task0start = time_us();
    }

    for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
        double tmp = 0.0;
        for (int k = 0; k < A_MATRIX_COLUMNS; k++) {
            tmp = tmp + (a_matrix[(i * A_MATRIX_COLUMNS) + k] * b_matrix[(k * B_MATRIX_COLUMNS) + j]);
        }
        result_matrix[(i * RESULT_MATRIX_COLUMNS) + j] = tmp;
    }

    capacity_task_i_row = capacity_task_i_row - 1;
    if (capacity_task_i_row == 0) {
        uint32_t end_time = time_us();
        tick();
        printf_("Stack usage: %u\r\n", START_STACK - largest_stack);
        printf_("%u\n", end_time - task0start);
        vTaskEndScheduler();
    }

    vTaskSuspend(NULL);

    while(1){
        taskYIELD();
    }
}
#endif

/*
 * RUNTIME START
 */

int main() {
    START_STACK = __get_MSP();
    enable_usart();
    enable_timer();

    //printf_("Hardware Initialized");
    //printf_("A_ROW %d A_COL %d\n\r",A_MATRIX_ROWS, A_MATRIX_COLUMNS);
    //printf_("B_ROW %d B_COL %d\n\r",B_MATRIX_ROWS, B_MATRIX_COLUMNS);
    //printf_("RESULT_ROW %d RESULT_COL %d\n\r",RESULT_MATRIX_ROWS, RESULT_MATRIX_COLUMNS);
    //print_a_matrix(a_matrix);
    //print_b_matrix(b_matrix);
    
    #ifndef REFERENCE
    char buf[5];
    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        // Convert 123 to string [buf]
        capacity_task_i_row++;
        sprintf_(buf,"%i", i);
        xTaskCreateStatic(task_i_row,
                        buf,
                        128,
                        (void *) i,
                        1,
                        stack_i_row[i],
                        &task_buffer_i_row[i]);
        //printf_("Created task %d\n\r", i);
    }
    #else
    xTaskCreateStatic(reference_task,
        "reference",
        128,
        NULL,
        1,
        stack_reference,
        &task_buffer_reference
    );
    #endif

            
    // Start the FreeRTOS scheduler if any of the tasks are good
    start_time = time_us();
    // Start the scheduler
    vTaskStartScheduler();

    // We should never get here, but just in case...
    while(1) {
        // NOP
    };
}

/* 
###############################################
    HELPER FUNCTIONS
###############################################
*/

/*
* PRINTS
*/

void print_result_matrix(double matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS]) {
    printf_("Matrix: \n\r");

    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        printf_("[");
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            if (j == (RESULT_MATRIX_COLUMNS - 1)) {
                printf_("%f", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
            } else {
                printf_("%f, ", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
            }
        }
        printf_("]\n\r");
    }
}

void print_a_matrix(double matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]) {
    printf_("Matrix A: \n\r");

    for (int i = 0; i < A_MATRIX_ROWS; i++) {
        printf_("[");
        for (int j = 0; j < A_MATRIX_COLUMNS; j++) {
            printf_("%f , ", matrix[(i * A_MATRIX_COLUMNS) + j]);
        }
        printf_("]\n\r");
    }
}

void print_b_matrix(double matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]) {
    printf_("Matrix B: \n\r");

    for (int i = 0; i < B_MATRIX_ROWS; i++) {
        printf_("[");
        for (int j = 0; j < B_MATRIX_COLUMNS; j++) {
            printf_("%f , ", matrix[(i * B_MATRIX_COLUMNS) + j]);
        }
        printf_("]\n\r");
    }
}


/*
* PERFORMANCE FUNCTIONS 
*/

/**
 * @brief Callback actioned when CPU usage timer fires.
 *
 * @param timer: The triggering timer.
 */
void task_cpu_usage(TimerHandle_t timer) {

    char str[64];
    int average_time = (AVERAGE_USAGE_INTERVAL_MS*1000);
    float usage = ( (float)average_time - (float)xTotalPICO) / (float)average_time;

    xTotalPICO = 0;

    sprintf_(str, "CPU USAGE: %f, BACKGROUND TASKS: %u", usage,capacity_task_i_row);
    printf_(str);    
}

/*
 * FREERTOS 
 */
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

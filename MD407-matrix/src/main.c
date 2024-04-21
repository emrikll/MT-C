/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"
#include <stdint.h>



/*
 * GLOBALS
 */

uint32_t start_time;

// Matrix
double a_matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS] = {6493.0010772183505, 7582.753585543219, 2886.5908298879344, 1580.996700648023, 4932.9083078769245, 6013.421565496147, 2305.8404418713453, 1048.636298653361, 148.59202833690065, 1134.661652116645, 871.5431799141179, 6248.729020110439, 4526.410447913371, 7416.572022893935, 5318.728645462816, 2264.366665541318, 9363.189989062224, 4354.314663900577, 8939.624077213286, 165.8935506800632, 7818.9114576900965, 1941.0876804045652, 1180.2907571639653, 9588.140345895139, 2544.9145787725733, 6049.3760538388615, 2259.57806649316, 4627.790691721134, 8058.766387412454, 5192.81221370038, 2127.3284373088104, 7145.906329881167, 9265.76581558166, 7932.765380883406, 9393.620314026273, 8306.099754520801, 7245.796722445732, 9618.16776018687, 3237.9271749498007, 1610.6617658901368, 3545.5344167886838, 4764.57591719395, 5211.271249047136, 4611.110547661095, 5808.2217634850085, 9671.267846984822, 7083.115880067184, 4317.228187223145, 7528.625553915294, 2398.337584389064, 682.4513459358881, 3484.7304274221574, 8636.360038001763, 3071.677603423843, 9542.331277835645, 5334.462250323774, 4228.76680697824, 3208.6078292413536, 2274.961792112064, 9032.255330847147, 3254.5630376316044, 5662.966925444376, 6113.256791993757, 702.0072707765635, 9840.347301067035, 2337.5425192843245, 3854.9208947289635, 7813.343685779217, 4218.9654512054485, 9054.215681088639, 4066.112159442206, 6554.7021020507245, 3476.4783871239233, 7012.979662516754, 3729.185165774779, 5760.576187758447, 4696.075569111393, 5209.256507431735, 5669.576319025982, 3126.950930924558, 5442.851511799855, 3628.944839370402, 3514.4653749119334, 3844.572237858752, 3721.355018663799, 498.1446825563108, 809.7601515668498, 6927.171762520157, 978.3928508296265, 6142.985864529906};
double b_matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS] = {8162.806005973125, 4183.3575643752865, 3616.6784794271102, 1424.211877165885, 4883.415129756621, 7680.044058197326, 3208.3571197764177, 5647.482112602054, 7758.894445905474, 3372.798518551431, 3269.664191229248, 9468.094553754803, 852.9864878873334, 5266.395183761748, 8645.194552183788, 1295.2148093118547, 5205.155787577218, 5804.09437452712, 6604.535067858797, 9069.73697107244, 1253.2205971449378, 9633.530391037692, 5087.094499282108, 9475.512727230005, 9351.737592711395, 7093.662432757216, 1698.177941028452, 9855.936415286305, 5552.050995228151, 6468.426436134641, 4498.610485305896, 825.4700764511294, 6275.650285097346, 4106.118643502823, 3789.860635398018, 3865.619458613959, 1629.9982786697515, 1454.0328898891237, 6618.369209073846, 5714.225622377182, 928.5424526495882, 8509.857790335747, 4963.056989680855, 4005.677667051236, 7148.615224883154, 1773.585644892449, 1695.477922786628, 9220.326219451767, 3245.58593380863, 3434.1195618982333, 2165.740786676334, 1865.1583642155967, 4627.125451139354, 9739.451563699517, 2844.513438650101, 610.8444315167823, 3395.5011134536585, 343.67569512016985, 5219.954019229468, 5666.910133764983, 7147.717810870016, 8460.441052499244, 3789.4407991191974, 4784.661144203094, 4854.985263132807, 9885.13756474279, 5269.523578606661, 5683.823610151527, 9873.041643459559, 8962.744599823338, 236.4377228203493, 9017.653291200235, 6796.120173968283, 6635.453974538446, 4695.168563969692, 6558.223008527843, 6408.92579849802, 4988.662199506984, 526.3881126572733, 9261.762614507998, 9634.6725784353, 952.3903396270152, 1889.5347461678562, 4644.680579658528, 9139.130029396725, 1203.772133151192, 3893.7964606782875, 8036.512452887755, 4427.554747450984, 1627.4787956163075};

double volatile result_matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS];


// Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO;

// Capacity
int capacity_task_i_row = 0;
int CAPACITY = 250;

// Static allocation task_i_row

/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_I_ROW 200

#ifndef REFERENCE
/* Structure that will hold the TCB of the task being created. */
StaticTask_t task_buffer_i_row[RESULT_MATRIX_ROWS];
StackType_t stack_i_row[RESULT_MATRIX_ROWS][ STACK_SIZE_I_ROW ];
#else
StaticTask_t task_buffer_reference;
StackType_t stack_reference[ STACK_SIZE_I_ROW ];
#endif
volatile uint32_t largest_stack = ~0;
uint32_t START_STACK = 0;

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
    //printf_("Stack usage: %u\r\n", START_STACK - largest_stack);
    printf_("%u\n", end_time - task0start);
    //print_result_matrix(result_matrix);
    vTaskEndScheduler();
 
}

#else
void task_i_row(void *parameter) {
    int i;
    i = (int) parameter;
    //printf_("Task %d\n\r", i);
    if (i == 0) {
        task0start = time_us();
    }

    for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
        double tmp = 0.0;
        tick();
        for (int k = 0; k < A_MATRIX_COLUMNS; k++) {
            tick();
            tmp = tmp + (a_matrix[(i * A_MATRIX_COLUMNS) + k] * b_matrix[(k * B_MATRIX_COLUMNS) + j]);
        }
        result_matrix[(i * RESULT_MATRIX_COLUMNS) + j] = tmp;
    }

    capacity_task_i_row = capacity_task_i_row - 1;
    if (capacity_task_i_row == 0) {
        uint32_t end_time = time_us();
        //TickType_t end = xTaskGetTickCount();
        //printf_("End_time FreeRTOS: %u\n\r", end);
        //printf_("End_time: %u\n\r", end_time - start_time);
        vTaskDelay(500);

        tick();

        printf_("stack %u %u\n\r", START_STACK, largest_stack);
        printf_("time %u\n", end_time - task0start);
        //print_result_matrix(result_matrix);
        vTaskDelay(500);
        NVIC_SystemReset(); 

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

    enable_timer();

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
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeInPICO = time_us();
        //printf_("Switched in to IDLE");
     }
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        char str[64];
        xTimeOutPICO = time_us();
        xDifferencePICO = xTimeOutPICO - xTimeInPICO;
        xTotalPICO = xTotalPICO + xDifferencePICO;
        sprintf_(str, "IDLE time: %lu", xDifferencePICO);
        //printf_(str);
        xTimeInPICO = xTimeOutPICO = xDifferencePICO = 0;
     }
}

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

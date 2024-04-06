/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"



/*
 * GLOBALS
 */

uint32_t start_time;

// Matrix
double a_matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS] = {7986.45, 1292.79, 8583.79, 2072.98, 2161.08, 7137.87, 8844.89542, 1241.16699, 9333.09941, 1046.33654, 1766.28663, 227.57371};
double b_matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS] = {2089.46, 484.29, 4070.86, 6388.14, 5878.41, 7796.02, 4174.04910, 3779.38097, 1819.78879, 392.12370, 5173.20243, 2525.39435, 4430.77499, 3700.96781, 3312.82425, 2487.50948, 1680.72726, 1257.68497, 5224.09199, 5027.58651, 4620.30426, 7821.20553, 5898.87661, 3104.60453, 4500.93917, 3847.383526, 1115.46655, 1150.36475};
double result_matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS];


// Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO = 0;

// Capacity
int capacity_task_i_row = 0;
int CAPACITY = 250;

// Static allocation task_i_row

/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_I_ROW 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t task_buffer_i_row[RESULT_MATRIX_ROWS];

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t stack_i_row[RESULT_MATRIX_ROWS][ STACK_SIZE_I_ROW ];

uint32_t task0start = 0;
void task_i_row(void *parameter) {
    int i;
    i = (int) parameter;

    if (i == 0) {
        task0start = time_us_32();
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
        uint32_t end_time = time_us_32();
        printf("End_time: %u\n\r", end_time - start_time);
        printf("End_time from task 0: %u\n\r", end_time - task0start);
        print_result_matrix(result_matrix);
    }

    vTaskSuspend(NULL);

    while(true){
        taskYIELD();
    }
}


/*
 * RUNTIME START
 */

int main() {
    timer_hw->dbgpause = 0;
    // DEBUG
    #ifdef DEBUG
    stdio_init_all();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    #endif

    // Set up the hardware
    setup();
    
    // Log app info
    #ifdef DEBUG
    log_device_info();
    #endif

    printf("A_ROW %d A_COL %d\n\r",A_MATRIX_ROWS, A_MATRIX_COLUMNS);
    printf("B_ROW %d B_COL %d\n\r",B_MATRIX_ROWS, B_MATRIX_COLUMNS);
    printf("RESULT_ROW %d RESULT_COL %d\n\r",RESULT_MATRIX_ROWS, RESULT_MATRIX_COLUMNS);
    print_a_matrix(a_matrix);
    print_b_matrix(b_matrix);

    char buf[5];
    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        // Convert 123 to string [buf]
        capacity_task_i_row++;
        sprintf(buf,"%ld", i);
        xTaskCreateStatic(task_i_row,
                        buf,
                        128,
                        (void *) i,
                        1,
                        stack_i_row[i],
                        &task_buffer_i_row[i]);
        printf("Created task %d\n\r", i);
    }

            
    // Start the FreeRTOS scheduler if any of the tasks are good
    if (true) {
        start_time = time_us_32();
        // Start the scheduler
        vTaskStartScheduler();
    } else {
        // Flash board LED 5 times
        uint8_t count = LED_ERROR_FLASHES;
        while (count > 0) {
            led_on();
            sleep_ms(100);
            led_off();
            sleep_ms(100);
            count--;
        }
    }

    // We should never get here, but just in case...
    while(true) {
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
    printf("Matrix: \n\r");

    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        printf("[");
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            if (j == (RESULT_MATRIX_COLUMNS - 1)) {
                printf("%f", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
            } else {
                printf("%f, ", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
            }
        }
        printf("]\n\r");
    }
}

void print_a_matrix(double matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]) {
    printf("Matrix A: \n\r");

    for (int i = 0; i < A_MATRIX_ROWS; i++) {
        printf("[");
        for (int j = 0; j < A_MATRIX_COLUMNS; j++) {
            printf("%f , ", matrix[(i * A_MATRIX_COLUMNS) + j]);
        }
        printf("]\n\r");
    }
}

void print_b_matrix(double matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]) {
    printf("Matrix B: \n\r");

    for (int i = 0; i < B_MATRIX_ROWS; i++) {
        printf("[");
        for (int j = 0; j < B_MATRIX_COLUMNS; j++) {
            printf("%f , ", matrix[(i * B_MATRIX_COLUMNS) + j]);
        }
        printf("]\n\r");
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
        xTimeInPICO = time_us_64();
        log_debug("Switched in to IDLE");
     }
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        char str[64];
        xTimeOutPICO = time_us_64();
        xDifferencePICO = xTimeOutPICO - xTimeInPICO;
        xTotalPICO = xTotalPICO + xDifferencePICO;
        sprintf(str, "IDLE time: %lu", xDifferencePICO);
        log_debug(str);
        xTimeInPICO, xTimeOutPICO, xDifferencePICO = 0;
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

    sprintf(str, "CPU USAGE: %f, BACKGROUND TASKS: %u", usage,capacity_task_i_row);
    log_debug(str);    
}

/*
 * LED FUNCTIONS
 */

/**
 * @brief Configure the on-board LED.
 */
void setup_led() {

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    led_off();
}

/**
 * @brief Set the on-board LED's state.
 */
void led_set(bool state) {

    gpio_put(PICO_DEFAULT_LED_PIN, state);
}

/**
 * @brief Turn the on-board LED on.
 */
void led_on() {

    led_set(true);
}


/**
 * @brief Turn the on-board LED off.
 */
void led_off() {

    led_set(false);
}

/*
* GPIO
*/

void setup_gpio() {

    gpio_init(SW_IRQ_PIN);
    gpio_set_dir(SW_IRQ_PIN, GPIO_OUT);
    gpio_put(SW_IRQ_PIN, 0);
}

/*
 * SETUP FUNCTIONS
 */

/**
 * @brief Umbrella hardware setup routine.
 */
void setup() {
    setup_gpio();
    setup_led();
}

/*
* DEBUG 
*/

/**
 * @brief Generate and print a debug message from a supplied string.
 *
 * @param msg: The base message to which `[DEBUG]` will be prefixed.
 */
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



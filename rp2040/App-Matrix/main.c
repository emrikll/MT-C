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

// Matrix
float a_matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
float b_matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
float result_matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS];


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


void task_i_row(void *parameter) {
    int i;
    i = (int) parameter;
    printf("Task %d\n\r", i);

    for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
        float tmp = 0;
        for (int k = 0; k < A_MATRIX_COLUMNS; k++) {
            tmp = tmp + (a_matrix[(i * A_MATRIX_COLUMNS) + k] * b_matrix[(k * B_MATRIX_COLUMNS) + j]);
        }
        result_matrix[(i * RESULT_MATRIX_COLUMNS) + j] = tmp;
    }

    capacity_task_i_row = capacity_task_i_row - 1;
    if (capacity_task_i_row == 0) {
        uint32_t end_time = time_us_32();
        TickType_t end = xTaskGetTickCount();
        printf("End_time: %u\n\r", end);
        print_result_matrix(result_matrix);
    }

    while(true){};
}


/*
 * RUNTIME START
 */

int main() {
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

void print_result_matrix(float matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS]) {
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

void print_a_matrix(float matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]) {
    printf("Matrix A: \n\r");

    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        printf("[");
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            printf("%f , ", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
        }
        printf("]\n\r");
    }
}

void print_b_matrix(float matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]) {
    printf("Matrix B: \n\r");

    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        printf("[");
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            printf("%f , ", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
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



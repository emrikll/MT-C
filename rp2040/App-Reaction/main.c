/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"


/*
 * GLOBALS
 */

// Task handles
TaskHandle_t handle_task_pico = NULL;

// Semaphores
SemaphoreHandle_t semaphore_irq = NULL;

//Tick counters IRQ
uint64_t xStart, xEnd, xStartISR, xDifference, xDifferenceISR = 0;

//Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO = 0;

// Static allocation LED task
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_INTERRUPT 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferInterrupt;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackInterrupt[ STACK_SIZE_INTERRUPT ];

/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer[250];

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack[250][ STACK_SIZE ];

int capacity_task_sleep = 0;
int CAPACITY = 250;

StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_sleep_capacity;

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
    enable_irq(true);
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

/*
 * IRQ
 */

/**
 * @brief Enable or disable the IRQ,
 *
 * @param state: The enablement state. Default: `true`.
 */
void enable_irq(bool state) {

    gpio_set_irq_enabled_with_callback(SW_IRQ_PIN,
                                       GPIO_IRQ_EDGE_RISE,
                                       state,
                                       &gpio_isr);
}

/**
 * @brief ISR for GPIO.
 *
 * @param gpio:   The pin that generates the event.
 * @param events: Which event(s) triggered the IRQ.
 */
void gpio_isr(uint gpio, uint32_t events) {
    // See BLOG POST https://blog.smittytone.net/2022/03/20/fun-with-freertos-and-pi-pico-interrupts-semaphores-notifications/
    
    xStartISR = time_us_64();

    // Signal the alert clearance task
    static BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(semaphore_irq, &higher_priority_task_woken);
    
    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/*
 * TASKS
 */

/**
 * @brief Turn the Pico's built-in LED on or off based on LED_STATE.
 */
void task_handle_interrupt(void* unused_arg) {

    bool LED_STATE = true;

    while(true){
        if (xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS) {
            xEnd = time_us_64();
            char str[64];
            xDifference = xEnd - xStart;
            xDifferenceISR = xStartISR - xStart;
            sprintf(str, "DifferenceISR: %lu, DIFFERENCEIRQ: %lu", xDifferenceISR, xDifference);
            log_debug(str);
            xStart, xEnd, xDifference = 0; 
            if (xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS){
                if (capacity_task_sleep < CAPACITY){
                    xTaskCreateStatic(task_sleep, "SLEEP_TASK", 128, NULL,  1, xStack[capacity_task_sleep], &xTaskBuffer[capacity_task_sleep]);
                    capacity_task_sleep++;
                }
            }
        }else{
            log_debug("Could not take semaphore");
        }
    }
}

/**
 * @brief Callback actioned when the post IRQ timer fires.
 *
 * @param timer: The triggering timer.
 */
void timer_fired_callback(TimerHandle_t timer) {

    xStart = time_us_64();
    // Create EDGE_RISE on Software interrupt pin to trigger LED 
    gpio_put(SW_IRQ_PIN, 1);
    
    gpio_put(SW_IRQ_PIN, 0);
    
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

    sprintf(str, "CPU USAGE: %f, BACKGROUND TASKS: %u, CLOCK_SPEED: %u", usage,capacity_task_sleep, clock_get_hz(clk_sys));
    log_debug(str);    
}

/**
 * @brief Sleeper task to take up CPU time.
 */
void task_sleep(void* unused_arg) {

    uint32_t rand_nr = get_rand_32() % 10;  

    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    while(true){
        vTaskDelayUntil(&xLastWakeTime, xDelay);
        rand_nr = get_rand_32() % 10;
        sleep_ms(rand_nr*1000);
    }
    
}

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

void resus_callback(void) {
    // Reconfigure PLL sys back to the default state of 1500 / 6 / 2 = 125MHz
    pll_init(pll_sys, 1, 1600 * MHZ, 6, 2);
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    133 * MHZ,
                    133 * MHZ);
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    133 * MHZ,
                    133 * MHZ);
    // Reconfigure uart as clocks have changed
    stdio_init_all();

    // Wait for uart output to finish
    uart_default_tx_wait_blocking();

}
/*
 * RUNTIME START
 */
int main() {
    // DEBUG
    timer_hw->dbgpause = 0;
    
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

    clocks_enable_resus(&resus_callback);
    pll_deinit(pll_sys);

    // Set up four tasks
    handle_task_pico = xTaskCreateStatic(task_handle_interrupt, 
                                        "PICO_INTERRUPT_TASK",  
                                        128, 
                                        NULL, 
                                        1, 
                                        xStackInterrupt, 
                                        &xTaskBufferInterrupt);

    TimerHandle_t task_timer = xTimerCreate("HANDLE_INTERRUPT_TIMER", pdMS_TO_TICKS(INTERRUPT_PERIOD_MS), pdTRUE, (void*)TIMER_ID_LED_ON, timer_fired_callback);
    TimerHandle_t timer_cpu_usage = xTimerCreate("USAGE_CPU_TIMER", pdMS_TO_TICKS(AVERAGE_USAGE_INTERVAL_MS), pdTRUE, (void*)TIMER_ID_LED_ON, task_cpu_usage);

    //xTaskCreate( prvIdleTask, ( signed portCHAR * ) "IDLE", tskIDLE_STACK_SIZE, ( void * ) NULL, tskIDLE_PRIORITY, &xIdleTaskHandle );
    log_debug("start");
    printf("test");
    if( task_timer == NULL || timer_cpu_usage == NULL ){
        /* The timer was not created. */
        log_debug("Timers was not created");
    }else{
    /* Start the timer.  No block time is specified, and
    even if one was it would be ignored because the RTOS
    scheduler has not yet been started. */
        if( xTimerStart( task_timer, 0 ) != pdPASS || xTimerStart( timer_cpu_usage, 0 ) != pdPASS )
        {
            /* The timer could not be set into the Active
            state. */
            log_debug("Timer could not be started");
        }
    }
        
    // Start the FreeRTOS scheduler if any of the tasks are good
    if (handle_task_pico != NULL) {
        // Create a binary semaphore to signal IRQs
        semaphore_irq = xSemaphoreCreateBinary();
        assert(semaphore_irq != NULL);
        mutex_sleep_capacity = xSemaphoreCreateMutexStatic( &xMutexBuffer );
        assert(mutex_sleep_capacity != NULL);
        
        // Start the scheduler
        vTaskStartScheduler();
    }

    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}



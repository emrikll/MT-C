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
#define STACK_SIZE_LED 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferLed;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackLed[ STACK_SIZE_LED ];

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
    setup_led();
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

    /* ISR FUNCTION BODY USING DIRECT TASK NOTIFICATIONS */
    /*
    // Signal the alert clearance task
    static BaseType_t higher_priority_task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(handle_task_pico, &higher_priority_task_woken);
    
    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
    */
    
    /* ISR FUNCTION BODY USING A SEMAPHORE */
    
    xStartISR = time_us_64();

    // Signal the alert clearance task
    static BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(semaphore_irq, &higher_priority_task_woken);
    
    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
    
    
    /*  ISR FUNCTION BODY USING A QUEUE */
    /*
    // Signal the alert clearance task
    static bool state = 1;
    xQueueSendToBackFromISR(irq_queue, &state, 0);
     */
}

/*
 * TASKS
 */

/**
 * @brief Turn the Pico's built-in LED on or off based on LED_STATE.
 */
void task_led_pico(void* unused_arg) {

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
            if (LED_STATE) {
                led_on();
                LED_STATE = false;
                //log_debug("LED turned on");
            } else {
                led_off();
                LED_STATE = true;
                //log_debug("LED turned off");
            }
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

    sprintf(str, "CPU USAGE: %f, BACKGROUND TASKS: %u", usage,capacity_task_sleep);
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

    // Set up four tasks
    handle_task_pico = xTaskCreateStatic(task_led_pico, 
                                        "PICO_LED_TASK",  
                                        128, 
                                        NULL, 
                                        1, 
                                        xStackLed, 
                                        &xTaskBufferLed);

    TimerHandle_t task_timer = xTimerCreate("LED_ON_TIMER", pdMS_TO_TICKS(LED_FLASH_PERIOD_MS), pdTRUE, (void*)TIMER_ID_LED_ON, timer_fired_callback);
    TimerHandle_t timer_cpu_usage = xTimerCreate("LED_ON_TIMER", pdMS_TO_TICKS(AVERAGE_USAGE_INTERVAL_MS), pdTRUE, (void*)TIMER_ID_LED_ON, task_cpu_usage);

    //xTaskCreate( prvIdleTask, ( signed portCHAR * ) "IDLE", tskIDLE_STACK_SIZE, ( void * ) NULL, tskIDLE_PRIORITY, &xIdleTaskHandle );
    
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
    } else {
        // Flash board LED 5 times
        uint8_t count = LED_ERROR_FLASHES;
        while (count > 0) {
            led_on();
            vTaskDelay(100);
            led_off();
            vTaskDelay(100);
            count--;
        }
    }

    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}



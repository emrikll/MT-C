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

// Timers
TimerHandle_t alert_timer = NULL;

// Semaphores
SemaphoreHandle_t semaphore_irq = NULL;

//Tick counters
uint64_t xStart, xEnd, xStartISR, xDifference, xDifferenceISR = 0;

//Function declarations
void gpio_isr(uint gpio, uint32_t events);
void enable_irq(bool state);



/* Dimensions of the buffer that the task being created will use as its stack.
    NOTE:  This is the number of words the stack will hold, not the number of
    bytes.  For example, if each stack item is 32-bits, and this is set to 100,
    then 400 bytes (100 * 32-bits) will be allocated. */
    #define STACK_SIZE 200

    /* Structure that will hold the TCB of the task being created. */
    StaticTask_t xTaskBuffer;

    /* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
    StackType_t xStack[ STACK_SIZE ];


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
    
    // Clear the IRQ source
    //enable_irq(false);
    

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

    while(true){
        if (xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS) {
            xEnd = time_us_64();
            char str[64];
            xDifference = xEnd - xStart;
            xDifferenceISR = xStartISR - xStart;
            
            sprintf(str, "DifferenceISR: %lu", xDifferenceISR);
            log_debug(str);
            xStart, xEnd, xDifference = 0;
            if (LED_STATE) {
                led_on();
                LED_STATE = false;
                log_debug("LED turned on");
            } else {
                led_off();
                LED_STATE = true;
                log_debug("LED turned off");
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

    log_debug("Timer callback fired");

    xStart = time_us_64();
    // Create EDGE_RISE on Software interrupt pin to trigger LED 
    gpio_put(SW_IRQ_PIN, 1);
    
    gpio_put(SW_IRQ_PIN, 0);
    
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
                                        xStack, 
                                        &xTaskBuffer);

    TimerHandle_t task_timer = xTimerCreate("LED_ON_TIMER", pdMS_TO_TICKS(LED_FLASH_PERIOD_MS), pdTRUE, (void*)TIMER_ID_LED_ON, timer_fired_callback);
    
    if( task_timer == NULL ){
        /* The timer was not created. */
        log_debug("Timer was not created");
    }else{
    /* Start the timer.  No block time is specified, and
    even if one was it would be ignored because the RTOS
    scheduler has not yet been started. */
        if( xTimerStart( task_timer, 0 ) != pdPASS )
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



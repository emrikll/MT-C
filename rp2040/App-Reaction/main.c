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
#include <time.h>


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
#define STACK_SIZE_INTERRUPT 400

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
#define CAPACITY 100
/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer[CAPACITY];

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack[CAPACITY][ STACK_SIZE ];

int capacity_task_sleep = 0;

StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_sleep_capacity;

TaskHandle_t sleep_handle;

volatile uint32_t largest_stack = ~0;
uint32_t START_STACK = 0;

__attribute__( ( always_inline ) ) uint32_t __get_MSP(void) {
  register uint32_t result;

  __asm__ volatile ("MRS %0, msp\n" : "=r" (result) );
  return(result);
}

void tick() {
    uint32_t current = __get_MSP();
    if (current < largest_stack) {
        largest_stack = current;
    }
}

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
    //tick(); 
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
            //tick();
            xDifference = xEnd - xStart;
            xDifferenceISR = xStartISR - xStart;
            UBaseType_t uxHighWaterMarkSleep;
            UBaseType_t uxHighWaterMarkCurrent;
            //uxHighWaterMarkSleep = uxTaskGetStackHighWaterMark(sleep_handle);
            //uxHighWaterMarkCurrent = uxTaskGetStackHighWaterMark(NULL);
            printf("%llu, %llu\n",xDifferenceISR,xDifference);
            //printf("%lu, %lu\n",xDifference,xDifferenceISR);
            UBaseType_t totalBytes = (STACK_SIZE_INTERRUPT - uxHighWaterMarkCurrent) * 4 + (STACK_SIZE - uxHighWaterMarkSleep) * 4 * CAPACITY; 
            //printf("%u, %08x\n", totalBytes, largest_stack);
            
            xStart, xEnd, xDifference = 0; 
            //tick();
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
 * @brief Sleeper task to take up CPU time.
 */
void task_sleep(void *vParameters) {
    uint32_t rand_nr_work = get_rand_32();
    uint32_t rand_nr_sleep = get_rand_32();
    uint32_t start = time_us_64();

    for(;;){
        rand_nr_work = get_rand_32() % 10;
        rand_nr_sleep = get_rand_32() % 10;
        start = time_us_64();

        while ((rand_nr_work * 1000) < (time_us_64() - start) );
        vTaskDelay(rand_nr_sleep / portTICK_PERIOD_MS);
    }
}

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

/*
 * RUNTIME START
 */
int main() {
    // DEBUG
    timer_hw->dbgpause = 0;
    
    stdio_init_all();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);

    // Set up the hardware
    setup();
    
    // Log app info
    #ifdef DEBUG
    log_device_info();
    #endif


    // Set up four tasks
    handle_task_pico = xTaskCreateStatic(task_handle_interrupt, 
                                        "PICO_INTERRUPT_TASK",  
                                        STACK_SIZE_INTERRUPT, 
                                        NULL, 
                                        2, 
                                        xStackInterrupt, 
                                        &xTaskBufferInterrupt);

    for (int i = 0; i < CAPACITY; i++) {
        sleep_handle = xTaskCreateStatic(task_sleep, "SLEEP_TASK", STACK_SIZE, NULL,  1, xStack[i], &xTaskBuffer[i]);
    }

    TimerHandle_t task_timer = xTimerCreate("HANDLE_INTERRUPT_TIMER", pdMS_TO_TICKS(INTERRUPT_PERIOD_MS), pdTRUE, (void*)TIMER_ID_LED_ON, timer_fired_callback);

    //xTaskCreate( prvIdleTask, ( signed portCHAR * ) "IDLE", tskIDLE_STACK_SIZE, ( void * ) NULL, tskIDLE_PRIORITY, &xIdleTaskHandle );
    log_debug("start");
    //printf("test");
    if( task_timer == NULL ){
        /* The timer was not created. */
        log_debug("Timers was not created");
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



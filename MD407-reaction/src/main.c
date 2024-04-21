#include "main.h"
#include "printf.h"
#include "projdefs.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

GPIO_InitTypeDef Gp;//Create GPIO struct
GPIO_InitTypeDef gpioStructure;

// EXT1 structures
GPIO_InitTypeDef GPIO_InitStruct;
EXTI_InitTypeDef EXTI_InitStruct;
NVIC_InitTypeDef NVIC_InitStruct;


// Defines
//LEDs
#define GreenLED_Pin GPIO_Pin_0
#define RedLED_Pin GPIO_Pin_1
#define LED_GPIO GPIOB

// EXTI0
#define EXTI0_Pin GPIO_Pin_12
#define EXTI0_GPIO GPIOD

// Init variables

// Timer variables
StaticTimer_t TimerBufferLed;
StaticTimer_t TimerBufferUsage;

// Semaphores
static SemaphoreHandle_t semaphore_irq = NULL;
StaticSemaphore_t xSemaphoreBuffer;
StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_sleep_capacity;

//Tick counters IRQ
uint32_t xStart, xEnd, xStartISR, xDifference, xDifferenceISR = 0;

//Tick counters IDLE
uint32_t xTimeInCPU, xTimeOutCPU, xDifferenceCPU, xTotalCPU = 0;

// FreeRTOS static allocation

// LED Task
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_LED 400
/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferLed;
/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackLed[STACK_SIZE_LED];

// Sleeper Task
uint32_t capacity_task_sleep = 0;
#define CAPACITY 20
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 200

/* 64ructure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer[CAPACITY];

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack[CAPACITY][ STACK_SIZE ];

int largest_stack = ~0;
int START_STACK = 0;

void tick() {
    uint32_t current = __get_MSP();
    if (current < largest_stack) {
        largest_stack = current;
    }
}

// Custom vector to get IRQ working
#define SCB_VTOR_CUSTOM ((volatile unsigned long *) 0xE000ED08)

/*
 ** Startup 
 */

void EnableInterruptEXTI0()
{    
    /* Enable clock for GPIOD */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Set pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = EXTI0_Pin;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(EXTI0_GPIO, &GPIO_InitStruct);
    
    /* Tell system that you will use PD0 for EXTI_Line0 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource0);
    
    /* PD0 is connected to EXTI_Line0 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PD0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
 
    NVIC_SetPriority(EXTI0_IRQn, 2);
}

void hardware_init(){
    
    //Deinit the rcc
    RCC_DeInit();
    //enable hse
    RCC_HSEConfig(RCC_HSE_ON);
    //wait for hse to start
    while (RCC_WaitForHSEStartUp() != SUCCESS);
    //configure pll modifiers to match 168 mhz
    RCC_PLLConfig(RCC_PLLSource_HSE, 4, 168, 2, 4);
    RCC_PCLK1Config(RCC_HCLK_Div4);
    RCC_PCLK2Config(RCC_HCLK_Div2);
    
    //enable pll
    RCC_PLLCmd(ENABLE);
    //wait for pll to become ready
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != SET);
    
    //set sysclk to use pll
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    //wait until pll is syclksource
    while (RCC_GetSYSCLKSource() != 0x08);


    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
    EnableInterruptEXTI0();
    enable_timer();
    enable_usart();
}

/*
 ** IRQ
 */

/* void TIM2_IRQHandler(uint64_t gpio, uint32_t event) {
  // ISR FUNCTION BODY USING A SEMAPHORE
    
  //xStartISR = time_us_64();

  // Signal the alert clearance task
  static BaseType_t higher_priority_task_woken = pdFALSE;
  xSemaphoreGiveFromISR(semaphore_irq, &higher_priority_task_woken);
  
  // Exit to context switch if necessary
  portYIELD_FROM_ISR(higher_priority_task_woken);
} */

/* Set interrupt handlers */
/* Handle PD0 interrupt */
void EXTI0_IRQHandler(void) {
    xStartISR = time_us();
    BaseType_t higher_priority_task_woken = pdFALSE;
    /* Make sure that interrupt flag is set */
    /* ISR FUNCTION BODY USING A SEMAPHORE */
    
    //xStartISR = time_us_64();

    // Signal the alert clearance task
    xSemaphoreGiveFromISR(semaphore_irq, &higher_priority_task_woken);

    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
    /* Clear interrupt flag */
    EXTI_ClearITPendingBit(EXTI_Line0);
}

/*
 ** Tasks 
 */
void task_sleep(void *vParameters) {
    uint32_t rand_nr = RNG_GetRandomNumber();
    uint32_t start = time_us();

    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        vTaskDelayUntil(&xLastWakeTime, xDelay);
        rand_nr = RNG_GetRandomNumber() % 10;
        start = time_us();
        while ((rand_nr * 1000000) < (time_us() - start) );
    }
}

void task_led(void *vParameters){

    int toggle = 0;
    GPIO_ResetBits(LED_GPIO, RedLED_Pin);
    vTaskDelay(1000);
    
    for(;;)
    {
        if(xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS) {
            xEnd = time_us();
            xDifference = xEnd - xStart;
            xDifferenceISR = xStartISR - xStart;
            char buffer1[16];
            char buffer2[32];
            char taskbuffer[8];
            sprintf_(buffer1, "%u", xDifferenceISR);
            sprintf_(buffer2, "%u", xDifference);
            sprintf_(taskbuffer, "%u", capacity_task_sleep);
            printf_("xDifferenceISR ");
            printf_(buffer1);
            
            printf_(", xDifference ");
            printf_(buffer2);
            printf_(", Background tasks ");
            printf_(taskbuffer);
            printf_("\n\r");
            xStart = xEnd = xDifference = xDifferenceISR = 0;
            if (toggle) {
                GPIO_ResetBits(LED_GPIO, RedLED_Pin);
            } else {
                GPIO_SetBits(LED_GPIO, RedLED_Pin);
            }
            
            RCC_ClocksTypeDef clocks;
            RCC_GetClocksFreq(&clocks);
            printf_("Clock speed: %d", clocks.SYSCLK_Frequency);

            toggle = ~toggle;
            if (xSemaphoreTake(mutex_sleep_capacity, portMAX_DELAY) == pdPASS){
                if (capacity_task_sleep < CAPACITY){
                    xTaskCreateStatic(task_sleep, "SLEEP_TASK", 128, NULL,  1, xStack[capacity_task_sleep], &xTaskBuffer[capacity_task_sleep]);
                    capacity_task_sleep++;
                }
                xSemaphoreGive(mutex_sleep_capacity);
            }
        }
        
    }
}

void task_cpu_average(TimerHandle_t timer) {
    int average_time = (AVERAGE_USAGE_INTERVAL_MS*1000);
    float usage = ( (float)average_time - (float)xTotalCPU) / (float)average_time;

    xTotalCPU = 0;

    char str[32];
    float_to_string(usage,str);
    printf_("Usage: ");
    printf_(str);
    printf_("\n\r");
}

/**
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeInCPU = time_us();
     }
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeOutCPU = time_us();
        xDifferenceCPU = xTimeOutCPU - xTimeInCPU;
        xTotalCPU = xTotalCPU + xDifferenceCPU;
        xTimeInCPU = xTimeOutCPU = xDifferenceCPU = 0;
     }
}

void led_timer_callback(TimerHandle_t timer) {

    xStart = time_us();
    
    EXTI_GenerateSWInterrupt(EXTI_Line0);
}



  
int main(void) {
    // Init hardware
    hardware_init();
    printf_("Hardware initialized\n\r");

    *((void (**)(void))0x2001C058) = EXTI0_IRQHandler;


    TaskHandle_t led_task = xTaskCreateStatic(task_led, "ToggleLED", 256, NULL, 2, xStackLed,&xTaskBufferLed);
    TimerHandle_t task_timer = xTimerCreateStatic("LED_ON_TIMER", pdMS_TO_TICKS(LED_FLASH_PERIOD_MS), pdTRUE, (void*)TIMER_ID, led_timer_callback, &TimerBufferLed);
    TimerHandle_t usage_timer = xTimerCreateStatic("CPU_UUSAGE_TIMER", pdMS_TO_TICKS(AVERAGE_USAGE_INTERVAL_MS), pdTRUE, (void*)TIMER_ID, task_cpu_average, &TimerBufferUsage);

    semaphore_irq = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
    configASSERT(semaphore_irq != NULL);
    mutex_sleep_capacity = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    configASSERT(mutex_sleep_capacity != NULL);

    if( task_timer == NULL || usage_timer == NULL ){
        /* The timer was not created. */
        printf_("Timers was not created\n");
    }else{
    /* Start the timer.  No block time is specified, and
    even if one was it would be ignored because the RTOS
    scheduler has not yet been started. */
        if( xTimerStart( task_timer, 0 ) != pdPASS || xTimerStart(usage_timer,0) != pdPASS)
        {
            /* The timer could not be set into the Active
            state. */
            printf_("Timer could not be started\n");
        }
    }

    printf_("Starting scheduler\n\r");
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

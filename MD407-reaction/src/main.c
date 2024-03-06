#include "main.h"
#include "FreeRTOSConfig.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rng.h"

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

// Task Handles

// Semaphores
static SemaphoreHandle_t semaphore_irq = NULL;

// Tick counters IRQ

// Tick counter IDLE


// FreeRTOS static allocation

// LED Task
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
StackType_t xStackLed[STACK_SIZE_LED];

// Sleeper Task
int capacity_task_sleep = 0;
#define CAPACITY 20
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer[CAPACITY];

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack[CAPACITY][ STACK_SIZE ];

// Semaphores
StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_sleep_capacity;

// Custom vector to get IRQ working
#define SCB_VTOR_CUSTOM ((volatile unsigned long *) 0xE000ED08)

/*
 ** Startup 
 */

void InitializeLEDs()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
    gpioStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    gpioStructure.GPIO_Mode = GPIO_Mode_OUT;
    gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpioStructure);
 
    GPIO_WriteBit(GPIOD, GPIO_Pin_12 | GPIO_Pin_13, Bit_RESET);
}
 
void InitializeTimer()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
 
    TIM_TimeBaseInitTypeDef timerInitStructure; 
    timerInitStructure.TIM_Prescaler = 40000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 500;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);
}

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

void EnableTimerInterrupt()
{
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void hardware_init(){
    //InitializeLEDs();
    //InitializeTimer(); 
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
    print("Enabling interrupts\n");
    EnableInterruptEXTI0();
    print("Interrupts enabled\n");
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
    print("In IRQ handler\n");
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

void task_led(void *vParameters){

    int toggle = 0;
    GPIO_ResetBits(LED_GPIO, RedLED_Pin);
    vTaskDelay(1000);
    
    for(;;)
    {
        if(xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS) {
            print("woo task!\n");
            if (toggle) {
                GPIO_ResetBits(LED_GPIO, RedLED_Pin);
            } else {
                GPIO_SetBits(LED_GPIO, RedLED_Pin);
            }

            toggle = ~toggle;
        }else{
            print("Could not take Semaphore\n");
        }
    }
}

void task_cpu_average(TimerHandle_t timer) {
    
}

void task_sleep(void *vParameters) {
    uint32_t rand_nr = RNG_GetRandomNumber(); 

    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        vTaskDelayUntil(&xLastWakeTime, xDelay);
        rand_nr = RNG_GetRandomNumber() % 10;
        //TODO add sleep function
    }
}

void led_timer_callback(TimerHandle_t timer) {
    print("In LED callback\n");
    
    EXTI_GenerateSWInterrupt(EXTI_Line0);
}



  
int main(void) {
    // Init hardware
    print("Hardware initialization\n");
    hardware_init();

    *((void (**)(void))0x2001C058) = EXTI0_IRQHandler;


    TaskHandle_t led_task = xTaskCreateStatic(task_led, "ToggleLED", 128, NULL, 1, xStackLed,&xTaskBufferLed);
    TimerHandle_t task_timer = xTimerCreate("LED_ON_TIMER", pdMS_TO_TICKS(LED_FLASH_PERIOD_MS), pdTRUE, (void*)TIMER_ID_LED_ON, led_timer_callback);

    semaphore_irq = xSemaphoreCreateBinary();
    configASSERT(semaphore_irq != NULL);

    if( task_timer == NULL ){
        /* The timer was not created. */
        print("Timers was not created\n");
    }else{
    /* Start the timer.  No block time is specified, and
    even if one was it would be ignored because the RTOS
    scheduler has not yet been started. */
        if( xTimerStart( task_timer, 0 ) != pdPASS )
        {
            /* The timer could not be set into the Active
            state. */
            print("Timer could not be started\n");
        }
    }

    print("Starting scheduler\n");
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

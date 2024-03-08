#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "usart.h"
#include "timer.h"
#include "timers.h"
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "utils/idle.h"
#include "rng.h"
#include "semphr.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "printf.h"
GPIO_InitTypeDef GPIO_InitStruct_EXTI;
EXTI_InitTypeDef EXTI_InitStruct;
NVIC_InitTypeDef NVIC_InitStruct;


GPIO_InitTypeDef Gp;//Create GPIO struct
//Tick counters IRQ
uint64_t xStart, xEnd, xStartISR, xDifference, xDifferenceISR = 0;

//Tick counters IDLE

// Semaphores
static SemaphoreHandle_t semaphore_irq = NULL;
StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_sleep_capacity;

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
#define CAPACITY 1
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
StaticSemaphore_t pxSemaphoreBuffer;

StaticTimer_t tasktimerbuffer;
StaticTimer_t usageTimerBuffer;
//Define LED pins
#define GreenLED_Pin GPIO_Pin_9
#define BlueLED_Pin GPIO_Pin_8
#define LED_GPIO GPIOC

//Define Push button
#define PushButton_Pin GPIO_Pin_0
#define PushButton_GPIO GPIOA   

#define EXTI0_Pin GPIO_Pin_12
#define EXTI0_GPIO GPIOD

#define         LED_ON                      1
#define         LED_OFF                     0
        
#define         LED_ERROR_FLASHES           5
#define         LED_FLASH_PERIOD_MS         2000
#define         SW_IRQ_PIN                  21

#define         TIMER_ID                    0

#define         AVERAGE_USAGE_INTERVAL_MS   5000

void EnableInterruptEXTI0()
{    
    /* Enable clock for GPIOD */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Set pin as input */
    GPIO_InitStruct_EXTI.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct_EXTI.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct_EXTI.GPIO_Pin = EXTI0_Pin;
    GPIO_InitStruct_EXTI.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct_EXTI.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(EXTI0_GPIO, &GPIO_InitStruct_EXTI);
    
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
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_1_IRQn;
    /* Set priority */
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
 
    NVIC_SetPriority(EXTI0_1_IRQn, 2);
}

void EXTI0_1_IRQHandler(void) {
    BaseType_t higher_priority_task_woken = pdFALSE;
    /* Make sure that interrupt flag is set */
    /* ISR FUNCTION BODY USING A SEMAPHORE */
    
    xStartISR = time_us();

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
    uint32_t rand_nr;
    uint32_t start;

    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        vTaskDelayUntil(&xLastWakeTime, xDelay);
        rand_nr = get_random_byte() % 10;
        start = time_us();
        while ((rand_nr * 1000000) < (time_us() - start) );
    }
}

void task_led(void *vParameters){

    int toggle = 0;
    GPIO_ResetBits(LED_GPIO, GreenLED_Pin);
    vTaskDelay(1000);
    
    for(;;)
    {
        if(xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS) {
            xEnd = time_us();
            xDifference = xEnd - xStart;
            xDifferenceISR = xStartISR - xStart;
        printf_("xDifferenceISR: %lu, xDifference: %lu \n\r", (long unsigned int)xDifferenceISR, (long unsigned int)xDifference);
            xStart = 0; 
            xEnd = 0;
            xDifference = 0;
            if (toggle) {
                GPIO_ResetBits(LED_GPIO, GreenLED_Pin);
            } else {
                GPIO_SetBits(LED_GPIO, GreenLED_Pin);
            }

            toggle = ~toggle;
            if (xSemaphoreTake(mutex_sleep_capacity, portMAX_DELAY) == pdPASS){
                if (capacity_task_sleep < CAPACITY){
                    xTaskCreateStatic(task_sleep, "SLEEP_TASK", 128, NULL,  1, xStack[capacity_task_sleep], &xTaskBuffer[capacity_task_sleep]);
                    capacity_task_sleep++;
                }

                xSemaphoreGive(mutex_sleep_capacity);
            }
        }else{
            printf_("Could not take Semaphore\n");
        }
        
    }
}

void led_timer_callback(TimerHandle_t timer) {

    xStart = time_us();
    
    EXTI_GenerateSWInterrupt(EXTI_Line0);
}

void task_cpu_average(TimerHandle_t timer) {
    float usage = cpu_usage_percent();

    printf_("CPU usage: %f \n\r", usage);
    printf_("\n\r");
}

int main(void)
{
	//Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
    printf_("Init\n\r");
    enable_usart();
    enable_timer();
    adc_init();
    EnableInterruptEXTI0();

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	Gp.GPIO_Pin = GreenLED_Pin | BlueLED_Pin; //Set pins inside the struct
	Gp.GPIO_Mode = GPIO_Mode_OUT; //Set GPIO pins as output
	Gp.GPIO_OType = GPIO_OType_PP; //Ensure output is push-pull vs open drain
	Gp.GPIO_PuPd = GPIO_PuPd_NOPULL; //No internal pullup resistors required
	Gp.GPIO_Speed = GPIO_Speed_Level_1; //Set GPIO speed to lowest
	GPIO_Init(LED_GPIO, &Gp); //Assign struct to LED_GPIO

	Gp.GPIO_Pin = PushButton_Pin; //Set pins inside the struct
	Gp.GPIO_Mode = GPIO_Mode_IN; //Set GPIO pins as output
	Gp.GPIO_PuPd = GPIO_PuPd_NOPULL; //No pullup required as pullup is external
	GPIO_Init(PushButton_GPIO, &Gp); //Assign struct to LED_GPIO
    
    printf_("Before timers\n\r");
    TaskHandle_t led_task = xTaskCreateStatic(task_led, "ToggleLED", 128, NULL, 1, xStackLed,&xTaskBufferLed);
    TimerHandle_t task_timer = xTimerCreateStatic("LED_ON_TIMER", pdMS_TO_TICKS(LED_FLASH_PERIOD_MS), pdTRUE, (void*)TIMER_ID, led_timer_callback, &tasktimerbuffer);
    TimerHandle_t usage_timer = xTimerCreateStatic("CPU_UUSAGE_TIMER", pdMS_TO_TICKS(AVERAGE_USAGE_INTERVAL_MS), pdTRUE, (void*)TIMER_ID, task_cpu_average, &usageTimerBuffer);
    printf_("AFter timers\n\r");
	//uint8_t ButtonRead = 0; //Initialize ButtonRead variable
    semaphore_irq = xSemaphoreCreateBinaryStatic(&pxSemaphoreBuffer);
    configASSERT(semaphore_irq != NULL);
    mutex_sleep_capacity = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    configASSERT(mutex_sleep_capacity != NULL);

    printf_("After asserts\n\r");

    if( task_timer == NULL || usage_timer == NULL ){
        printf_("Timers was not created\n");
    }else{
        if( xTimerStart( task_timer, 0 ) != pdPASS || xTimerStart(usage_timer,0) != pdPASS)
        {
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

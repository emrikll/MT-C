#include "main.h"
#include "printf.h"
#include "projdefs.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include <stdint.h>
#include <math.h>

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
StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_sleep_capacity;

//Tick counters IRQ
uint64_t xStart, xEnd, xStartISR, xDifference, xDifferenceISR = 0;

//Tick counters IDLE
uint64_t xTimeInCPU, xTimeOutCPU, xDifferenceCPU, xTotalCPU = 0;

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

// Reverses a string 'str' of length 'len' 
void reverse(char* str, int len) 
{ 
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
} 
 
// Converts a given integer x to string str[]. 
// d is the number of digits required in the output. 
// If d is more than the number of digits in x, 
// then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
 
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
 
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 

// Converts a floating-point/double number to a string. 
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extract integer part 
    int ipart = (int)n; 
 
    // Extract floating part 
    float fpart = n - (float)ipart; 
 
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
 
    // check for display option after point 
    if (afterpoint != 0) { 
        res[i] = '.'; // add dot 
 
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter 
        // is needed to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
 
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
} 

/** Number on countu **/

int n_tu(int number, int count)
{
    int result = 1;
    while(count-- > 0)
        result *= number;

    return result;
}

/*** Convert float to string ***/
void float_to_string(float f, char r[])
{
    long long int length, length2, i, number, position, sign;
    float number2;

    sign = -1;   // -1 == positive number
    if (f < 0)
    {
        sign = '-';
        f *= -1;
    }

    number2 = f;
    number = f;
    length = 0;  // Size of decimal part
    length2 = 0; // Size of tenth

    /* Calculate length2 tenth part */
    while( (number2 - (float)number) != 0.0 && !((number2 - (float)number) < 0.0) )
    {
         number2 = f * (n_tu(10.0, length2 + 1));
         number = number2;

         length2++;
    }

    /* Calculate length decimal part */
    for (length = (f > 1) ? 0 : 1; f > 1; length++)
        f /= 10;

    position = length;
    length = length + 1 + length2;
    number = number2;
    if (sign == '-')
    {
        length++;
        position++;
    }

    for (i = length; i >= 0 ; i--)
    {
        if (i == (length))
            r[i] = '\0';
        else if(i == (position))
            r[i] = '.';
        else if(sign == '-' && i == 0)
            r[i] = '-';
        else
        {
            r[i] = (number % 10) + '0';
            number /=10;
        }
    }
}


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
    printf_("In IRQ handler\n\r");
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
            printf_("woo task!\n\r");
            xEnd = time_us();
            xDifference = xEnd - xStart;
            xDifferenceISR = xStartISR - xStart;
            char buffer[32];
            float_to_string(3.1, buffer);
            printf_(buffer);
            xStart = xEnd = xDifference = 0;
            if (toggle) {
                GPIO_ResetBits(LED_GPIO, RedLED_Pin);
            } else {
                GPIO_SetBits(LED_GPIO, RedLED_Pin);
            }

            toggle = ~toggle;
            if (xSemaphoreTake(mutex_sleep_capacity, portMAX_DELAY) == pdPASS){
                if (capacity_task_sleep < CAPACITY){
                    printf_("capacity added\n\r");
                    //xTaskCreateStatic(task_sleep, "SLEEP_TASK", 128, NULL,  1, xStack[capacity_task_sleep], &xTaskBuffer[capacity_task_sleep]);
                    capacity_task_sleep++;
                }
                xSemaphoreGive(mutex_sleep_capacity);
            }
        }else{
            printf_("Could not take Semaphore\n\r");
        }
        
    }
}

void task_cpu_average(TimerHandle_t timer) {
    int average_time = (AVERAGE_USAGE_INTERVAL_MS*1000);
    float usage = ( (float)average_time - (float)xTotalCPU) / (float)average_time;

    xTotalCPU = 0;

    printf_("HERE SHOULD CPU USAGE BE PRINTED\n\r");
}

/**
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeInCPU = time_us();
        printf_("Switched in to IDLE\n\r");
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
        printf_("Switched out\n\r");
        xTimeInCPU = xTimeOutCPU = xDifferenceCPU = 0;
     }
}

void led_timer_callback(TimerHandle_t timer) {
    printf_("In LED callback\n\r");

    xStart = time_us();
    
    EXTI_GenerateSWInterrupt(EXTI_Line0);
}



  
int main(void) {
    // Init hardware
    printf_("Hardware initialization\n\r");
    hardware_init();

    *((void (**)(void))0x2001C058) = EXTI0_IRQHandler;


    TaskHandle_t led_task = xTaskCreateStatic(task_led, "ToggleLED", 128, NULL, 1, xStackLed,&xTaskBufferLed);
    TimerHandle_t task_timer = xTimerCreate("LED_ON_TIMER", pdMS_TO_TICKS(LED_FLASH_PERIOD_MS), pdTRUE, (void*)TIMER_ID, led_timer_callback);
    TimerHandle_t usage_timer = xTimerCreate("CPU_UUSAGE_TIMER", pdMS_TO_TICKS(AVERAGE_USAGE_INTERVAL_MS), pdTRUE, (void*)TIMER_ID, task_cpu_average);

    semaphore_irq = xSemaphoreCreateBinary();
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

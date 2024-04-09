#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "printf.h"
#include "usart.h"
#include "timer.h"
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "utils/idle.h"
GPIO_InitTypeDef Gp;//Create GPIO struct

//Define LED pins
#define GreenLED_Pin GPIO_Pin_9
#define BlueLED_Pin GPIO_Pin_8
#define LED_GPIO GPIOC

//Define Push button
#define PushButton_Pin GPIO_Pin_0
#define PushButton_GPIO GPIOA   


/**
**===========================================================================
**
**  Abstract: SendPacket
**
**===========================================================================
*/

static StackType_t test_task2_stack[300];
static StaticTask_t test_task2_buffer;

void vTask1(void *vParameters){

  int toggle = 0;
  const TickType_t xDelay = 1000 / portTICK_PERIOD_MS - 1;
  for(;;)
  {
    
    if(toggle){
      //set bit
      GPIO_SetBits(LED_GPIO, GreenLED_Pin);
      toggle = 0;
    }else{
      //reset bit
      GPIO_ResetBits(LED_GPIO, GreenLED_Pin);
      toggle = 1;
    }
    uint32_t tim_value = time_us();
    float cpu_usage = cpu_usage_percent();
    printf_("test");
    vTaskDelay(xDelay);
  }
}


int main(void)
{
	//Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
    enable_usart();
    enable_timer();
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

	//uint8_t ButtonRead = 0; //Initialize ButtonRead variable
  
  xTaskCreateStatic(vTask1, 
              "ToggleLED", 
              40, 
              NULL, 
              1, 
              test_task2_stack,
              &test_task2_buffer);

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

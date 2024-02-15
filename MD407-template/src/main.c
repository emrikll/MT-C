#include <stdio.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

GPIO_InitTypeDef Gp;//Create GPIO struct

//Define LED pins
//PB0 Green LED
//PB1 Red LED
#define GreenLED_Pin GPIO_Pin_0
#define RedLED_Pin GPIO_Pin_1
#define LED_GPIO GPIOB

//Define Push button
#define PushButton_Pin GPIO_Pin_0
#define PushButton_GPIO GPIOA   

#define SystemCoreClock 168000000 
//extern void initialise_monitor_handles(void);

/**
**===========================================================================
**
**  Abstract: SendPacket
**
**===========================================================================
*/

void vTask1(void *vParameters){

  int toggle = 0;
  const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
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
      vTaskDelay(xDelay);
    }
  }


  int main(void)
  {
  //Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
  RCC_AHB3PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB3PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	Gp.GPIO_Pin = GreenLED_Pin | RedLED_Pin; //Set pins inside the struct
	Gp.GPIO_Mode = GPIO_Mode_OUT; //Set GPIO pins as output
	Gp.GPIO_OType = GPIO_OType_PP; //Ensure output is push-pull vs open drain
	Gp.GPIO_PuPd = GPIO_PuPd_NOPULL; //No internal pullup resistors required
	Gp.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_Speed_Level_1; //Set GPIO speed to lowest
	GPIO_Init(LED_GPIO, &Gp); //Assign struct to LED_GPIO

	Gp.GPIO_Pin = PushButton_Pin; //Set pins inside the struct
	Gp.GPIO_Mode = GPIO_Mode_IN; //Set GPIO pins as output
	Gp.GPIO_PuPd = GPIO_PuPd_NOPULL; //No pullup required as pullup is external
	GPIO_Init(PushButton_GPIO, &Gp); //Assign struct to LED_GPIO
  //
	//uint8_t ButtonRead = 0; //Initialize ButtonRead variable
  static StackType_t test_task2_stack[128];
	static StaticTask_t test_task2_buffer;
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

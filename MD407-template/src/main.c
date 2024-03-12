#include <stdio.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <FreeRTOS.h>
#include "system_stm32f4xx.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "FreeRTOSConfig.h"

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

#define SCB_VTOR_CUSTOM ((volatile unsigned long *) 0xE000ED08)

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
  GPIO_ResetBits(LED_GPIO, RedLED_Pin);

  const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
  for(;;)
  {
      print("woo task!\n");
      if (toggle) {
        GPIO_ResetBits(LED_GPIO, RedLED_Pin);
      } else {
        GPIO_SetBits(LED_GPIO, RedLED_Pin);
      }

      toggle = ~toggle;
      vTaskDelay(xDelay);
    }
  }

  
  int main(void)
  {
  //Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
	//uint8_t ButtonRead = 0; //Initialize ButtonRead variable
  //GPIO_ResetBits(LED_GPIO, RedLED_Pin);


  static StackType_t test_task2_stack[128];
	static StaticTask_t test_task2_buffer;

  
    
  print("lmao\n");
  printf("Int: %d", 3);
  xTaskCreateStatic(vTask1, 
              "ToggleLED", 
              40, 
              NULL, 
              1, 
              test_task2_stack,
              &test_task2_buffer);
  print("before start");
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

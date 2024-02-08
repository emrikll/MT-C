#include <stdio.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

GPIO_InitTypeDef Gp;//Create GPIO struct

//Define LED pins
#define GreenLED_Pin GPIO_Pin_9
#define BlueLED_Pin GPIO_Pin_8
#define LED_GPIO GPIOC

//Define Push button
#define PushButton_Pin GPIO_Pin_0
#define PushButton_GPIO GPIOA   

extern void initialise_monitor_handles(void);

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
	initialise_monitor_handles();
	//Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
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
  
  xTaskCreate(vTask1, 
              "ToggleLED", 
              40, 
              NULL, 
              1, 
              NULL);

  vTaskStartScheduler();

}

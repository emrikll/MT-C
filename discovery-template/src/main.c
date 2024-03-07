#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "usart.h"
#include "timer.h"
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "utils/idle.h"
#include "rng.h"
#include "print.h"
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

/*
 ** Tasks 
 
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
}*/
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
    printf("CPU usage: %f",cpu_usage);
    print("\n\r");
    uint16_t rand = get_random_byte();
    printf("Random byte: %u \n\r", rand);
    vTaskDelay(xDelay);
  }
}


int main(void)
{
	//Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
    enable_usart();
    enable_timer();
    adc_init();

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

#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "io.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
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
void vTask1(void *vParameters){

  int toggle = 0;
  const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
  for(;;)
  {
    printf("test");
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

void enable_usart(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;
	USART_InitTypeDef USART_InitStructure;

	GPIO_DeInit( GPIOA );
	USART_DeInit( USART1 );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);

	/* Connect USART pins to AF */
	/* PA9 - USART1 TX  */
	/* PA10 - USART1 RX */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);  
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);  
	/* PA11 - USART CTS not used */
	/* PA12 - USART RTS not used */

	GPIO_StructInit( &GPIO_InitStructure );


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 |  GPIO_Pin_9;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // should really be GPIO_OType_PP?
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  USART_StructInit( &USART_InitStructure );

   /* USARTx configured as follow:
         - BaudRate = 115200 baud  
         - Word Length = 8 Bits
         - One Stop Bit
         - No parity
         - Hardware flow control disabled (RTS and CTS signals)
         - Receive and transmit enabled
   */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( USART1, &USART_InitStructure);

	USART_ClockStructInit(&USART_ClockInitStructure);	
	USART_ClockInit(USART1, &USART_ClockInitStructure);

	USART_Cmd( USART1, ENABLE);	

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

}

int main(void)
{
	//Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
  enable_usart();
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

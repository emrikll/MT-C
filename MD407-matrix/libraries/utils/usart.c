#include "usart.h"
#include "stm32f4xx_gpio.h"

void enable_usart(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;
	USART_InitTypeDef USART_InitStructure;

	GPIO_DeInit( GPIOA );
	USART_DeInit( USART1 );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE);

	/* Connect USART pins to AF */
	/* PA9 - USART1 TX  */
	/* PA10 - USART1 RX */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  
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

int print(const char* str) {
    USART_ITConfig( USART1, USART_IT_RXNE, ENABLE);
    for (int i = 0; str[i] != '\0'; i++){
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData( USART1, str[i]);
    }
    return 0;
  }

void _putchar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);  
    USART_SendData( USART1, c);

}


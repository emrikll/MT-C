#include "usart.h"

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

int print(const char* str) {
    USART_ITConfig( USART1, USART_IT_RXNE, ENABLE);
    for (int i = 0; str[i] != '\0'; i++){
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData( USART1, str[i]);
    }
    return 0;
  }

  int printf(const char* str, ...)
  {
    // initializing list pointer 
    va_list ptr; 
    va_start(ptr, str); 
  
    // char array to store token 
    char token[1000]; 
    // index of where to store the characters of str in 
    // token 
    int k = 0; 
  
    // parsing the formatted string 
    for (int i = 0; str[i] != '\0'; i++) { 
        token[k++] = str[i]; 
  
        if (str[i + 1] == '%' || str[i + 1] == '\0') { 
            token[k] = '\0'; 
            k = 0; 
            if (token[0] != '%') { 
                char string[64];
                sprintf(
                    string, "%s", 
                    token); // printing the whole token if 
                            // it is not a format specifier
                print(string);
            } 
            else { 
                int j = 1; 
                char ch1 = 0; 
  
                // this loop is required when printing 
                // formatted value like 0.2f, when ch1='f' 
                // loop ends 
                while ((ch1 = token[j++]) < 58) { 
                } 
                // for integers 
                if (ch1 == 'i' || ch1 == 'd' || ch1 == 'u'
                    || ch1 == 'h') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, int));
                    print(string);
                } 
                // for characters 
                else if (ch1 == 'c') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, int)); 
                    print(string);
                } 
                // for float values 
                else if (ch1 == 'f') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, double));
                    print(string);
                } 
                else if (ch1 == 'l') { 
                    char ch2 = token[2]; 
  
                    // for long int 
                    if (ch2 == 'u' || ch2 == 'd'
                        || ch2 == 'i') {
                        char string[64];
                        sprintf(string, token, 
                                va_arg(ptr, long));

                        print(string);
                    } 
  
                    // for double 
                    else if (ch2 == 'f') {
                        char string[64];
                        sprintf(string, token, 
                                va_arg(ptr, double));
                        print(string);
                    } 
                } 
                else if (ch1 == 'L') { 
                    char ch2 = token[2]; 
  
                    // for long long int 
                    if (ch2 == 'u' || ch2 == 'd'
                        || ch2 == 'i') { 
                        char string[64];
                        sprintf(string, token, 
                                va_arg(ptr, long long));
                        print(string);
                    } 
  
                    // for long double 
                    else if (ch2 == 'f') {
                        char string[64];
                        sprintf(string,  token, 
                                va_arg(ptr, long double));
                        print(string);
                    } 
                } 
  
                // for strings 
                else if (ch1 == 's') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, char*)); 
                    print(string);
                } 
  
                // print the whole token 
                // if no case is matched 
                else { 
                    char string[64];
                    sprintf(string, "%s", token);
                    print(string);
                } 
            } 
        } 
    } 
  
    // ending traversal 
    va_end(ptr); 
    return 0; 
  }

#include <stdio.h>
#include <stdarg.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "stm32f4xx_usart.h"
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
      //set bit
    }
  }

  int print(const char* str) {
    USART_ITConfig( USART1, USART_IT_RXNE, ENABLE);
    for (int i = 0; str[i] != '\0'; i++){

      USART_SendData( USART1, str[i]);
      for (int i=1; i<10000; i++) {}
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
  int main(void)
  {
  //Enable clocks to both GPIOA (push button) and GPIOC (output LEDs)
	//uint8_t ButtonRead = 0; //Initialize ButtonRead variable
  //GPIO_ResetBits(LED_GPIO, RedLED_Pin);
  static StackType_t test_task2_stack[128];
	static StaticTask_t test_task2_buffer;

  USART_ITConfig( USART1, USART_IT_RXNE, ENABLE);
  print("hello\n");
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

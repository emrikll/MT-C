#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include <stdio.h>
#include <stdarg.h>

int printf(const char* str, ...);
int print(const char* str);

void enable_usart(void);

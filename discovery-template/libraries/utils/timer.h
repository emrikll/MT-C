#ifndef UTILS_TIMER_H
#define UTILS_TIMER_H

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

void enable_timer(void);
uint32_t time_us(void);

#endif

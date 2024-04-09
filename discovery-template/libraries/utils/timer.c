#include "timer.h"
#include "stm32f0xx_tim.h"

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

void enable_timer(void) {
    RCC_USARTCLKConfig(RCC_USART1CLK_SYSCLK);
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler = 
        (uint16_t) (clocks.PCLK_Frequency / 1000000) - 2; // To Hz
    TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF; // To Hz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    /* TIM Interrupts enable */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
}

uint32_t time_us(void) {
    return TIM_GetCounter(TIM2);
}

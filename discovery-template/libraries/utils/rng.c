#include "stm32f0xx_adc.h"
#include "stm32f0xx_gpio.h"
#include <stdint.h>

uint8_t get_random_byte() {
    uint8_t result = 0;
    for(int i = 0; i < 8; i++) {
        uint16_t rand = ADC_GetConversionValue(ADC1);
        result = result + ((rand & 1) << i);
        for(int j = 0; j < 10000; j++);
    }
    return result;
}

uint16_t get_random_bit() {
    return ADC_GetConversionValue(ADC1) & 1;
}

void adc_init(void){
    ADC_DeInit(ADC1);

  /* GPIOC Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    GPIO_InitTypeDef tmp_gpio;
    tmp_gpio.GPIO_Pin = GPIO_Pin_0;
    tmp_gpio.GPIO_Mode = GPIO_Mode_AN;
    tmp_gpio.GPIO_OType = GPIO_OType_PP;
    tmp_gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    tmp_gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &tmp_gpio);

    ADC_InitTypeDef tmp_adc;
    tmp_adc.ADC_Resolution = ADC_Resolution_12b;
    tmp_adc.ADC_ContinuousConvMode = ENABLE;
    tmp_adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    tmp_adc.ADC_DataAlign = ADC_DataAlign_Right;
    tmp_adc.ADC_ScanDirection = ADC_ScanDirection_Upward;
    ADC_Init(ADC1, &tmp_adc);

    /* ADC Calibration */
    ADC_GetCalibrationFactor(ADC1);
    asm volatile("nop; nop;");

    ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_1_5Cycles);

    ADC_Cmd(ADC1, ENABLE);

    /* Wait the ADRDY flag */
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)){

    }

    ADC_StartOfConversion(ADC1);
}

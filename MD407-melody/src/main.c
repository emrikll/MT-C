/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"
#include "projdefs.h"
#include "stm32f4xx_tim.h"
#include <stdint.h>

/*
 * GLOBALS
 */

uint32_t start_time;

// Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO;

// Capacity
int capacity_task_i_row = 0;
int CAPACITY = 250;

// Static allocation task_i_row

/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferMelody;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackMelody[STACK_SIZE];

TaskHandle_t handle_melody;

/*
 * DAC
 */
#define DAC_ADDRESS 0x4000741C
#define DAC_REGISTER ((volatile unsigned char *)(DAC_ADDRESS))

static void __dac_init() {
  GPIO_InitTypeDef GPIO_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;

  GPIO_DeInit( GPIOA );   // already done in __usart_init()

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // already done
  //in __usart_init()
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  GPIO_StructInit(&GPIO_InitStructure);

  /* DAC channel 2 (DAC_OUT2 = PA.5) configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  DAC_DeInit();

  DAC_StructInit(&DAC_InitStructure);

  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);
  DAC_Cmd(DAC_Channel_2, ENABLE);
  DAC_SetChannel2Data(DAC_Align_8b_R, 0);
}

/*
 * HARDWARE INTERRUPT
 */

void EnableTimerInterrupt() {
  NVIC_InitTypeDef nvicStructure;
  nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
  nvicStructure.NVIC_IRQChannelSubPriority = 1;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);
}

void TIM2_IRQHandler() {
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    BaseType_t higher_priority_task_woken = pdFALSE;
    /* Make sure that interrupt flag is set */
    /* ISR FUNCTION BODY USING A SEMAPHORE */

    // xStartISR = time_us_64();

    vTaskNotifyGiveFromISR(handle_melody, &higher_priority_task_woken);

    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }
}

/*
 * TASKS
 */

void task_melody(void *vParameters) {
  printf_("TEST\n\r");
  uint32_t i = 1;
  int volume = 3;
  int bin = 1;
  uint32_t lasttime = 0;

  while (1) {
    if (time_us() >= lasttime + 2024) {
      lasttime = time_us();
      char buffer[64];
      sprintf_(buffer, "%u\n\r", lasttime);
      printf_(buffer);
      if (bin) {
        *DAC_REGISTER = 0;
        bin = 0;
      } else {
        *DAC_REGISTER = volume;
        bin = 1;
      }
    }
  }
}

/*
 * RUNTIME START
 */

int main() {

  enable_timer();
  __dac_init();
  // EnableTimerInterrupt();

  *((void (**)(void))0x2001C0B0) = TIM2_IRQHandler;
  handle_melody = xTaskCreateStatic(task_melody, "MELODY", 128, NULL, 1,
                                    xStackMelody, &xTaskBufferMelody);

  // Start the FreeRTOS scheduler if any of the tasks are good
  start_time = time_us();
  // Start the scheduler
  vTaskStartScheduler();

  // We should never get here, but just in case...
  while (1) {
    // NOP
  };
}

/*
###############################################
    HELPER FUNCTIONS
###############################################
*/

/*
 * PERFORMANCE FUNCTIONS
 */
/**
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int *pxCurrentTCB) {
  TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
  if (handle == xTaskGetIdleTaskHandle()) {
    xTimeInPICO = time_us();
    // printf_("Switched in to IDLE");
  }
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int *pxCurrentTCB) {
  TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
  if (handle == xTaskGetIdleTaskHandle()) {
    char str[64];
    xTimeOutPICO = time_us();
    xDifferencePICO = xTimeOutPICO - xTimeInPICO;
    xTotalPICO = xTotalPICO + xDifferencePICO;
    sprintf_(str, "IDLE time: %lu", xDifferencePICO);
    // printf_(str);
    xTimeInPICO = xTimeOutPICO = xDifferencePICO = 0;
  }
}

/**
 * @brief Callback actioned when CPU usage timer fires.
 *
 * @param timer: The triggering timer.
 */
void task_cpu_usage(TimerHandle_t timer) {

  char str[64];
  int average_time = (AVERAGE_USAGE_INTERVAL_MS * 1000);
  float usage = ((float)average_time - (float)xTotalPICO) / (float)average_time;

  xTotalPICO = 0;

  sprintf_(str, "CPU USAGE: %f, BACKGROUND TASKS: %u", usage,
           capacity_task_i_row);
  printf_(str);
}

/*
 * FREERTOS
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  /* If the buffers to be provided to the Timer task are declared inside this
   * function then they must be declared static - otherwise they will be
   * allocated on the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

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

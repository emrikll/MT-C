/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "timer.h"
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

TaskHandle_t handle_melody = NULL;

/*
 * DAC
 */
#define DAC_ADDRESS 0x4000741C
#define DAC_REGISTER ((volatile unsigned char *)(DAC_ADDRESS))

// Custom vector to get IRQ working
#define SCB_VTOR_CUSTOM ((volatile unsigned long *)0xE000ED08)

// Semaphores
static SemaphoreHandle_t semaphore_irq = NULL;
StaticSemaphore_t xSemaphoreBuffer;

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
  NVIC_SetPriority(TIM2_IRQn, 2);
}

void TIM2_IRQHandler() {
  NVIC_ClearPendingIRQ(TIM2_IRQn);
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    BaseType_t higher_priority_task_woken = pdFALSE;
    /* Make sure that interrupt flag is set */
    /* ISR FUNCTION BODY USING A SEMAPHORE */

    // xStartISR = time_us_64();

    vTaskNotifyGiveFromISR(handle_melody, &higher_priority_task_woken);

    // Signal the alert clearance task
    //xSemaphoreGiveFromISR(semaphore_irq, &higher_priority_task_woken);
    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }
}

/*
 * TASKS
 */

void task_melody(void *vParameters) {
  printf_("Begin task\n\r");
  int volume = 40;
  int bin = 1;

  while (1) {
    //if (xSemaphoreTake(semaphore_irq, portMAX_DELAY) == pdPASS) {
      ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
      if (bin) {
        *DAC_REGISTER = 0;
        bin = 0;
      } else {
        *DAC_REGISTER = volume;
        bin = 1;
      }
    //}
  }
}

/*
 * RUNTIME START
 */

int main() {

  *((void (**)(void))0x2001C0B0) = TIM2_IRQHandler;

  printf_("Hardware begin\n\r");
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  EnableTimerInterrupt();

  semaphore_irq = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
  configASSERT(semaphore_irq != NULL);

  enable_timer();
  printf_("Hardware done\n\r");

  handle_melody = xTaskCreateStatic(task_melody, "MELODY", 128, NULL, 1,
                                    xStackMelody, &xTaskBufferMelody);

  // Start the FreeRTOS scheduler if any of the tasks are good
  // start_time = time_us();
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

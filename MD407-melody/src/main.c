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
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "timer.h"
#include <stdint.h>

/*
 * GLOBALS
 */

// Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO;

// Background Task
int capacity_background_task = 0;
#define CAPACITY 128
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_BACKGROUND 92

/* 64ructure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferBackground[CAPACITY];

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackBackground[CAPACITY][ STACK_SIZE_BACKGROUND ];

// Melody Task
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_MELODY 150

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferMelody;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackMelody[STACK_SIZE_MELODY];

TaskHandle_t handle_melody = NULL;

// Background Task
/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_SPAWN_BACKGROUND 150

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBufferSpawnBackground;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStackSpawnBackground[STACK_SIZE_MELODY];

TaskHandle_t handle_spawn_background = NULL;

/*
 * DAC
 */
#define DAC_ADDRESS 0x4000741C
#define DAC_REGISTER ((volatile unsigned char *)(DAC_ADDRESS))

// Custom vector to get IRQ working
#define SCB_VTOR_CUSTOM ((volatile unsigned long *)0xE000ED08)

// Semaphores
SemaphoreHandle_t semaphore_irq = NULL;
StaticSemaphore_t xSemaphoreBuffer;
StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t mutex_background_capacity = NULL;

/*
 * HARDWARE INTERRUPT
 */

TIM_TimeBaseInitTypeDef TIM5_TimeBaseStructure;

void enable_timer_tim5(void) {
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM5_TimeBaseStructure.TIM_Prescaler = 
        (uint16_t) (2 * RCC_ClocksStatus.PCLK1_Frequency / 1000000) - 1; // To Hz
    TIM5_TimeBaseStructure.TIM_Period = 0x000007E8; // To Hz
    TIM5_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM5_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM5_TimeBaseStructure);
    /* TIM Interrupts enable */
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    TIM_SetAutoreload(TIM5, 0x000007E8);
    /* TIM5 enable counter */
    TIM_Cmd(TIM5, ENABLE);
}

void EnableTimerInterrupt() {
  NVIC_InitTypeDef nvicStructure;
  nvicStructure.NVIC_IRQChannel = TIM5_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
  nvicStructure.NVIC_IRQChannelSubPriority = 1;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);
  NVIC_SetPriority(TIM5_IRQn, 2);
}

void TIM5_IRQHandler() {
  NVIC_ClearPendingIRQ(TIM5_IRQn);
  if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {

    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

    BaseType_t higher_priority_task_woken = pdFALSE;
    /* Make sure that interrupt flag is set */
    /* ISR FUNCTION BODY USING A SEMAPHORE */

    // xStartISR = time_us_64();

    vTaskNotifyGiveFromISR(handle_melody, &higher_priority_task_woken);

    // Signal the alert clearance task
    // xSemaphoreGiveFromISR(semaphore_irq, &higher_priority_task_woken);
    // Exit to context switch if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }
}

/*
 * TASKS
 */

void task_melody(void *vParameters) {
  printf_("Begin task\n\r");
  int volume = 10;
  int bin = 1;

  while (1) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    if (bin) {
      *DAC_REGISTER = 0;
      bin = 0;
    } else {
      *DAC_REGISTER = volume;
      bin = 1;
    }
  }
}

void task_background(void *vParameters) {
    uint32_t endtime = 0;
    const TickType_t xDelay = 1 / portTICK_PERIOD_MS;

    while (1) {
        endtime = time_us() + BACKGROUND_TASK_SLEEP_US;
        while(time_us() < endtime){}
        vTaskDelay(xDelay);
    }
}

void task_spawn_background(void *vParameters) {
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

  printf_("Begin task spawn\n\r");
    while (1) {
        if (capacity_background_task < CAPACITY) {
            xTaskCreateStatic(task_background, "BACKGROUND", 64, NULL, 1, xStackBackground[capacity_background_task], &xTaskBufferBackground[capacity_background_task]);
            capacity_background_task++;
            vTaskDelayUntil(&xLastWakeTime,xDelay);
        }else {
            vTaskSuspend(NULL);
        }
    }
}

/*
 * RUNTIME START
 */

int main() {

  *((void (**)(void))0x2001C108) = TIM5_IRQHandler;

  printf_("Hardware begin\n\r");
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  EnableTimerInterrupt();

  semaphore_irq = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
  configASSERT(semaphore_irq != NULL);

  enable_timer();
  enable_timer_tim5();
  printf_("Hardware done\n\r");

  handle_melody = xTaskCreateStatic(task_melody, "MELODY", 128, NULL, 1, xStackMelody, &xTaskBufferMelody);

  handle_spawn_background = xTaskCreateStatic(task_spawn_background, "SPAWN BACKGROUND", 128, NULL, 1, xStackSpawnBackground, &xTaskBufferSpawnBackground); 
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

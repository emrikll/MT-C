#include "idle.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"
#include "timer.h"

uint64_t xTimeInCPU, xTimeOutCPU, xTotalSleepCPU, xLastUsageCheck = 0;

/**
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeInCPU = time_us();
     }
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeOutCPU = time_us();
        uint64_t xDifferenceCPU = xTimeOutCPU - xTimeInCPU;
        xTotalSleepCPU = xTotalSleepCPU + xDifferenceCPU;
     }
}

float cpu_usage_percent(void) {
     uint32_t current_time = time_us();
     uint32_t time_elapsed = current_time - xLastUsageCheck;
     xLastUsageCheck = current_time;
     float usage = ( (float)time_elapsed - (float)xTotalSleepCPU) / (float)time_elapsed;
     xTotalSleepCPU = 0;
     return usage * 100;
}

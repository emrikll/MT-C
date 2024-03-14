#ifndef UTILS_IDLE_H
#define UTILS_IDLE_H

void handle_switched_in(int* pxCurrentTCB);
void handle_switched_out(int* pxCurrentTCB);
float cpu_usage_percent(void);

#endif

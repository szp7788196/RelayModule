#ifndef __TASK_MAIN_H
#define __TASK_MAIN_H

#include "sys.h"
#include "rtos_task.h"



extern TaskHandle_t xHandleTaskMAIN;

void vTaskMAIN(void *pvParameters);

void AutoLoopRegularTimeGroups(u16 *bit,u16 *state);



































#endif

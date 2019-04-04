#ifndef __TASK_KEY_H
#define __TASK_KEY_H

#include "sys.h"
#include "rtos_task.h"


#define SINGLE_CLICK	1
#define DOUBLE_CLICK	2
#define TRIPLE_CLICK	3

#define INVALID_TIME    35

extern TaskHandle_t xHandleTaskKEY;

void vTaskKEY(void *pvParameters);






































#endif

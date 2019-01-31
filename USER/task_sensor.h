#ifndef __TASK_SENSOR_H
#define __TASK_SENSOR_H

#include "sys.h"
#include "rtos_task.h"


extern TaskHandle_t xHandleTaskSENSOR;

void vTaskSENSOR(void *pvParameters);







#endif

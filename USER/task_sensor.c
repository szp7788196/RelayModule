#include "task_sensor.h"
#include "delay.h"
#include "common.h"
#include "rtc.h"
#include "usart.h"
#include "input.h"


TaskHandle_t xHandleTaskSENSOR = NULL;

void vTaskSENSOR(void *pvParameters)
{
	while(1)
	{
		AllRelayPowerState = GetAllInputState();		//获取各个继电器通道是否有电源输入

		AllRelayState = GetAllOutPutState();			//获取各个继电器的输出状态

		
		delay_ms(100);
	}
}



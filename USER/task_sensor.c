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
		AllRelayPowerState = GetAllInputState();		//��ȡ�����̵���ͨ���Ƿ��е�Դ����

		AllRelayState = GetAllOutPutState();			//��ȡ�����̵��������״̬


		delay_ms(100);
	}
}



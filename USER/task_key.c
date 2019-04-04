#include "task_key.h"
#include "key.h"
#include "delay.h"
#include "common.h"


TaskHandle_t xHandleTaskKEY = NULL;

void vTaskKEY(void *pvParameters)
{
	static u8 delay_cnt = 0;
	static u8 press_cnt = 0;
	u8 key_state = 0;
	
	while(1)
	{
		key_state = KEY_Scan(0);
		
		if(key_state)
		{
			delay_cnt = INVALID_TIME;
			press_cnt ++;
		}
		
		if(press_cnt != 0 && delay_cnt != 0)
		{
			delay_cnt --;
			
			if(delay_cnt == 0 || press_cnt == TRIPLE_CLICK)
			{
				if(xQueueSend(xQueue_key,(void *)&press_cnt,(TickType_t)10) != pdPASS)
				{
#ifdef DEBUG_LOG
					printf("send p_tSensorMsg fail 1.\r\n");
#endif
				}
				
				press_cnt = 0;
			}
		}

		delay_ms(10);
	}
}















































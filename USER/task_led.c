#include "task_led.h"
#include "led.h"
#include "delay.h"


TaskHandle_t xHandleTaskLED = NULL;

void vTaskLED(void *pvParameters)
{
	u32 cnt = 0;
	u8 led_state = 0;
	
	while(1)
	{
		if(cnt % 50 == 0)					//√ø∏Ù0.5√ÎŒπø¥√≈π∑
		{
			IWDG_Feed();
		}
		
		if(cnt % 30 == 0)
		{
			led_state = !led_state;
		}
		
		if(led_state)
		{
			GREEN_LED = 0;
		}
		else
		{
			GREEN_LED = 1;
		}
		
		cnt = (cnt + 1) & 0xFFFFFFFF;
		
		delay_ms(10);
	}
}







































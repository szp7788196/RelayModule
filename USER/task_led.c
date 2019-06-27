#include "task_led.h"
#include "led.h"
#include "delay.h"


TaskHandle_t xHandleTaskLED = NULL;

void vTaskLED(void *pvParameters)
{
	u32 cnt = 0;
	u8 g_led_state = 0;
	
	while(1)
	{
		if(cnt % 50 == 0)					//ÿ��0.5��ι���Ź�
		{
			IWDG_Feed();
		}
		
		if(cnt % 30 == 0)
		{
			g_led_state = !g_led_state;
		}
		
		if(g_led_state)
		{
			GREEN_LED = 1;
		}
		else
		{
			GREEN_LED = 0;
		}
		
		cnt = (cnt + 1) & 0xFFFFFFFF;
		
		delay_ms(10);
	}
}







































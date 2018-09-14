#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "usart.h"
#include "relay.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u8 MirrorLightLevelPercent = 0;
u16 MirrorOutPutControlBit = 0;


void vTaskMAIN(void *pvParameters)
{
//	time_t times_sec = 0;

//	times_sec = GetSysTick1s();

	while(1)
	{
		if(MirrorOutPutControlBit != OutPutControlBit)
		{
			MirrorOutPutControlBit = OutPutControlBit;
			
			ControlAllRelay(MirrorOutPutControlBit,&OutPutControlBitCh);
		}
		
		if(NeedToReset == 1)			//接收到重启的命令
		{
			NeedToReset = 0;
			delay_ms(1000);

			__disable_fault_irq();		//重启指令
			NVIC_SystemReset();
		}
		delay_ms(100);
	}
}



































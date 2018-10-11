#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "usart.h"
#include "relay.h"
#include "input.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u16 MirrorOutPutControlBitState = 0;
u16 MirrorAllRelayState = 0;


void vTaskMAIN(void *pvParameters)
{
	while(1)
	{
		if(MirrorOutPutControlBitState != OutPutControlState)
		{
			MirrorOutPutControlBitState = OutPutControlState;

			ControlAllRelay(MirrorOutPutControlBitState,&OutPutControlBit);
		}

		AllRelayPowerState = GetAllInputState();		//获取各个继电器通道是否有电源输入

		AllRelayState = GetAllOutPutState();			//获取各个继电器的输出状态

		if(MirrorAllRelayState != AllRelayState)
		{
			MirrorAllRelayState = AllRelayState;

			WriteAllRelayState();						//将继电器状态存储到EEPROM中
		}

		if(NeedToReset == 1)							//接收到重启的命令
		{
			NeedToReset = 0;
			delay_ms(1000);

			__disable_fault_irq();						//重启指令
			NVIC_SystemReset();
		}
		delay_ms(100);
	}
}



































#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "usart.h"
#include "relay.h"
#include "input.h"
#include "task_key.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u16 MirrorOutPutControlBitState = 0;
u16 MirrorAllRelayState = 0;


void vTaskMAIN(void *pvParameters)
{
	BaseType_t xResult;
	time_t times_sec = 0;
	u8 key_state = 0;
	
	USART2_Init(RS485BuadRate);
	
	while(1)
	{
		if(DeviceWorkMode == MODE_AUTO)					//只有在自动模式下才进行策略判断
		{
			if(GetSysTick1s() - times_sec >= 1)
			{
				times_sec = GetSysTick1s();

				AutoLoopRegularTimeGroups();
			}
		}
		
		xResult = xQueueReceive(xQueue_key,
							(void *)&key_state,
							(TickType_t)pdMS_TO_TICKS(1));
		if(xResult == pdPASS)
		{
			if(key_state == TRIPLE_CLICK)
			{
				OutPutControlState = 0x0000;
				OutPutControlBit = 0x0FFF;
				HaveNewActionCommand = 1;
			}
		}
		
		if(MirrorOutPutControlBitState != OutPutControlState || HaveNewActionCommand == 1)
		{
			MirrorOutPutControlBitState = OutPutControlState;
			HaveNewActionCommand = 0;

			ControlAllRelayDelay(MirrorOutPutControlBitState,&OutPutControlBit,RelayActionINCL);
		}

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

//轮询时间策略
void AutoLoopRegularTimeGroups(void)
{
	u8 ret = 0;
	u16 gate0 = 0;
	u16 gate1 = 0;
	u16 gate2 = 0;
	u16 gate24 = 1440;	//24*60;
	u16 gate_n = 0;
	
	pRegularTime tmp_time = NULL;

	if(GetTimeOK != 0)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
		
		if(calendar.week >= 1 && calendar.week <= 5)	//判断是否是工作日
		{
			if(RegularTimeWeekDay->next != NULL)		//判断策略列表是否不为空
			{
				for(tmp_time = RegularTimeWeekDay->next; tmp_time != NULL; tmp_time = tmp_time->next)	//轮训策略列表
				{
					if(tmp_time->hour 	== calendar.hour &&
					   tmp_time->minute == calendar.min)		//判断当前时间是否同该条策略时间相同
					{
						ret = 1;
					}
					else if(tmp_time->next != NULL)				//该条策略是不是最后一条
					{
						if(tmp_time->next->hour   == calendar.hour &&
					       tmp_time->next->minute == calendar.min)		//判断该条策略的next的时间是否与当前时间相同
						{
							tmp_time = tmp_time->next;
							
							ret = 1;
						}
						else
						{
							gate1 = tmp_time->hour * 60 + tmp_time->minute;					//该条策略的分钟数
							gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;		//该条策略的next的分钟数
							gate_n = calendar.hour * 60 + calendar.min;						//当前时间的分钟数

							if(gate1 < gate2)												//该条策略时间早于next的时间
							{
								if(gate1 <= gate_n && gate_n <= gate2)						//判断当前时间是否在两条策略时间段中间
								{
									ret = 1;
								}
							}
							else if(gate1 > gate2)											//该条策略时间晚于next的时间
							{
								if(gate1 <= gate_n && gate_n <= gate24)						//判断当前时间是否在该条策略时间和24点时间段中间
								{
									ret = 1;
								}
								else if(gate0 <= gate_n && gate_n <= gate2)					//判断当前时间是否在0点和next的时间段中间
								{
									ret = 1;
								}
							}
						}
					}
					else
					{
						ret = 1;
					}

					if(ret == 1)
					{
						OutPutControlBit = tmp_time->control_bit;
						OutPutControlState = tmp_time->control_state;

						break;
					}
				}
			}
		}
		else if(calendar.week >= 6 && calendar.week <= 7)
		{
			if(RegularTimeWeekEnd->next != NULL)
			{
				for(tmp_time = RegularTimeWeekEnd->next; tmp_time != NULL; tmp_time = tmp_time->next)
				{
					if(tmp_time->hour 	== calendar.hour &&
					   tmp_time->minute == calendar.min)
					{
						ret = 1;
					}
					else if(tmp_time->next != NULL)
					{
						if(tmp_time->next->hour   == calendar.hour &&
					       tmp_time->next->minute == calendar.min)
						{
							tmp_time = tmp_time->next;
							
							ret = 1;
						}
						else
						{
							gate1 = tmp_time->hour * 60 + tmp_time->minute;
							gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;
							gate_n = calendar.hour * 60 + calendar.min;

							if(gate1 < gate2)
							{
								if(gate1 <= gate_n && gate_n <= gate2)
								{
									ret = 1;
								}
							}
							else if(gate1 > gate2)
							{
								if(gate1 <= gate_n && gate_n <= gate24)
								{
									ret = 1;
								}
								else if(gate0 <= gate_n && gate_n <= gate2)
								{
									ret = 1;
								}
							}
						}
					}
					else
					{
						ret = 1;
					}

					if(ret == 1)
					{
						OutPutControlBit = tmp_time->control_bit;
						OutPutControlState = tmp_time->control_state;

						break;
					}
				}
			}
		}
		
		if(RegularTimeHoliday->next != NULL)
		{
			for(tmp_time = RegularTimeHoliday->next; tmp_time != NULL; tmp_time = tmp_time->next)
			{
				if(tmp_time->year + 2000 	== calendar.w_year &&
				   tmp_time->month 			== calendar.w_month &&
				   tmp_time->date 			== calendar.w_date &&
				   tmp_time->hour 			== calendar.hour &&
				   tmp_time->minute 		== calendar.min)
				{
					ret = 1;
				}
				else if(tmp_time->next != NULL)
				{
					if(tmp_time->next->hour   == calendar.hour &&
					   tmp_time->next->minute == calendar.min)
					{
						tmp_time = tmp_time->next;
						
						ret = 1;
					}
					else
					{
						gate1 = tmp_time->hour * 60 + tmp_time->minute;
						gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;
						gate_n = calendar.hour * 60 + calendar.min;

						if(gate1 < gate2)
						{
							if(gate1 <= gate_n && gate_n <= gate2)
							{
								ret = 1;
							}
						}
						else if(gate1 > gate2)
						{
							if(gate1 <= gate_n && gate_n <= gate24)
							{
								ret = 1;
							}
							else if(gate0 <= gate_n && gate_n <= gate2)
							{
								ret = 1;
							}
						}
					}
				}
				else
				{
					ret = 1;
				}

				if(ret == 1)
				{
					OutPutControlBit = tmp_time->control_bit;
					OutPutControlState = tmp_time->control_state;

					break;
				}
			}
		}
		
		xSemaphoreGive(xMutex_STRATEGY);
	}
}































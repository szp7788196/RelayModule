#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "usart.h"
#include "relay.h"
#include "input.h"
#include "task_key.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u16 MirrorOutPutControlState = 0;
u16 MirrorOutPutControlBit = 0;
u16 MirrorAllRelayState = 0;


void vTaskMAIN(void *pvParameters)
{
	BaseType_t xResult;
	time_t times_sec = 0;
	u8 key_state = 0;
	u8 calendar_date = 0xFF;
	Location_S location;

	USART2_Init(RS485BuadRate);

	GetTimeOK = GetSysTimeState();

	location.longitude = 0.0f;
	location.latitude = 0.0f;

	while(1)
	{
		if(GetTimeOK != 0)
		{
			//根据当前日期和经纬度计算日出日落时间
			if(calendar_date != calendar.w_date ||
			   location.longitude != Location.longitude ||
			   location.latitude != Location.latitude)
			{
				calendar_date = calendar.w_date;

				location.longitude = Location.longitude;
				location.latitude = Location.latitude;

				SunRiseSetTime = GetSunTime(calendar.w_year,
				                            calendar.w_month,
				                            calendar.w_date,
				                            Location.longitude,
				                            Location.latitude);
			}
		}

		if(GetSysTick1s() - times_sec >= 1)
		{
			times_sec = GetSysTick1s();

			if(DeviceWorkMode == MODE_AUTO)					//只有在自动模式下才进行策略判断
			{
				AutoLoopRegularTimeGroups(&OutPutControlBit,&OutPutControlState);
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

		if((MirrorOutPutControlState != OutPutControlState) ||
			(MirrorOutPutControlBit != OutPutControlBit) ||
			HaveNewActionCommand == 1)
		{
			MirrorOutPutControlState = OutPutControlState;
			MirrorOutPutControlBit = OutPutControlBit;
			HaveNewActionCommand = 0;

			ControlAllRelayDelay(OutPutControlState,&OutPutControlBit,RelayActionINCL);
		}

		if(MirrorAllRelayState != AllRelayState)
		{
			MirrorAllRelayState = AllRelayState;

			WriteAllRelayState();						//将继电器状态存储到EEPROM中
		}

		if(FrameWareState.state == FIRMWARE_DOWNLOADED)
		{
			delay_ms(2000);

			__disable_fault_irq();						//重启指令
			NVIC_SystemReset();
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
void AutoLoopRegularTimeGroups(u16 *bit,u16 *state)
{
	u8 ret = 0;
	u16 gate0 = 0;
	u16 gate1 = 0;
	u16 gate2 = 0;
	u16 gate24 = 1440;	//24*60;
	u16 gate_n = 0;
	u32 gate_day_s = 0;
	u32 gate_day_e = 0;
	u32 gate_day_n = 0;

	static u16 last_bit = 0;
	static u16 current_bit = 0;
	static u16 last_state = 0;
	static u16 current_state = 0;

	pRegularTime tmp_time = NULL;

	xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);

	if(RegularTimeHoliday->next != NULL)
	{
		ret = 0;

		for(tmp_time = RegularTimeHoliday->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
//			if(tmp_time->range.year_s <= calendar.w_year - 2000 &&
//			   calendar.w_year - 2000 <= tmp_time->range.year_e)		//当前年处于起始和结束年之间
//			{
//				if(tmp_time->range.month_s <= calendar.w_month &&
//				   calendar.w_month <= tmp_time->range.month_e)
//				{
//					if(tmp_time->range.date_s <= calendar.w_date &&
//					   calendar.w_date <= tmp_time->range.date_e)
//					{
						gate_day_s = get_days_form_calendar(tmp_time->range.year_s + 2000,tmp_time->range.month_s,tmp_time->range.date_s);

						gate_day_e = get_days_form_calendar(tmp_time->range.year_e + 2000,tmp_time->range.month_e,tmp_time->range.date_e);

						gate_day_n = get_days_form_calendar(calendar.w_year,calendar.w_month,calendar.w_date);

						if(gate_day_s <= gate_day_n && gate_day_n <= gate_day_e)
						{
							if(tmp_time->hour 	== calendar.hour &&
							   tmp_time->minute == calendar.min)		//判断当前时间是否同该条策略时间相同
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
						}
//					}
//				}
//			}

			if(ret == 1)
			{
				current_bit = tmp_time->control_bit;
				current_state = tmp_time->control_state;

				goto UPDATE_PERCENT;
			}
		}
	}

	if(calendar.week <= 6)				//判断是否是工作日
	{
		if(RegularTimeWeekDay->next != NULL)		//判断策略列表是否不为空
		{
			ret = 0;

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
					current_bit = tmp_time->control_bit;
					current_state = tmp_time->control_state;

					goto UPDATE_PERCENT;
				}
			}
		}
	}

	UPDATE_PERCENT:
	if(last_bit != current_bit ||
	   last_state != current_state ||
	   RefreshStrategy == 1)
	{
		last_bit = current_bit;
	    last_state = current_state;

		*bit = current_bit;
		*state = current_state;
	}

	xSemaphoreGive(xMutex_STRATEGY);
}





























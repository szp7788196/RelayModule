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
	u32 relay_state = 0;
	Location_S location;

	USART2_Init(RS485BuadRate);

	GetTimeOK = GetSysTimeState();

	location.longitude = 0.0f;
	location.latitude = 0.0f;

	while(1)
	{
		if(GetTimeOK != 0)
		{
			//���ݵ�ǰ���ں;�γ�ȼ����ճ�����ʱ��
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

				RefreshStrategySunRiseSetTime();	//ˢ�����в����ճ�����ʱ��
			}
		}

		if(GetSysTick1s() - times_sec >= 1)
		{
			times_sec = GetSysTick1s();

			if(DeviceWorkMode == MODE_AUTO)					//ֻ�����Զ�ģʽ�²Ž��в����ж�
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

		xResult = xQueueReceive(xQueue_RelayState,
							(void *)&relay_state,
							(TickType_t)pdMS_TO_TICKS(1));

		if(xResult == pdPASS)
		{
			OutPutControlBit = (u16)(relay_state >> 16);
			OutPutControlState = (u16)relay_state;
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

			WriteAllRelayState();						//���̵���״̬�洢��EEPROM��
		}

		if(FrameWareState.state == FIRMWARE_DOWNLOADED)
		{
			delay_ms(2000);

			__disable_fault_irq();						//����ָ��
			NVIC_SystemReset();
		}

		if(NeedToReset == 1)							//���յ�����������
		{
			NeedToReset = 0;
			delay_ms(1000);

			__disable_fault_irq();						//����ָ��
			NVIC_SystemReset();
		}
		delay_ms(100);
	}
}

//��ѯʱ�����
void AutoLoopRegularTimeGroups(u16 *bit,u16 *state)
{
	u8 ret = 0;
	u8 res = 0;
	u16 gate0 = 0;
	u16 gate1 = 0;
	u16 gate2 = 0;
	u16 gate24 = 1440;	//24*60;
	u16 gate_n = 0;

	static u8 minute = 255;

	static u16 last_bit = 0;
	static u16 current_bit = 0;
	static u16 last_state = 0;
	static u16 current_state = 0;

	pStrategyTime tmp_time = NULL;

	xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);

	if(RefreshStrategy == 1 ||
	   minute != calendar.min)
	{
		minute = calendar.min;

		res = GetCurrentStrategy();			//��ȡ��ǰ������
	}

	if(res == 0)						//û�з��������Ĳ�����
	{
		goto UPDATE_PERCENT;
	}

	if(CurrentStrategy->next != NULL)		//�жϲ����б��Ƿ�Ϊ��
	{
		for(tmp_time = CurrentStrategy->next; tmp_time != NULL; tmp_time = tmp_time->next)	//��ѵ�����б�
		{
			if(tmp_time->hour 	== calendar.hour &&
			   tmp_time->minute == calendar.min)		//�жϵ�ǰʱ���Ƿ�ͬ��������ʱ����ͬ
			{
				ret = 1;
			}
			else if(tmp_time->next != NULL)				//���������ǲ������һ��
			{
				if(tmp_time->next->hour   == calendar.hour &&
				   tmp_time->next->minute == calendar.min)		//�жϸ������Ե�next��ʱ���Ƿ��뵱ǰʱ����ͬ
				{
					tmp_time = tmp_time->next;

					ret = 1;
				}
				else
				{
					gate1 = tmp_time->hour * 60 + tmp_time->minute;					//�������Եķ�����
					gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;		//�������Ե�next�ķ�����
					gate_n = calendar.hour * 60 + calendar.min;						//��ǰʱ��ķ�����

					if(gate1 < gate2)												//��������ʱ������next��ʱ��
					{
						if(gate1 <= gate_n && gate_n <= gate2)						//�жϵ�ǰʱ���Ƿ�����������ʱ����м�
						{
							ret = 1;
						}
					}
					else if(gate1 > gate2)											//��������ʱ������next��ʱ��
					{
						if(gate1 <= gate_n && gate_n <= gate24)						//�жϵ�ǰʱ���Ƿ��ڸ�������ʱ���24��ʱ����м�
						{
							ret = 1;
						}
						else if(gate0 <= gate_n && gate_n <= gate2)					//�жϵ�ǰʱ���Ƿ���0���next��ʱ����м�
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

	UPDATE_PERCENT:
	if(last_bit != current_bit ||
	   last_state != current_state ||
	   RefreshStrategy == 1)
	{
		RefreshStrategy = 0;

		last_bit = current_bit;
	    last_state = current_state;

		*bit = current_bit;
		*state = current_state;
	}

	xSemaphoreGive(xMutex_STRATEGY);
}





























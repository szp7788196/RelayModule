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
		if(DeviceWorkMode == MODE_AUTO)					//ֻ�����Զ�ģʽ�²Ž��в����ж�
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

			WriteAllRelayState();						//���̵���״̬�洢��EEPROM��
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
		
		if(calendar.week >= 1 && calendar.week <= 5)	//�ж��Ƿ��ǹ�����
		{
			if(RegularTimeWeekDay->next != NULL)		//�жϲ����б��Ƿ�Ϊ��
			{
				for(tmp_time = RegularTimeWeekDay->next; tmp_time != NULL; tmp_time = tmp_time->next)	//��ѵ�����б�
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































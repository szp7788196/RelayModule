#include "common.h"
#include "rtos_task.h"
#include "24cxx.h"
#include "led.h"
#include "rtc.h"
#include "usart.h"
#include "tpic6c595.h"

//u16 i = 0;
//u8 eepbuf[256];
//u16 cnt = 0;
//u8 led_s = 0;
RCC_ClocksTypeDef RCC_Clocks;
int main(void)
{
//	IWDG_Init(IWDG_Prescaler_128,625);	//128分频 312.5HZ 625为2秒
	RCC_GetClocksFreq(&RCC_Clocks);		//查看各个总线的时钟频率
	__set_PRIMASK(1);	//关闭全局中断

	NVIC_Configuration();
	delay_init(72);
	RTC_Init();
	AT24CXX_Init();
	LED_Init();
	TIM2_Init(99,7199);
	USART1_Init(256000);
	USART2_Init(256000);
	TPIC6C595_Init();

	__set_PRIMASK(0);	//开启全局中断

//	for(i = 0; i < 256; i ++)
//	{
//		AT24CXX_WriteOneByte(i,i);
//	}
//	for(i = 0; i < 256; i ++)
//	{
//		eepbuf[i] = AT24CXX_ReadOneByte(i);
//	}


	mem_init();

	IWDG_Feed();				//喂看门狗

	ReadParametersFromEEPROM();	//读取所有的运行参数

	AppObjCreate();				//创建消息队列、互斥量
	AppTaskCreate();			//创建任务

	vTaskStartScheduler();		//启动调度，开始执行任务

	while(1)
	{
//		ControlAllRelay(0x0FFF);
//		delay_ms(1000);
//		ControlAllRelay(0x0000);
//		delay_ms(1000);
//		ControlAppointedRelay(1,1);
//		delay_ms(1000);
//		ControlAppointedRelay(2,1);
//		delay_ms(1000);
//		ControlAppointedRelay(3,1);
//		delay_ms(1000);
//		ControlAppointedRelay(4,1);
//		delay_ms(1000);
//		ControlAppointedRelay(5,1);
//		delay_ms(1000);
//		ControlAppointedRelay(6,1);
//		delay_ms(1000);
//		ControlAppointedRelay(7,1);
//		delay_ms(1000);
//		ControlAppointedRelay(8,1);
//		delay_ms(1000);
//		ControlAppointedRelay(9,1);
//		delay_ms(1000);
//		ControlAppointedRelay(10,1);
//		delay_ms(1000);
//		ControlAppointedRelay(11,1);
//		delay_ms(1000);
//		ControlAppointedRelay(12,1);
//		delay_ms(1000);
//		ControlAppointedRelay(1,0);
//		delay_ms(1000);
//		ControlAppointedRelay(2,0);
//		delay_ms(1000);
//		ControlAppointedRelay(3,0);
//		delay_ms(1000);
//		ControlAppointedRelay(4,0);
//		delay_ms(1000);
//		ControlAppointedRelay(5,0);
//		delay_ms(1000);
//		ControlAppointedRelay(6,0);
//		delay_ms(1000);
//		ControlAppointedRelay(7,0);
//		delay_ms(1000);
//		ControlAppointedRelay(8,0);
//		delay_ms(1000);
//		ControlAppointedRelay(9,0);
//		delay_ms(1000);
//		ControlAppointedRelay(10,0);
//		delay_ms(1000);
//		ControlAppointedRelay(11,0);
//		delay_ms(1000);
//		ControlAppointedRelay(12,0);
		delay_ms(1000);
	}
}


























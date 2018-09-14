#include "tpic6c595.h"


#include "delay.h"


void TPIC6C595_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |  RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	HC595_G_H;
	HC595_CLR_H;
}

void TPIC6C595WriteOneByte(u32 byte)
{
	u8 i;
	u32 num;
	num = byte;
	
	HC595_SRCK_L;
	HC595_G_H;
	
	for (i = 0; i < 24; i ++)
	{
		HC595_LRCK_L;
		
		if(num & 0x800000)
		{
			HC595_SDATA_H;
		}
		else
		{
			HC595_SDATA_L;
		}
		
		HC595_LRCK_H;
		
		num = num << 1;
	}
	
	TPIC6C595OutPut();
}

void TPIC6C595OutPut(void)
{
	HC595_SRCK_L;
	delay_us(1);
	HC595_SRCK_H;
	delay_us(1);
	HC595_SRCK_L;
	delay_us(1);
	HC595_G_L;
}






























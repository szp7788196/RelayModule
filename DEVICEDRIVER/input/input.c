#include "input.h"


void INPUT_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11 |\
									GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

//读取开入状态
u16 GetAllInputState(void)
{
	u16 state = 0;
	
	if(INPUT1)
	{
		state &= ~(1 << 0);
	}
	else
	{
		state |= (1 << 0);
	}
	
	if(INPUT2)
	{
		state &= ~(1 << 1);
	}
	else
	{
		state |= (1 << 1);
	}
	
	if(INPUT3)
	{
		state &= ~(1 << 2);
	}
	else
	{
		state |= (1 << 2);
	}
	
	if(INPUT4)
	{
		state &= ~(1 << 3);
	}
	else
	{
		state |= (1 << 3);
	}
	
	if(INPUT5)
	{
		state &= ~(1 << 4);
	}
	else
	{
		state |= (1 << 4);
	}
	
	if(INPUT6)
	{
		state &= ~(1 << 5);
	}
	else
	{
		state |= (1 << 5);
	}
	
	if(INPUT7)
	{
		state &= ~(1 << 6);
	}
	else
	{
		state |= (1 << 6);
	}
	
	if(INPUT8)
	{
		state &= ~(1 << 7);
	}
	else
	{
		state |= (1 << 7);
	}
	
	if(INPUT9)
	{
		state &= ~(1 << 8);
	}
	else
	{
		state |= (1 << 8);
	}
	
	if(INPUT10)
	{
		state &= ~(1 << 9);
	}
	else
	{
		state |= (1 << 9);
	}
	
	if(INPUT11)
	{
		state &= ~(1 << 10);
	}
	else
	{
		state |= (1 << 10);
	}
	
	if(INPUT12)
	{
		state &= ~(1 << 11);
	}
	else
	{
		state |= (1 << 11);
	}
	
	return state;
}

//读取开出状态
u16 GetAllOutPutState(void)
{
	u16 state = 0;
	
	if(INPUT1)
	{
		state &= ~(1 << 0);
	}
	else
	{
		state |= (1 << 0);
	}
	
	if(INPUT2)
	{
		state &= ~(1 << 1);
	}
	else
	{
		state |= (1 << 1);
	}
	
	if(INPUT3)
	{
		state &= ~(1 << 2);
	}
	else
	{
		state |= (1 << 2);
	}
	
	if(INPUT4)
	{
		state &= ~(1 << 3);
	}
	else
	{
		state |= (1 << 3);
	}
	
	if(INPUT5)
	{
		state &= ~(1 << 4);
	}
	else
	{
		state |= (1 << 4);
	}
	
	if(INPUT6)
	{
		state &= ~(1 << 5);
	}
	else
	{
		state |= (1 << 5);
	}
	
	if(INPUT7)
	{
		state &= ~(1 << 6);
	}
	else
	{
		state |= (1 << 6);
	}
	
	if(INPUT8)
	{
		state &= ~(1 << 7);
	}
	else
	{
		state |= (1 << 7);
	}
	
	if(INPUT9)
	{
		state &= ~(1 << 8);
	}
	else
	{
		state |= (1 << 8);
	}
	
	if(INPUT10)
	{
		state &= ~(1 << 9);
	}
	else
	{
		state |= (1 << 9);
	}
	
	if(INPUT11)
	{
		state &= ~(1 << 10);
	}
	else
	{
		state |= (1 << 10);
	}
	
	if(INPUT12)
	{
		state &= ~(1 << 11);
	}
	else
	{
		state |= (1 << 11);
	}
	
	return state;
}







































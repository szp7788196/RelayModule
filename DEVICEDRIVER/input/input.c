#include "input.h"
#include "delay.h"


void INPUT_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11 |\
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
	u8 i = 0;
	u8 input1 = 0;
	u8 input2 = 0;
	u8 input3 = 0;
	u8 input4 = 0;
	u8 input5 = 0;
	u8 input6 = 0;
	u8 input7 = 0;
	u8 input8 = 0;
	u8 input9 = 0;
	u8 input10 = 0;
	u8 input11 = 0;
	u8 input12 = 0;
	
	input1 = INPUT1;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input1 != INPUT1)
		{
			state |= (1 << 0);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 0);
		}
	}
	
	input2 = INPUT2;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input2 != INPUT2)
		{
			state |= (1 << 1);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 1);
		}
	}
	
	input3 = INPUT3;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input3 != INPUT3)
		{
			state |= (1 << 2);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 2);
		}
	}
	
	input4 = INPUT4;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input4 != INPUT4)
		{
			state |= (1 << 3);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 3);
		}
	}
	
	input5 = INPUT5;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input5 != INPUT5)
		{
			state |= (1 << 4);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 4);
		}
	}
	
	input6 = INPUT6;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input6 != INPUT6)
		{
			state |= (1 << 5);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 5);
		}
	}
	
	input7 = INPUT7;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input7 != INPUT7)
		{
			state |= (1 << 6);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 6);
		}
	}
	
	input8 = INPUT8;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input8 != INPUT8)
		{
			state |= (1 << 7);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 7);
		}
	}
	
	input9 = INPUT9;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input9 != INPUT9)
		{
			state |= (1 << 8);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 8);
		}
	}
	
	input10 = INPUT10;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input10 != INPUT10)
		{
			state |= (1 << 9);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 9);
		}
	}
	
	input11 = INPUT11;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input11 != INPUT11)
		{
			state |= (1 << 10);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 10);
		}
	}
	
	input12 = INPUT12;
	for(i = 0; i < 4; i ++)
	{
		delay_ms(5);
		
		if(input12 != INPUT12)
		{
			state |= (1 << 11);
			
			i = 4;
		}
		else
		{
			state &= ~(1 << 11);
		}
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







































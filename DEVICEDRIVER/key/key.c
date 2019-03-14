#include "key.h"
#include "delay.h"

void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//返回值：
//0，没有任何按键按下
//KEY0_PRES，KEY0按下
//KEY1_PRES，KEY1按下
//WKUP_PRES，WK_UP按下
//注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up=1;

	if(mode)
	{
		key_up=1;
	}
	if(key_up && (KEY1 == 0))
	{
		delay_ms(5);

		key_up=0;

		if(KEY1 == 0)
		{
			return KEY1_PRES;
		}
	}
	else if(KEY1 == 1)
	{
		key_up = 1;
	}

	return 0;
}


















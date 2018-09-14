#include "relay.h"
#include "delay.h"


//控制指定的继电器
void ControlAppointedRelay(u8 ch,u8 state)
{
	u32 RelayControlBit = 0;
	
	if(state == 1)
	{
		RelayControlBit |=  (1 << (ch * 2 - 1));
		RelayControlBit &= ~(1 << (ch * 2 - 2));
	}
	else if(state == 0)
	{
		RelayControlBit &= ~(1 << (ch * 2 - 1));
		RelayControlBit |=  (1 << (ch * 2 - 2));
	}
	
	TPIC6C595WriteOneByte(RelayControlBit);
	
	delay_ms(10);
	
	TPIC6C595WriteOneByte(0);
}

void ControlAllRelay(u16 out_put_control_bit,u16 *ch)
{
	u8 i = 0;
	u32 RelayControlBit = 0;
	
	for(i = 0; i < 12; i ++)
	{
		if(*ch & (1 << i))
		{
			if(out_put_control_bit & (1 << i))
			{
				RelayControlBit |=  (1 << (i * 2 + 1));
				RelayControlBit &= ~(1 << (i * 2 + 0));
			}
			else
			{
				RelayControlBit &= ~(1 << (i * 2 + 1));
				RelayControlBit |=  (1 << (i * 2 + 0));
			}
		}
	}
	
	*ch = 0;
	
	TPIC6C595WriteOneByte(RelayControlBit);
	
	delay_ms(10);
	
	TPIC6C595WriteOneByte(0);
}






























#include "relay.h"
#include "delay.h"


//控制指定的继电器
void ControlAppointedRelay(u8 ch,u8 state)
{
	u32 RelayControlBit = 0;

#ifndef FORWARD
	ch = CH_NUM  - ch;
#endif
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
	u16 relay_bit = 0;
	u16 relay_ch = 0;
	u32 RelayControlBit = 0;

	out_put_control_bit &= 0x0FFF;
	relay_bit = out_put_control_bit;
	relay_ch = *ch;

#ifndef FORWARD
	for(i = 0; i < CH_NUM; i ++)
	{
		if(out_put_control_bit & (1 << (CH_NUM - 1 - i)))
		{
			relay_bit |= (1 << i);
		}
		else
		{
			relay_bit &= ~(1 << i);
		}

		if(*ch & (1 << (CH_NUM - 1 - i)))
		{
			relay_ch |= (1 << i);
		}
		else
		{
			relay_ch &= ~(1 << i);
		}
	}
#endif

	for(i = 0; i < 12; i ++)
	{
		if(relay_ch & (1 << i))
		{
			if(relay_bit & (1 << i))
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

void ControlAllRelayDelay(u16 out_put_control_bit,u16 *ch,u16 ms)
{
	u8 i = 0;
	u16 relay_bit = 0;
	u16 relay_ch = 0;
	u32 RelayControlBit = 0;

	out_put_control_bit &= 0x0FFF;
	relay_bit = out_put_control_bit;
	relay_ch = *ch;

#ifndef FORWARD
	for(i = 0; i < CH_NUM; i ++)
	{
		if(out_put_control_bit & (1 << (CH_NUM - 1 - i)))
		{
			relay_bit |= (1 << i);
		}
		else
		{
			relay_bit &= ~(1 << i);
		}

		if(*ch & (1 << (CH_NUM - 1 - i)))
		{
			relay_ch |= (1 << i);
		}
		else
		{
			relay_ch &= ~(1 << i);
		}
	}
#endif

	for(i = 0; i < 12; i ++)
	{
		if(relay_ch & (1 << i))
		{
			if(relay_bit & (1 << i))
			{
				if(ms != 0)
				{
					ControlAppointedRelay(i,1);
				}
				else
				{
					RelayControlBit |=  (1 << (i * 2 + 1));
					RelayControlBit &= ~(1 << (i * 2 + 0));
				}
			}
			else
			{
				if(ms != 0)
				{
					ControlAppointedRelay(i,0);
				}
				else
				{
					RelayControlBit &= ~(1 << (i * 2 + 1));
					RelayControlBit |=  (1 << (i * 2 + 0));
				}
			}

			if(ms >= 10)
			{
				delay_ms(ms - 10);
			}
		}
	}

	*ch = 0;

	TPIC6C595WriteOneByte(RelayControlBit);

	delay_ms(10);

	TPIC6C595WriteOneByte(0);
}




























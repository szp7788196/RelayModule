#include "task_hci.h"
#include "delay.h"
#include "usart.h"
#include "at_protocol.h"
#include "net_protocol.h"


TaskHandle_t xHandleTaskHCI = NULL;

void vTaskHCI(void *pvParameters)
{
	u16 send_len1 = 0;
	u16 send_len2 = 0;
	u16 head_pos = 0xFFFF;
	u16 tail_pos = 0xFFFF;
	u16 frame_len = 0;
	u8 head_buf[8] = {0x68,0x00,0x00,0x00,0x00,0x00,0x02,0x68};
	u8 tail_buf[6] = {0xFE,0xFD,0xFC,0xFB,0xFA,0xF9};
	time_t time_s = 0;

	AT_CommandInit();

	UsartSendString(USART1,"READY\r\n", 7);

	while(1)
	{
		if(Usart1RecvEnd == 0xAA)
		{
			Usart1RecvEnd = 0;

			send_len1 = AT_CommandDataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf);
//			send_len1 = NetDataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf);

			memset(Usart1RxBuf,0,Usart1FrameLen);
		}

		if(send_len1 != 0)
		{
			UsartSendString(USART1,Usart1TxBuf, send_len1);

			memset(Usart1TxBuf,0,send_len1);

			send_len1 = 0;
		}

		if(Usart2RecvEnd == 0xAA)
		{
			Usart2RecvEnd = 0;

			time_s = GetSysTick1s();

			head_pos = MyStrstr(Usart2RxBuf, head_buf, Usart2FrameLen, 8);
			tail_pos = MyStrstr(Usart2RxBuf, tail_buf, Usart2FrameLen, 6);

			if(head_pos != 0xFFFF && tail_pos != 0xFFFF)
			{
				if(tail_pos > head_pos)
				{
					frame_len = tail_pos + 6 - head_pos;

					send_len2 = NetDataAnalysis(&Usart2RxBuf[head_pos],frame_len,Usart2TxBuf);
				}

				OldUsart2RxCnt = 0;
				Usart2RxCnt = 0;

				memset(Usart2RxBuf,0,Usart2FrameLen);

				Usart2FrameLen = 0;
			}
			else if(Usart2RxBuf[0] == 'A' && Usart2RxBuf[1] == 'T')
			{
				send_len2 = AT_CommandDataAnalysis(Usart2RxBuf,Usart2FrameLen,Usart2TxBuf);

				OldUsart2RxCnt = 0;
				Usart2RxCnt = 0;

				memset(Usart2RxBuf,0,Usart2FrameLen);

				Usart2FrameLen = 0;
			}
		}

		if(Usart2FrameLen != 0)
		{
			if(GetSysTick1s() - time_s >= 5)	//接收到数据但是不完整，超时n秒数据作废
			{
				OldUsart2RxCnt = 0;
				Usart2RxCnt = 0;

				memset(Usart2RxBuf,0,Usart2FrameLen);

				Usart2FrameLen = 0;
			}
		}

		if(send_len2 != 0)
		{
			DIR_485_TX;

			UsartSendString(USART2,Usart2TxBuf, send_len2);

			DIR_485_RX;

			memset(Usart2TxBuf,0,send_len2);

			send_len2 = 0;
		}

		delay_ms(50);
	}
}







































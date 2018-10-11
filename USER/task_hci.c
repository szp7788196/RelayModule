#include "task_hci.h"
#include "delay.h"
#include "usart.h"
#include "at_protocol.h"


TaskHandle_t xHandleTaskHCI = NULL;

void vTaskHCI(void *pvParameters)
{
	u16 send_len1 = 0;
	u16 send_len2 = 0;
	
	AT_CommandInit();
	
	UsartSendString(USART1,"READY\r\n", 7);
	
	while(1)
	{
		if(Usart1RecvEnd == 0xAA)
		{
			Usart1RecvEnd = 0;
			
			send_len1 = AT_CommandDataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf,HoldReg);
			
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
			
			send_len2 = AT_CommandDataAnalysis(Usart2RxBuf,Usart2FrameLen,Usart2TxBuf,HoldReg);
			
			memset(Usart2RxBuf,0,Usart2FrameLen);
		}
		
		if(send_len2 != 0)
		{
			DIR_485_TX;
			
			UsartSendString(USART2,Usart2TxBuf, send_len2);
			
			DIR_485_RX;
			
			memset(Usart2TxBuf,0,send_len2);
			
			send_len2 = 0;
		}
		
		delay_ms(100);
	}
}







































#include "at_protocol.h"
#include "usart.h"
#include "common.h"

u8 AT_ECHO = 0;
AT_Command_S AT_CommandBuf[AT_MAX_NUM];

void AT_CommandInit(void)
{
	u8 i = 0;
	
	AT_CommandBuf[i].id = i;
	AT_CommandBuf[i].len = strlen("RST");
	AT_CommandBuf[i].cmd = (char *)mymalloc(sizeof(char) * AT_CommandBuf[i].len + 1);
	memset(AT_CommandBuf[i].cmd,0,AT_CommandBuf[i].len);
	memcpy(AT_CommandBuf[i].cmd,"RST",AT_CommandBuf[i].len);
	i ++;
	
	AT_CommandBuf[i].id = i;
	AT_CommandBuf[i].len = strlen("GMR");
	AT_CommandBuf[i].cmd = (char *)mymalloc(sizeof(char) * AT_CommandBuf[i].len + 1);
	memset(AT_CommandBuf[i].cmd,0,AT_CommandBuf[i].len);
	memcpy(AT_CommandBuf[i].cmd,"GMR",AT_CommandBuf[i].len);
	i ++;
	
	AT_CommandBuf[i].id = i;
	AT_CommandBuf[i].len = strlen("DEVNAME");
	AT_CommandBuf[i].cmd = (char *)mymalloc(sizeof(char) * AT_CommandBuf[i].len + 1);
	memset(AT_CommandBuf[i].cmd,0,AT_CommandBuf[i].len);
	memcpy(AT_CommandBuf[i].cmd,"DEVNAME",AT_CommandBuf[i].len);
	i ++;
	
	AT_CommandBuf[i].id = i;
	AT_CommandBuf[i].len = strlen("DEVID");
	AT_CommandBuf[i].cmd = (char *)mymalloc(sizeof(char) * AT_CommandBuf[i].len + 1);
	memset(AT_CommandBuf[i].cmd,0,AT_CommandBuf[i].len);
	memcpy(AT_CommandBuf[i].cmd,"DEVID",AT_CommandBuf[i].len);
	i ++;
	
	AT_CommandBuf[i].id = i;
	AT_CommandBuf[i].len = strlen("UUID");
	AT_CommandBuf[i].cmd = (char *)mymalloc(sizeof(char) * AT_CommandBuf[i].len + 1);
	memset(AT_CommandBuf[i].cmd,0,AT_CommandBuf[i].len);
	memcpy(AT_CommandBuf[i].cmd,"UUID",AT_CommandBuf[i].len);
	i ++;
	
	AT_CommandBuf[i].id = i;
	AT_CommandBuf[i].len = strlen("RELAY");
	AT_CommandBuf[i].cmd = (char *)mymalloc(sizeof(char) * AT_CommandBuf[i].len + 1);
	memset(AT_CommandBuf[i].cmd,0,AT_CommandBuf[i].len);
	memcpy(AT_CommandBuf[i].cmd,"RELAY",AT_CommandBuf[i].len);
}


//AT指令帧协议解析,由串口1控制单灯控制器
u16 AT_CommandDataAnalysis(u8 *inbuf,u16 inbuf_len,u8 *outbuf,u8 *hold_reg)
{
	u8 i = 0;
	u16 ret = 0;
	u8 err_code = 1;
	u8 cmd_id = 255;
	u8 respbuf[256];
	
//	xSemaphoreTake(xMutex_AT_COMMAND, portMAX_DELAY);
	
	if((MyStrstr(inbuf, (u8 *)"AT", inbuf_len, 2) != 0xFFFF || \
		MyStrstr(inbuf, (u8 *)"AT+", inbuf_len, 3) != 0xFFFF) &&\
		MyStrstr(inbuf, (u8 *)"\r\n", inbuf_len, 2) != 0xFFFF)
	{
		memset(respbuf,0,256);
		
		if(inbuf_len == 4 || inbuf_len == 6)
		{
			if(inbuf_len == 4)
			{
				err_code = 0;
			}
			
			if(inbuf_len == 6)
			{
				if(MyStrstr(inbuf, (u8 *)"ATE0", inbuf_len, 4) != 0xFFFF)
				{
					AT_ECHO = 0;
					err_code = 0;
				}
				else if(MyStrstr(inbuf, (u8 *)"ATE1", inbuf_len, 4) != 0xFFFF)
				{
					AT_ECHO = 1;
					err_code = 0;
				}
			}
		}
		else
		{
			for(i = 0; i < AT_MAX_NUM; i ++)
			{
				if(MyStrstr(inbuf, (u8 *)AT_CommandBuf[i].cmd, inbuf_len, AT_CommandBuf[i].len) != 0xFFFF)
				{
					if(inbuf_len == AT_CommandBuf[i].len + 3 + 2 || \
						MyStrstr(inbuf, (u8 *)"=", inbuf_len, 1) != 0xFFFF)
					{
						cmd_id = i;
					}
					
					break;
				}
			}
			
			switch(cmd_id)
			{
				case RST:
					err_code = AT_CommandRST(cmd_id,inbuf,inbuf_len,respbuf);
				break;
				
				case GMR:
					err_code = AT_CommandGMR(cmd_id,inbuf,inbuf_len,respbuf);
				break;
				
				case DEVNAME:
					err_code = AT_CommandDEVNAME(cmd_id,inbuf,inbuf_len,respbuf);
				break;
				
				case DEVID:
					err_code = AT_CommandDEVID(cmd_id,inbuf,inbuf_len,respbuf);
				break;
				
				case UUID:
					err_code = AT_CommandUUID(cmd_id,inbuf,inbuf_len,respbuf);
				break;
				
				case RELAY:
					err_code = AT_CommandRELAY(cmd_id,inbuf,inbuf_len,respbuf);
				break;
				
				default:
					
				break;
			}
		}
		
		ret = PackAT_CommandRespone(inbuf,err_code,respbuf,outbuf);
	}
	
//	xSemaphoreGive(xMutex_AT_COMMAND);
	
	return ret;
}

//打包AT指令回复数据
u8 PackAT_CommandRespone(u8 *inbuf,u8 err_code,u8 *respbuf,u8 *outbuf)
{
	u8 len = 0;
	
	if(AT_ECHO)	//开启回显
	{
		sprintf((char *)outbuf, "%s",inbuf);		//填充回显
	}
	
	if(strlen((char *)respbuf))
	{
		strcat((char *)outbuf,(char *)respbuf);		//填充返回的内容
	}
	
	if(err_code == 0)
	{
		strcat((char *)outbuf,"\r\nOK\r\n");		//填充结果OK
	}
	else
	{
		strcat((char *)outbuf,"\r\nERROR\r\n");		//填充结果ERROR
	}
	
	len = strlen((char *)outbuf);
	
	return len;
}

//复位
u8 AT_CommandRST(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf)
{
	u8 ret = 0;
	
	NeedToReset = 1;
	
	return ret;
}

//获取版本信息
u8 AT_CommandGMR(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf)
{
	u8 ret = 1;
	
	if(inbuf_len == AT_CommandBuf[cmd_id].len + 3 + 2)
	{
		sprintf((char *)respbuf, "software: %s\r\nhardware: %s",SoftWareVersion,HardWareVersion);
		ret = 0;
	}
	
	return ret;
}

//获取/设置设备名称
u8 AT_CommandDEVNAME(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf)
{
	u8 ret = 1;
	u8 content[34];

	if(inbuf_len == AT_CommandBuf[cmd_id].len + 3 + 2)
	{
		if(DeviceName != NULL)
		{
			sprintf((char *)respbuf, "device name: %s\r\n",DeviceName);
		}
		else
		{
			sprintf((char *)respbuf, "device name: null\r\n");
		}
		ret = 0;
	}
	else if(MyStrstr(inbuf, (u8 *)"=\"", inbuf_len, 2) != 0xFFFF)
	{
		memset(content,0,34);
		
		if(get_str1(inbuf, "\"", 1, "\"", 2, &content[1]))
		{
			content[0] = strlen((char *)&content[1]);
			
			if(content[0] <= 32)
			{
				CopyStrToPointer(&DeviceName, &content[1],content[0]);
				
				memcpy(&HoldReg[DEVICE_NAME_ADD],content,DEVICE_NAME_LEN - 2);
				
				WriteDataFromHoldBufToEeprom(&HoldReg[DEVICE_NAME_ADD],DEVICE_NAME_ADD, DEVICE_NAME_LEN - 2);
				
				ret = 0;
			}
		}
	}
	
	return ret;
}

//获取/设置设备ID
u8 AT_CommandDEVID(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf)
{
	u8 ret = 1;
	u8 i = 0;
	u8 content_str[13];
	u8 content_hex[6];
	u8 content_str_len = 0;

	if(inbuf_len == AT_CommandBuf[cmd_id].len + 3 + 2)
	{
		if(DeviceID != NULL)
		{
			sprintf((char *)respbuf, "device id: %02x%02x%02x%02x%02x%02x\r\n",\
				DeviceID[0],DeviceID[1],DeviceID[2],DeviceID[3],DeviceID[4],DeviceID[5]);
		}
		else
		{
			sprintf((char *)respbuf, "device id: null\r\n");
		}
		
		ret = 0;
	}
	else if(MyStrstr(inbuf, (u8 *)"=\"", inbuf_len, 2) != 0xFFFF)
	{
		memset(content_str,0,13);
		memset(content_hex,0,6);
		
		if(get_str1(inbuf, "\"", 1, "\"", 2, content_str))
		{
			content_str_len = strlen((char *)content_str);
			
			for(i = 0; i < 6; i ++)
			{
				content_hex[i] = (content_str[i * 2 + 0] - 0x30) * 16 + (content_str[i * 2 + 1] - 0x30);
			}
			
			if(content_str_len == 12)
			{
				CopyStrToPointer(&DeviceID, content_hex,content_str_len / 2);
				
				memcpy(&HoldReg[DEVICE_ID_ADD],content_hex,DEVICE_ID_ADD - 2);
				
				WriteDataFromHoldBufToEeprom(&HoldReg[DEVICE_ID_ADD],DEVICE_ID_ADD, DEVICE_ID_LEN - 2);
				
				ret = 0;
			}
		}
	}
	
	return ret;
}

//获取/设置UUID
u8 AT_CommandUUID(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf)
{
	u8 ret = 1;
	u8 content[37];
	u8 content_len = 0;

	if(inbuf_len == AT_CommandBuf[cmd_id].len + 3 + 2)
	{
		if(DeviceUUID != NULL)
		{
			sprintf((char *)respbuf, "uuid: %s\r\n",DeviceUUID);
		}
		else
		{
			sprintf((char *)respbuf, "uuid: null\r\n");
		}
		ret = 0;
	}
	else if(MyStrstr(inbuf, (u8 *)"=\"", inbuf_len, 2) != 0xFFFF)
	{
		memset(content,0,37);
		
		if(get_str1(inbuf, "\"", 1, "\"", 2, content))
		{
			content_len = strlen((char *)content);
			
			if(content_len == 36)
			{
				CopyStrToPointer(&DeviceUUID, content,content_len);
				
				memcpy(&HoldReg[UU_ID_ADD],content,UU_ID_ADD - 2);
				
				WriteDataFromHoldBufToEeprom(&HoldReg[UU_ID_ADD],UU_ID_ADD, UU_ID_LEN - 2);
				
				ret = 0;
			}
		}
	}
	
	return ret;
}

//控制继电器
u8 AT_CommandRELAY(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf)
{
	u8 ret = 1;
	u8 ch;
	u8 state;
//	s8 bin[16];

	if(inbuf_len == AT_CommandBuf[cmd_id].len + 3 + 2)
	{
//		memset(bin,0,16);
		
//		itoa(OutPutControlBit, bin, 2);
		
		sprintf((char *)respbuf, "relay: %x\r\n",OutPutControlState);
		ret = 0;
	}
	else if(inbuf_len == AT_CommandBuf[cmd_id].len +3 + 2 + 2 + 2&& \
		MyStrstr(inbuf, (u8 *)"=", inbuf_len, 1) != 0xFFFF)
	{
		ch = *(inbuf + inbuf_len - 5);
		
		if(ch <= 0x39)
		{
			ch = ch - 0x30;
		}
		else
		{
			ch = ch - 0x41 + 10;
		}
		if(ch <= 12)
		{
			state = *(inbuf + inbuf_len - 3) - 0x30;
		
			if(state < 2)
			{
				if(state == 0)
				{
					OutPutControlState &= ~(1 << ch - 1);
				}
				else if(state == 1)
				{
					OutPutControlState |= (1 << ch - 1);
				}
				
				OutPutControlBit |= (1 << ch - 1);
			
				HaveNewActionCommand = 1;
				
				ret = 0;
			}
		}
	}
	
	return ret;
}




















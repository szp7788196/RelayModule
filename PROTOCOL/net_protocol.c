#include "net_protocol.h"
#include "rtc.h"
#include "usart.h"
#include "24cxx.h"
#include "common.h"

//网络数据帧协议解析
u16 NetDataAnalysis(u8 *buf,u16 len,u8 *outbuf,u8 *hold_reg)
{
	u16 ret = 0;
	u16 pos1 = 0;
	u16 pos2 = 0xFFFF;
	u16 data_len = 0;

	u8 cmd_code = 0;
	u8 cmd_id = 0;
	u8 read_check_sum = 0;
	u8 cal_check_sum = 0;

	u8 buf_tail[6] = {0xFE,0xFD,0xFC,0xFB,0xFA,0xF9};

	pos1 = MyStrstr(buf,buf_tail,len,6);

	if(DeviceUUID != NULL)
	{
		pos2 = MyStrstr(buf,DeviceUUID,len,UU_ID_LEN - 2);
	}

	cmd_code = *(buf + 8);								//获取功能码
	
	if(pos1 != 0xFFFF && (pos2 != 0xFFFF || cmd_code == 0xD5))
	{
		if(*(buf + 0) == 0x68 && \
			*(buf + 7) == 0x68 && \
			*(buf + pos1 - 1) == 0x16)							//判断包头和包尾
		{
			cmd_id = *(buf + 9);								//命令ID
			data_len = DeviceUUID != NULL ? (*(buf + 10) - UU_ID_LEN - 2) : (*(buf + 10));	//获取有效数据的长度
			read_check_sum = *(buf + pos1 - 2);					//获取校验和
			cal_check_sum = CalCheckSum(buf, pos1 - 2);			//计算校验和

			if(read_check_sum == cal_check_sum)
			{
				switch(cmd_code)
				{
					case 0xD0:									//发送固定信息(心跳)，上行
						ret = UpdateRelayModeInfo(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0xD1:									//控制继电器开闭状态
						ret = ControlRelayState(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0xD2:									//开关灯/调光，下行
						ret = ControlDeviceReset(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0xD3:									//设置定时发送间隔,下行
						ret = SetDeviceUpLoadINCL(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0xD4:									//读取/发送信息

					break;

					case 0xD5:									//从服务器获取时间
						ret = GetTimeDateFromServer(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0xD6:									//设置继电器定时策略，下行
						ret = SetRegularTimeGroups(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0xD7:									//设置设备工作模式
						ret = SetDeviceWorkMode(cmd_code,cmd_id,buf + 10 + 36,data_len,outbuf);
					break;

					case 0x80:									//应答，下行,上行在别处处理
						UnPackAckPacket(cmd_code,cmd_id,buf + 10 + 36,data_len);
					break;

					default:									//此处要给云端应答一个功能码错误信息

					break;
				}
			}
		}
		else	//此处可以给云端应答一个校验错误信息
		{

		}
	}
	else		//此处可以给云端应答一个校验错误信息
	{

	}

	return ret;
}

//解析ACK包
u8 UnPackAckPacket(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len)
{
	u8 ret = 0;

	if(len == 2)
	{
		if(*(buf + 1) == 0)
		{
			ret = 1;
		}
	}

	return ret;
}

//ACK打包
u16 PackAckPacket(u8 cmd_code,u8 cmd_id,u8 *data,u8 *outbuf)
{
	u16 len = 0;

	len = PackNetData(0x80,cmd_id,data,2,outbuf);

	return len;
}

u16 UpdateRelayModeInfo(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;

	u8 data_buf[8] = {0,0,0,0,0,0,0,0};

	out_len = PackDataOfRelayInfo(data_buf);
	out_len = PackNetData(cmd_code,cmd_id,data_buf,out_len,outbuf);

	return out_len;
}

//控制继电器开闭
u16 ControlRelayState(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u16 bit = 0;
	u16 state = 0;

	data_buf[0] = cmd_code;

	if(len == 4)												//数据长度必须是64
	{
		bit = ((u16)(*(buf + 0)) << 16) + (*(buf + 1));
		state = ((u16)(*(buf + 2)) << 16) + (*(buf + 3));

		if(bit < 0x0FFF && state < 0x0FFF)
		{
			OutPutControlBit = bit;
			OutPutControlState = state;
		}
		else
		{
			data_buf[1] = 1;
		}
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//下发更新固件命令
u16 SetUpdateFirmWareInfo(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	data_buf[0] = cmd_code;

	if(len == 5)
	{
		NewFirmWareVer    = (((u16)(*(buf + 0))) << 8) + (u16)(*(buf + 1));
		NewFirmWareBagNum = (((u16)(*(buf + 2))) << 8) + (u16)(*(buf + 3));
		LastBagByteNum    = *(buf + 4);

		if(NewFirmWareBagNum == 0 || NewFirmWareBagNum > MAX_FW_BAG_NUM \
			|| NewFirmWareVer == 0 || NewFirmWareVer > MAX_FW_VER \
			|| LastBagByteNum == 0 || LastBagByteNum > MAX_FW_LAST_BAG_NUM)  //128 + 2 + 4 = 134
		{
			data_buf[1] = 1;
		}
		else
		{
			HaveNewFirmWare = 0xAA;
			if(NewFirmWareAdd == 0xAA)
			{
				NewFirmWareAdd = 0x55;
			}
			else if(NewFirmWareAdd == 0x55)
			{
				NewFirmWareAdd = 0xAA;
			}
			else
			{
				NewFirmWareAdd = 0xAA;
			}

			WriteOTAInfo(HoldReg,0);		//将数据写入EEPROM

			NeedToReset = 1;				//重新启动
		}
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//远程重启
u16 ControlDeviceReset(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	data_buf[0] = cmd_code;

	if(len == 0)
	{
		NeedToReset = 1;
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//设置设备数据上传时间间隔
u16 SetDeviceUpLoadINCL(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u16 incl;

	data_buf[0] = cmd_code;

	if(len == 2)												//数据长度必须是64
	{
		incl = ((u16)(*(buf + 0)) << 16) + (*(buf + 1));

		if(incl <= MAX_UPLOAD_INVL)
		{
			UpLoadINCL = incl;

			memcpy(&HoldReg[UPLOAD_INVL_ADD],buf,2);
			WriteDataFromHoldBufToEeprom(&HoldReg[UPLOAD_INVL_ADD],UPLOAD_INVL_ADD, UPLOAD_INVL_LEN - 2);
		}
		else
		{
			data_buf[1] = 1;
		}
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//从服务器获取时间戳
u16 GetTimeDateFromServer(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u8 year = 0;
	u8 mon = 0;
	u8 day = 0;
	u8 hour = 0;
	u8 min = 0;
	u8 sec = 0;

	data_buf[0] = cmd_code;

	if(len  == 6)												//数据长度必须是64
	{
		year = *(buf + 0);
		mon  = *(buf + 1);
		day  = *(buf + 2);
		hour = *(buf + 3);
		min  = *(buf + 4);
		sec  = *(buf + 5);

		if(year >= 18 && mon <= 12 && day <= 31 && hour <= 23 && min <= 59 && sec <= 59)
		{
			RTC_Set(year + 2000,mon,day,hour,min,sec);

			GetTimeOK = 1;
		}
		else
		{
			data_buf[1] = 1;
		}
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//设置策略时间
u16 SetRegularTimeGroups(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 group_num = 0;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u8 data_buf[2] = {0,0};
	u8 time_group[256];
	u16 crc16 = 0;

	data_buf[0] = cmd_code;

	if(len % 10 == 0)							//数据长度必须是7的倍数
	{
		group_num = len / 10;									//计算下发了几组数据

		if(group_num % 2 == 0 || group_num <= MAX_GROUP_NUM)	//组数必须是2的倍数，并且要小于MAX_GROUP_NUM
		{
			TimeGroupNumber = group_num;

			crc16 = CRC16(&group_num,1);

			AT24CXX_WriteOneByte(TIME_GROUP_NUM_ADD + 0,TimeGroupNumber);
			AT24CXX_WriteOneByte(TIME_GROUP_NUM_ADD + 1,(u8)(crc16 >> 8));
			AT24CXX_WriteOneByte(TIME_GROUP_NUM_ADD + 2,(u8)(crc16 & 0x00FF));

			memset(time_group,0,256);

			for(i = 0; i < group_num; i ++)
			{
				for(j = i * 12; j < i * 12 + 10; j ++, k ++)
				{
					time_group[j] = *(buf + k);
				}

				crc16 = CRC16(&time_group[j - 10],10);

				time_group[j + 0] = (u8)(crc16 >> 8);
				time_group[j + 1] = (u8)(crc16 & 0x00FF);
			}

			for(i = 0; i <= group_num / 2; i += 2)
			{
				RegularTimeStruct[i / 2].type 			= time_group[(i + 0) * 12 + 0];

				RegularTimeStruct[i / 2].s_year 		= time_group[(i + 0) * 12 + 1];
				RegularTimeStruct[i / 2].s_month 		= time_group[(i + 0) * 12 + 2];
				RegularTimeStruct[i / 2].s_date 		= time_group[(i + 0) * 12 + 3];
				RegularTimeStruct[i / 2].s_hour 		= time_group[(i + 0) * 12 + 4];
				RegularTimeStruct[i / 2].s_minute 		= time_group[(i + 0) * 12 + 5];

				RegularTimeStruct[i / 2].control_bit	= (((u16)time_group[(i + 0) * 12 + 6]) << 8) + (u16)time_group[(i + 0) * 12 + 7];
				RegularTimeStruct[i / 2].control_state	= (((u16)time_group[(i + 0) * 12 + 8]) << 8) + (u16)time_group[(i + 0) * 12 + 9];

				RegularTimeStruct[i / 2].e_year 		= time_group[(i + 1) * 12 + 1];
				RegularTimeStruct[i / 2].e_month 		= time_group[(i + 1) * 12 + 2];
				RegularTimeStruct[i / 2].e_date 		= time_group[(i + 1) * 12 + 3];
				RegularTimeStruct[i / 2].e_hour 		= time_group[(i + 1) * 12 + 4];
				RegularTimeStruct[i / 2].e_minute 		= time_group[(i + 1) * 12 + 5];

				RegularTimeStruct[i / 2].s_seconds  	= RegularTimeStruct[i / 2].s_hour * 3600 + RegularTimeStruct[i / 2].s_minute * 60;
				RegularTimeStruct[i / 2].e_seconds  	= RegularTimeStruct[i / 2].e_hour * 3600 + RegularTimeStruct[i / 2].e_minute * 60;
			}

			for(i = 0; i < group_num * 12 + group_num * 2; i ++)				//每组7个字节+2个字节(CRC16)
			{
				AT24CXX_WriteOneByte(TIME_RULE_ADD + i,time_group[i]);
			}
		}
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//控制设备的工作模式
u16 SetDeviceWorkMode(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 mode = 0;
	u8 data_buf[2] = {0,0};
	data_buf[0] = cmd_code;

	if(len == 1)
	{
		mode = *(buf + 0);

		if(mode == 0 || mode == 1)
		{
			DeviceWorkMode = mode;			//置工作模式标志
		}
		else
		{
			data_buf[1] = 1;
		}
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}

//设置设备的UUID
u16 SetDeviceUUID(u8 cmd_code,u8 cmd_id,u8 *buf,u8 len,u8 *outbuf)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u8 uuid_buf[38];

	data_buf[0] = cmd_code;

	if(len == UU_ID_LEN)												//数据长度必须是64
	{
		memset(uuid_buf,0,38);

		memcpy(&HoldReg[UU_ID_ADD],buf,36);

		GetDeviceUUID();

		WriteDataFromHoldBufToEeprom(&HoldReg[UU_ID_ADD],UU_ID_ADD, UU_ID_LEN - 2);
	}
	else
	{
		data_buf[1] = 2;
	}

	out_len = PackAckPacket(cmd_code,cmd_id,data_buf,outbuf);

	return out_len;
}



































#include "net_protocol.h"
#include "rtc.h"
#include "usart.h"
#include "24cxx.h"
#include "common.h"
#include "relay.h"

//网络数据帧协议解析
u16 NetDataAnalysis(u8 *buf,u16 len,u8 *outbuf)
{
	u8 i = 0;
	u16 ret = 0;
	u16 pos1 = 0;
	u16 got_uuid = 0xFFFF;
	u8 uuid_type = 0;		//0:HEX		1:ASCII
	u8 response = 0;

	u8 area_id = 0;
	u8 box_id = 0;
	u8 *uuid = NULL;
	u8 cmd_code = 0;
	u16 data_len = 0;
	u8 *data = NULL;
	u8 read_check_sum = 0;
	u8 cal_check_sum = 0;

	u8 buf_tail[6] = {0xFE,0xFD,0xFC,0xFB,0xFA,0xF9};

	pos1 = MyStrstr(buf,buf_tail,len,6);

	area_id = *(buf + 8);								//获取逻辑区码
	box_id = *(buf + 9);								//获取逻辑区码
	uuid = buf + 10;									//获取UUID
	cmd_code = *(buf + 27);								//获取功能码
	data_len = ((((u16)(*(buf + 28))) << 8) & 0xFF00) +
	           (((u16)(*(buf + 29))) & 0x00FF);			//获取数据长度
	data = buf + 30;									//获取数据域

	if(pos1 != 0xFFFF)
	{
		if(*(buf + 0) == 0x68 && \
			*(buf + 7) == 0x68 && \
			*(buf + pos1 - 1) == 0x16)							//判断包头和包尾
		{
			read_check_sum = *(buf + pos1 - 2);					//获取校验和
			cal_check_sum = CalCheckSum(buf, pos1 - 2);			//计算校验和

			if(read_check_sum == cal_check_sum)
			{
				if(area_id == 0xFF && box_id == 0xFF)
				{
					if(DeviceUUID != NULL)
					{
						got_uuid = MyStrstr(uuid,DeviceUUID,len - 10,UU_ID_LEN - 2);

						if(got_uuid == 0xFFFF)
						{
							for(i = 0; i < 17; i ++)
							{
								*(uuid + i) = *(uuid + i) + 0x30;
							}

							got_uuid = MyStrstr(uuid,DeviceUUID,len - 10,UU_ID_LEN - 2);
						}
						else
						{
							uuid_type = 1;
						}

						if(got_uuid == 0xFFFF)
						{
							return 0;
						}
						else
						{
							response = 1;
						}
					}
				}
				else if((area_id == DeviceAreaID && box_id == DeviceBoxID) ||
						(area_id == DeviceAreaID && box_id == 0xFE) ||
						(area_id == 0xFE && box_id == 0xFE))
				{
					response = 2;
				}
				else
				{
					return 0;
				}

				if(response != 0)
				{
					switch(cmd_code)
					{
						case 0xD0:									//读取各个继电器状态
							ret = UpdateRelayModeInfo(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD1:									//控制继电器开闭状态
							ret = ControlRelayState(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD2:									//重启设备
							ret = ControlDeviceReset(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD3:									//设置定时发送间隔,下行
							ret = SetDeviceUpLoadINCL(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD4:									//读取设备基本信息
							ret = ReadDeviceInfo(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD5:									//从服务器获取时间
							ret = GetTimeDateFromServer(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD6:									//设置继电器定时策略，下行
							ret = SetRegularTimeGroups(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD7:									//设置设备工作模式
							ret = SetDeviceWorkMode(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD8:									//设置定时发送间隔,下行
							ret = SetRelayActionINCL(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xD9:									//设置通讯波特率
							ret = SetRS485BuarRate(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xDA:									//设置逻辑区和物理区,下行
							ret = SetAreaID_BoxID(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xDB:									//设置升级包信息
							ret = SetUpdateFirmWareInfo(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xDC:									//写升级包
							ret = WriteFrameWareBags(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xDD:									//设置位置信息
							ret = SetPosition(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xDE:									//设置平日策略组
							ret = SetNormalStrategyGroup(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xDF:									//设置节日策略组
							ret = SetAppointmentGroups(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xF0:									//设置六路采集模块对照表
							ret = SetContrastTable(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0xF1:									//读取六路采集模块对照表
							ret = GetContrastTable(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;
						
						case 0xF2:									//读取继电器开闭策略
							ret = GetRegularTimeGroups(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;
						
						case 0xF3:									//读取平日策略组配置
							ret = GetNormalStrategyGroup(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;
						
						case 0xF4:									//读取节日策略组配置
							ret = GetAppointmentGroups(cmd_code,data,data_len,outbuf,response,uuid_type);
						break;

						case 0x80:									//应答，下行,上行在别处处理
							UnPackAckPacket(cmd_code,data,data_len);
						break;

						default:									//此处要给云端应答一个功能码错误信息

						break;
					}
				}
			}
		}
		else	//此处可以给云端应答一个帧头错误信息
		{

		}
	}
	else		//此处可以给云端应答一个校验错误信息
	{

	}

	return ret;
}

//解析ACK包
u8 UnPackAckPacket(u8 cmd_code,u8 *buf,u16 len)
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
u16 PackAckPacket(u8 cmd_code,u8 *data,u8 *outbuf,u8 id_type)
{
	u16 len = 0;

	len = PackNetData(0x80,data,2,outbuf,id_type);

	return len;
}

u16 UpdateRelayModeInfo(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;

	u8 data_buf[10] = {0};

	if(len == 0)
	{
		if(resp == 1)
		{
			out_len = PackDataOfRelayInfo(data_buf);
			out_len = PackNetData(cmd_code,data_buf,out_len,outbuf,id_type);
		}
	}

	return out_len;
}

//控制继电器开闭
u16 ControlRelayState(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 i = 0;
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u16 bit = 0;
	u16 state = 0;

	u32 relay_state = 0;

	data_buf[0] = cmd_code;

	if(len == 4)												//数据长度必须是64
	{
		bit = ((u16)(*(buf + 0)) << 8) + (*(buf + 1));
		state = ((u16)(*(buf + 2)) << 8) + (*(buf + 3));

		if(bit <= 0x0FFF && state <= 0x0FFF)
		{
//			OutPutControlBit = bit;
//			OutPutControlState = state;

			relay_state = bit;
			relay_state = relay_state << 16;
			relay_state = relay_state + state;

			if(xQueueSend(xQueue_RelayState,(void *)&relay_state,(TickType_t)10) != pdPASS)
			{
#ifdef DEBUG_LOG
				printf("send p_tSensorMsg fail 2.\r\n");
#endif
			}

			for(i = 0; i < CH_NUM; i ++)
			{
				if(bit & (1 << i))
				{
					if(state & (1 << i))
					{
						RelaysState |= (1 << i);
					}
					else
					{
						RelaysState &= ~(1 << i);
					}
				}
			}

			HaveNewActionCommand = 1;
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//下发OTA命令
u16 SetUpdateFirmWareInfo(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u16 i = 0;
	u8 out_len = 0;
	u8 data_buf[5] = {0,0,0,0,0};

	if(len == 6)
	{
		if(*(buf + 0) <= 100)
		{
			FrameWareInfo.version = (((u16)(*(buf + 0))) << 8) + (u16)(*(buf + 1));

			FrameWareInfo.length = (((u32)(*(buf + 2))) << 24) +
								   (((u32)(*(buf + 3))) << 16) +
								   (((u32)(*(buf + 4))) << 8) +
								   (((u32)(*(buf + 5))) << 0);

			WriteDataFromMemoryToEeprom(buf,SOFT_WARE_INFO_ADD,SOFT_WARE_INFO_LEN - 2);

			if(FrameWareInfo.length > FIRMWARE_SIZE)
			{
				data_buf[0] = 0;
			}
			else
			{
				u16 page_num = 0;

				FrameWareState.state 			= FIRMWARE_DOWNLOADING;
				FrameWareState.total_bags 		= FrameWareInfo.length % FIRMWARE_BAG_SIZE != 0 ?
												  FrameWareInfo.length / FIRMWARE_BAG_SIZE + 1 : FrameWareInfo.length / FIRMWARE_BAG_SIZE;
				FrameWareState.current_bag_cnt 	= 1;
				FrameWareState.bag_size 		= FIRMWARE_BAG_SIZE;
				FrameWareState.last_bag_size 	= FrameWareInfo.length % FIRMWARE_BAG_SIZE != 0 ?
												  FrameWareInfo.length % FIRMWARE_BAG_SIZE : FIRMWARE_BAG_SIZE;
				FrameWareState.total_size 		= FrameWareInfo.length;

				WriteFrameWareStateToEeprom();	//将固件升级状态写入EEPROM

				page_num = (FIRMWARE_MAX_FLASH_ADD - FIRMWARE_BUCKUP_FLASH_BASE_ADD) / 2048;	//得到备份区的扇区总数

				FLASH_Unlock();						//解锁FLASH

				for(i = 0; i < page_num; i ++)
				{
					FLASH_ErasePage(i * 2048 + FIRMWARE_BUCKUP_FLASH_BASE_ADD);	//擦除当前FLASH扇区
				}

				FLASH_Lock();						//上锁

				data_buf[0] = 1;
				data_buf[1] = (u8)(FrameWareState.total_bags >> 8);
				data_buf[2] = (u8)(FrameWareState.total_bags & 0x00FF);
				data_buf[3] = (u8)(FrameWareState.current_bag_cnt >> 8);
				data_buf[4] = (u8)(FrameWareState.current_bag_cnt & 0x00FF);
			}
		}
	}
	else
	{
		data_buf[0] = 0;
	}

	if(resp == 1)
	{
		out_len = PackNetData(cmd_code,data_buf,5,outbuf,id_type);
	}

	return out_len;
}

//下发更新固件命令
u16 WriteFrameWareBags(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[5] = {0,0,0,0,0};

	u16 i = 0;
	u16 total_bags = 0;
	u16 current_bags = 0;
	u16 bag_size = 0;
	u8 *msg = NULL;
	u16 crc_read = 0;
	u16 crc_cal = 0;
	u32 crc32_cal = 0xFFFFFFFF;
	u32 crc32_read = 0;
	u8 crc32_cal_buf[1024];
	u32 file_len = 0;
	u16 k_num = 0;
	u16 last_k_byte_num = 0;
	u16 temp = 0;

	data_buf[0] = 1;

	if(len == 6 + FIRMWARE_BAG_SIZE)
	{
		total_bags = (((u16)(*(buf + 0))) << 8) + (u16)(*(buf + 1));
		current_bags = (((u16)(*(buf + 2))) << 8) + (u16)(*(buf + 3));
		bag_size = (((u16)(*(buf + 4))) << 8) + (u16)(*(buf + 5));

		if(total_bags != FrameWareState.total_bags ||
		   current_bags > FrameWareState.total_bags)		//总包数匹配错误
		{
			return 0;
		}

		msg = buf + 6;

		crc_read = (((u16)(*(msg + bag_size - 2))) << 8) + (u16)(*(msg + bag_size - 1));

		crc_cal = GetCRC16(msg,bag_size - 2);

		if(crc_cal == crc_read)
		{
			if(current_bags == FrameWareState.current_bag_cnt)
			{
				if(current_bags < FrameWareState.total_bags)
				{
					FLASH_Unlock();						//解锁FLASH

					if(bag_size == FIRMWARE_BAG_SIZE)
					{
						for(i = 0; i < (FIRMWARE_BAG_SIZE - 2) / 2; i ++)
						{
							temp = ((u16)(*(msg + i * 2 + 1)) << 8) + (u16)(*(msg + i * 2));

							FLASH_ProgramHalfWord(FIRMWARE_BUCKUP_FLASH_BASE_ADD + (current_bags - 1) * (FIRMWARE_BAG_SIZE - 2) + i * 2,temp);
						}
					}

					FLASH_Lock();						//上锁

					FrameWareState.current_bag_cnt ++;

					FrameWareState.state = FIRMWARE_DOWNLOADING;	//当前包下载完成
				}
				else if(current_bags == FrameWareState.total_bags)
				{
					crc32_read = (((u32)(*(msg + 0))) << 24) +
					             (((u32)(*(msg + 1))) << 16) +
					             (((u32)(*(msg + 2))) << 8) +
					             (((u32)(*(msg + 3))));

					file_len = 128 * (FrameWareState.total_bags - 1);

					k_num = file_len / 1024;
					last_k_byte_num = file_len % 1024;
					if(last_k_byte_num > 0)
					{
						k_num += 1;
					}

					for(i = 0; i < k_num; i ++)
					{
						memset(crc32_cal_buf,0,1024);
						if(i < k_num - 1)
						{
							STMFLASH_ReadBytes(FIRMWARE_BUCKUP_FLASH_BASE_ADD + 1024 * i,crc32_cal_buf,1024);
							crc32_cal = CRC32(crc32_cal_buf,1024,crc32_cal,0);
						}
						if(i == k_num - 1)
						{
							if(last_k_byte_num == 0)
							{
								STMFLASH_ReadBytes(FIRMWARE_BUCKUP_FLASH_BASE_ADD + 1024 * i,crc32_cal_buf,1024);
								crc32_cal = CRC32(crc32_cal_buf,1024,crc32_cal,1);
							}
							else if(last_k_byte_num > 0)
							{
								STMFLASH_ReadBytes(FIRMWARE_BUCKUP_FLASH_BASE_ADD + 1024 * i,crc32_cal_buf,last_k_byte_num);
								crc32_cal = CRC32(crc32_cal_buf,last_k_byte_num,crc32_cal,1);
							}
						}
					}

					if(crc32_read == crc32_cal)
					{
						FrameWareState.state = FIRMWARE_DOWNLOADED;				//全部下载完成

						data_buf[0] = 2;
					}
					else
					{
						FrameWareState.state = FIRMWARE_DOWNLOAD_FAILED;		//全部下载完成
						data_buf[0] = 3;
					}

					WriteFrameWareStateToEeprom();
				}
			}
		}
	}

	data_buf[1] = (u8)(FrameWareState.total_bags >> 8);
	data_buf[2] = (u8)(FrameWareState.total_bags & 0x00FF);
	data_buf[3] = (u8)(FrameWareState.current_bag_cnt >> 8);
	data_buf[4] = (u8)(FrameWareState.current_bag_cnt & 0x00FF);

	if(resp == 1)
	{
		out_len = PackNetData(cmd_code,data_buf,5,outbuf,id_type);
	}

	return out_len;
}

//远程重启
u16 ControlDeviceReset(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置设备数据上传时间间隔
u16 SetDeviceUpLoadINCL(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u16 incl;

	data_buf[0] = cmd_code;

	if(len == 2)												//数据长度必须是64
	{
		incl = ((u16)(*(buf + 0)) << 8) + (*(buf + 1));

		if(incl <= MAX_UPLOAD_INVL)
		{
			UpLoadINCL = incl;

			WriteDataFromMemoryToEeprom(buf,UPLOAD_INVL_ADD,UPLOAD_INVL_LEN - 2);
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//读取继电器信息
u16 ReadDeviceInfo(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 i = 0;

	u8 temp_buf[POSITION_LEN];
	u8 data_buf[29] = {0};

	if(len == 0)
	{
		if(resp == 1)
		{
			data_buf[0] = (u8)(((u16)SOFT_WARE_VRESION) >> 8);
			data_buf[1] = (u8)SOFT_WARE_VRESION;

			data_buf[2] = DeviceWorkMode;

			data_buf[3] = (u8)(RelayActionINCL >> 8);
			data_buf[4] = (u8)RelayActionINCL;

			data_buf[5] = DeviceAreaID;
			data_buf[6] = DeviceBoxID;

			memcpy(temp_buf,&Location.longitude,8);

			for(i = 0; i < 8; i ++)
			{
				data_buf[7 + i] = temp_buf[7 - i];
			}

			memcpy(temp_buf,&Location.latitude,8);

			for(i = 0; i < 8; i ++)
			{
				data_buf[15 + i] = temp_buf[7 - i];
			}

			data_buf[23] = ((((u8)(calendar.w_year - 2000)) / 10) << 4) | (((u8)(calendar.w_year - 2000)) % 10);
			data_buf[24] = ((calendar.w_month / 10) << 4) | (calendar.w_month % 10);
			data_buf[25] = ((calendar.w_date / 10) << 4) | (calendar.w_date % 10);
			data_buf[26] = ((calendar.hour / 10) << 4) | (calendar.hour % 10);
			data_buf[27] = ((calendar.min / 10) << 4) | (calendar.min % 10);
			data_buf[28] = ((calendar.sec / 10) << 4) | (calendar.sec % 10);

			out_len = PackNetData(cmd_code,data_buf,29,outbuf,id_type);
		}
	}

	return out_len;
}

//从服务器获取时间戳
u16 GetTimeDateFromServer(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
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

			RefreshStrategy = 1;
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置策略时间
u16 SetRegularTimeGroups(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 group_num = 0;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u8 data_buf[2] = {0,0};
	u8 time_group[MAX_STRATEGY_NUM * STRATEGY_CONTENT_LEN];
	u16 crc16 = 0;
	s16 minutes = 0;

	data_buf[0] = cmd_code;

	if(len % (STRATEGY_CONTENT_LEN - 2) == 0)			//数据长度必须是10的倍数
	{
		group_num = len / (STRATEGY_CONTENT_LEN - 2);	//计算下发了几组数据

		if(group_num <= MAX_STRATEGY_NUM)				//组数要小于MAX_STRATEGY_NUM
		{
			TimeStrategyNumber = group_num;

			crc16 = CRC16(&group_num,1);

			AT24CXX_WriteOneByte(STRATEGY_GROUP_NUM_ADD + 0,TimeStrategyNumber);
			AT24CXX_WriteOneByte(STRATEGY_GROUP_NUM_ADD + 1,(u8)(crc16 >> 8));
			AT24CXX_WriteOneByte(STRATEGY_GROUP_NUM_ADD + 2,(u8)(crc16 & 0x00FF));

			RemoveAllStrategy();						//删除所有本地存储策略

			memset(time_group,0,MAX_STRATEGY_NUM * STRATEGY_CONTENT_LEN);

			k = 0;

			for(i = 0; i < group_num; i ++)
			{
				for(j = i * STRATEGY_CONTENT_LEN; j < i * STRATEGY_CONTENT_LEN + (STRATEGY_CONTENT_LEN - 2); j ++, k ++)
				{
					time_group[j] = *(buf + k);
				}

				crc16 = CRC16(&time_group[j - (STRATEGY_CONTENT_LEN - 2)],(STRATEGY_CONTENT_LEN - 2));

				time_group[j + 0] = (u8)(crc16 >> 8);
				time_group[j + 1] = (u8)(crc16 & 0x00FF);
			}

			for(i = 0; i < group_num; i ++)
			{
				pStrategyTime tmp_time = NULL;

				tmp_time = (pStrategyTime)mymalloc(sizeof(StrategyTime_S));

				tmp_time->prev = NULL;
				tmp_time->next = NULL;

				tmp_time->number 		= i;
				tmp_time->group 		= time_group[i * STRATEGY_CONTENT_LEN + 0];
				tmp_time->type 			= time_group[i * STRATEGY_CONTENT_LEN + 1];
				tmp_time->hour 			= time_group[i * STRATEGY_CONTENT_LEN + 2];
				tmp_time->minute 		= time_group[i * STRATEGY_CONTENT_LEN + 3];
				tmp_time->offset_min 	= (s16)((((u16)time_group[i * STRATEGY_CONTENT_LEN + 4]) << 8) + (u16)time_group[i * STRATEGY_CONTENT_LEN + 5]);
				tmp_time->control_bit	= (((u16)time_group[i * STRATEGY_CONTENT_LEN + 6]) << 8) + (u16)time_group[i * STRATEGY_CONTENT_LEN + 7];
				tmp_time->control_state	= (((u16)time_group[i * STRATEGY_CONTENT_LEN + 8]) << 8) + (u16)time_group[i * STRATEGY_CONTENT_LEN + 9];

				if(tmp_time->type == 2 || tmp_time->type == 3)
				{
					if(tmp_time->type == 2)
					{
						tmp_time->hour = SunRiseSetTime.rise_h;
						tmp_time->minute = SunRiseSetTime.rise_m;
					}
					else if(tmp_time->type == 3)
					{
						tmp_time->hour = SunRiseSetTime.set_h;
						tmp_time->minute = SunRiseSetTime.set_m;
					}

					minutes = tmp_time->hour * 60 + tmp_time->minute + tmp_time->offset_min;

					tmp_time->hour = minutes / 60;
					tmp_time->minute = minutes % 60;
				}

				RegularTimeGroupAdd(tmp_time);
			}

			for(i = 0; i < group_num * STRATEGY_CONTENT_LEN + group_num; i ++)
			{
				AT24CXX_WriteOneByte(STRATEGY_CONTENT_ADD + i,time_group[i]);
			}
		}

		RefreshStrategy = 1;	//需要刷新策略状态
	}
	else
	{
		data_buf[1] = 2;
	}

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//读取策略
u16 GetRegularTimeGroups(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u16 out_len = 0;
	u16 i = 0;
	u16 j = 0;

	u8 data_buf[MAX_STRATEGY_NUM * STRATEGY_CONTENT_LEN];

	if(len == 0)
	{
		if(resp == 1)
		{
			for(i = 0; i < TimeStrategyNumber; i ++)
			{
				for(j = i * STRATEGY_CONTENT_LEN; j < i * STRATEGY_CONTENT_LEN + STRATEGY_CONTENT_LEN; j ++)
				{
					data_buf[j] = AT24CXX_ReadOneByte(STRATEGY_CONTENT_ADD + j);
				}
				
				memcpy(&data_buf[i * (STRATEGY_CONTENT_LEN - 2)],
				       &data_buf[i * STRATEGY_CONTENT_LEN],
				       STRATEGY_CONTENT_LEN - 2);
			}
			
			out_len = TimeStrategyNumber * (STRATEGY_CONTENT_LEN - 2);

			out_len = PackNetData(cmd_code,data_buf,out_len,outbuf,id_type);
		}
	}

	return out_len;
}

//设置平日策略组
u16 SetNormalStrategyGroup(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};

	data_buf[0] = cmd_code;

	if(len == 6)									//数据长度必须是10的倍数
	{
		NormalControl.week_enable = *(buf + 0);
		NormalControl.cycle_min = (((u16)(*(buf + 1))) << 8) + (u16)(*(buf + 2));
		NormalControl.strategy_group = *(buf + 5);

		WriteDataFromMemoryToEeprom(buf,NORMAL_STRATEGY_GROUP_ADD,NORMAL_STRATEGY_GROUP_LEN - 2);

		RefreshStrategy = 1;	//需要刷新策略状态
	}
	else
	{
		data_buf[1] = 2;
	}

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//读取平日策略组配置
u16 GetNormalStrategyGroup(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u16 out_len = 0;
	u8 data_buf[6] = {0};

	if(len == 0)
	{
		if(resp == 1)
		{
			data_buf[0] = NormalControl.week_enable;
			
			data_buf[1] = (u8)(NormalControl.cycle_min >> 8);
			data_buf[2] = (u8)NormalControl.cycle_min;
			
			data_buf[3] = 0;
			data_buf[4] = 0;
			
			data_buf[5] = NormalControl.strategy_group;

			out_len = PackNetData(cmd_code,data_buf,6,outbuf,id_type);
		}
	}

	return out_len;
}

//设置节日策略组
u16 SetAppointmentGroups(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 group_num = 0;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u8 data_buf[2] = {0,0};
	u8 time_group[MAX_GROUP_NUM * APPOIN_STRATEGY_GROUP_LEN];
	u16 crc16 = 0;

	data_buf[0] = cmd_code;

	if(len % (APPOIN_STRATEGY_GROUP_LEN - 2) == 0)				//数据长度必须是10的倍数
	{
		group_num = len / (APPOIN_STRATEGY_GROUP_LEN - 2);		//计算下发了几组数据

		if(group_num <= MAX_GROUP_NUM)							//组数要小于MAX_GROUP_NUM
		{
			RemoveAllAppointmentStrategy();						//删除所有本地存储策略

			AppoinGroupNumber = group_num;

			crc16 = CRC16(&group_num,1);

			AT24CXX_WriteOneByte(APPOIN_GROUP_NUM_ADD + 0,AppoinGroupNumber);
			AT24CXX_WriteOneByte(APPOIN_GROUP_NUM_ADD + 1,(u8)(crc16 >> 8));
			AT24CXX_WriteOneByte(APPOIN_GROUP_NUM_ADD + 2,(u8)(crc16 & 0x00FF));

			memset(time_group,0,MAX_GROUP_NUM * APPOIN_STRATEGY_GROUP_LEN);

			k = 0;

			for(i = 0; i < group_num; i ++)
			{
				for(j = i * APPOIN_STRATEGY_GROUP_LEN; j < i * APPOIN_STRATEGY_GROUP_LEN + (APPOIN_STRATEGY_GROUP_LEN - 2); j ++, k ++)
				{
					time_group[j] = *(buf + k);
				}

				crc16 = CRC16(&time_group[j - (APPOIN_STRATEGY_GROUP_LEN - 2)],(APPOIN_STRATEGY_GROUP_LEN - 2));

				time_group[j + 0] = (u8)(crc16 >> 8);
				time_group[j + 1] = (u8)(crc16 & 0x00FF);
			}

			for(i = 0; i < group_num; i ++)
			{
				pAppointmentControl tmp_time = NULL;

				tmp_time = (pAppointmentControl)mymalloc(sizeof(AppointmentControl_S));

				tmp_time->prev = NULL;
				tmp_time->next = NULL;

				tmp_time->number 		 = i;
				tmp_time->s_year 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 0];
				tmp_time->s_month 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 1];
				tmp_time->s_date 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 2];
				tmp_time->s_hour 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 3];
				tmp_time->s_minute 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 4];

				tmp_time->e_year 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 5];
				tmp_time->e_month 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 6];
				tmp_time->e_date 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 7];
				tmp_time->e_hour 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 8];
				tmp_time->e_minute 		 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 9];

				tmp_time->week_enable 	 = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 10];

				tmp_time->cycle_min 	 = (((u16)time_group[i * APPOIN_STRATEGY_GROUP_LEN + 11]) << 8) + (u16)time_group[i * APPOIN_STRATEGY_GROUP_LEN + 12];

				tmp_time->strategy_group = time_group[i * APPOIN_STRATEGY_GROUP_LEN + 15];

				AppointmentGroupAdd(tmp_time);
			}

			for(i = 0; i < group_num * APPOIN_STRATEGY_GROUP_LEN + group_num; i ++)				//每组7个字节+2个字节(CRC16)
			{
				AT24CXX_WriteOneByte(APPOIN_STRATEGY_GROUP_ADD + i,time_group[i]);
			}
		}

		RefreshStrategy = 1;	//需要刷新策略状态
	}
	else
	{
		data_buf[1] = 2;
	}

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//读取节日策略配置
u16 GetAppointmentGroups(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u16 out_len = 0;
	u16 i = 0;
	u16 j = 0;

	u8 data_buf[MAX_GROUP_NUM * APPOIN_STRATEGY_GROUP_LEN];

	if(len == 0)
	{
		if(resp == 1)
		{
			for(i = 0; i < AppoinGroupNumber; i ++)
			{
				for(j = i * APPOIN_STRATEGY_GROUP_LEN; j < i * APPOIN_STRATEGY_GROUP_LEN + APPOIN_STRATEGY_GROUP_LEN; j ++)
				{
					data_buf[j] = AT24CXX_ReadOneByte(APPOIN_STRATEGY_GROUP_ADD + j);
				}
				
				memcpy(&data_buf[i * (APPOIN_STRATEGY_GROUP_LEN - 2)],
				       &data_buf[i * APPOIN_STRATEGY_GROUP_LEN],
				       APPOIN_STRATEGY_GROUP_LEN - 2);
			}
			
			out_len = AppoinGroupNumber * (APPOIN_STRATEGY_GROUP_LEN - 2);

			out_len = PackNetData(cmd_code,data_buf,out_len,outbuf,id_type);
		}
	}

	return out_len;
}


//控制设备的工作模式
u16 SetDeviceWorkMode(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置设备的UUID
u16 SetDeviceUUID(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};

	data_buf[0] = cmd_code;

	if(len == UU_ID_LEN - 2)												//数据长度必须是64
	{
		GetMemoryForSpecifyPointer(&DeviceUUID,UU_ID_LEN - 2, buf);

		WriteDataFromMemoryToEeprom(buf,UU_ID_ADD, UU_ID_LEN - 2);
	}
	else
	{
		data_buf[1] = 2;
	}

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置继电器动作间隔
u16 SetRelayActionINCL(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u16 incl;

	data_buf[0] = cmd_code;

	if(len == 2)												//数据长度必须是64
	{
		incl = ((u16)(*(buf + 0)) << 8) + (*(buf + 1));

		if(incl <= MAX_REALY_ACTION_INVL)
		{
			RelayActionINCL = incl;

			WriteDataFromMemoryToEeprom(buf,REALY_ACTION_INVL_ADD, REALY_ACTION_INVL_LEN - 2);
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置继电器动作间隔
u16 SetRS485BuarRate(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u32 baud_rate = 9600;

	data_buf[0] = cmd_code;

	if(len == 4)												//数据长度必须是64
	{
		baud_rate = ((u32)(*(buf + 0)) << 24) + ((u32)(*(buf + 1)) << 16) + ((u32)(*(buf + 2)) << 8) + (u32)(*(buf + 3));

		if(baud_rate <= 19200)
		{
			RS485BuadRate = baud_rate;

			WriteDataFromMemoryToEeprom(buf,RS485_BUAD_RATE_ADD, RS485_BUAD_RATE_LEN - 2);

			USART2_Init(RS485BuadRate);
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置逻辑区和物理区
u16 SetAreaID_BoxID(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};
	u8 temp_buf[AREA_ID_LEN];
	data_buf[0] = cmd_code;

	if(len == 2)
	{
		if(*(buf + 0) < 0xFE && *(buf + 1) < 0xFE)
		{
			DeviceAreaID = *(buf + 0);
			temp_buf[0] = *(buf + 0);
			WriteDataFromMemoryToEeprom(temp_buf,AREA_ID_ADD, AREA_ID_LEN - 2);

			DeviceBoxID = *(buf + 1);
			temp_buf[0] = *(buf + 1);
			WriteDataFromMemoryToEeprom(temp_buf,BOX_ID_ADD, BOX_ID_LEN - 2);
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

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置位置信息
u16 SetPosition(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 i = 0;
	u8 data_buf[2] = {0,0};
	u8 temp_buf[POSITION_LEN];

	data_buf[0] = cmd_code;

	if(len == POSITION_LEN - 2)
	{
		memset(temp_buf,0,POSITION_LEN);

		for(i = 0; i < 8; i ++)
		{
			temp_buf[i] = buf[7 - i];
		}

		memcpy(&Location.longitude,temp_buf,8);

		for(i = 0; i < 8; i ++)
		{
			temp_buf[i] = buf[15 - i];
		}

		memcpy(&Location.latitude,temp_buf,8);

		WriteDataFromMemoryToEeprom(buf,POSITION_ADD, POSITION_LEN - 2);
	}
	else
	{
		data_buf[1] = 2;
	}

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//设置计量模块回路对照表
u16 SetContrastTable(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;
	u8 data_buf[2] = {0,0};

	data_buf[0] = cmd_code;

	if(len == CONTRAST_TABLE_LEN - 2)
	{
		memcpy(ContrastTable,buf,CONTRAST_TABLE_LEN - 2);

		WriteDataFromMemoryToEeprom(buf,CONTRAST_TABLE_ADD, CONTRAST_TABLE_LEN - 2);
	}
	else
	{
		data_buf[1] = 2;
	}

	if(resp == 1)
	{
		out_len = PackAckPacket(cmd_code,data_buf,outbuf,id_type);
	}

	return out_len;
}

//读取计量模块回路对照表
u16 GetContrastTable(u8 cmd_code,u8 *buf,u16 len,u8 *outbuf,u8 resp,u8 id_type)
{
	u8 out_len = 0;

	u8 data_buf[CONTRAST_TABLE_LEN];

	if(len == 0)
	{
		if(resp == 1)
		{
			memcpy(data_buf,ContrastTable,CONTRAST_TABLE_LEN - 2);

			out_len = PackNetData(cmd_code,data_buf,CONTRAST_TABLE_LEN - 2,outbuf,id_type);
		}
	}

	return out_len;
}

































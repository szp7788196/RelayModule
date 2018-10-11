#include "common.h"
#include "24cxx.h"


u8 HoldReg[HOLD_REG_LEN];						//保持寄存器
u8 RegularTimeGroups[TIME_BUF_LEN];				//时间策略缓存
u8 TimeGroupNumber = 0;							//时间策略组数
RegularTime_S RegularTimeStruct[MAX_GROUP_NUM];	//时间策略结构体数组


/****************************互斥量相关******************************/
SemaphoreHandle_t  xMutex_IIC1 			= NULL;	//IIC总线1的互斥量

/***************************固件升级相关*****************************/
u8 NeedUpDateFirmWare = 0;			//有新固件需要加载
u8 HaveNewFirmWare = 0;				//0xAA有新固件 others无新固件
u8 NewFirmWareAdd = 0;				//0xAA新固件地址0x0800C000 0x55新固件地址0x08026000
u16 NewFirmWareBagNum = 0;			//固件包的数量（一个固件包含多个小包）
u16 NewFirmWareVer = 1;				//固件包的版本
u8 LastBagByteNum = 0;				//最后一包的字节数

/***************************系统心跳相关*****************************/
u32 SysTick1ms = 0;					//1ms滴答时钟
u32 SysTick10ms = 0;				//10ms滴答时钟
u32 SysTick100ms = 0;				//10ms滴答时钟
time_t SysTick1s = 0;				//1s滴答时钟

/***************************版本相关*********************************/
u8 *SoftWareVersion = NULL;			//应用程序版本号
u8 *HardWareVersion = NULL;			//硬件版本号

/***************************设备相关*********************************/
u8 *DeviceName = NULL;				//设备名称
u8 *DeviceID = NULL;				//设备ID
u8 *DeviceUUID = NULL;				//设备UUID

/***************************运行参数相关*****************************/
u16 UpLoadINCL = 10;				//数据上传时间间隔0~65535秒
u8 DeviceWorkMode = 0;				//运行模式，0：自动，1：手动

/***************************其他*****************************/
u8 NeedToReset = 0;					//复位/重启标志
u16 OutPutControlBit = 0;			//开出位标志
u16 OutPutControlState = 0;			//开出位标志(具体哪几位)
u16 AllRelayPowerState = 0;			//继电器输入端是否带电
u16 AllRelayState = 0;				//继电器的状态


//在str1中查找str2，失败返回0xFF,成功返回str2首个元素在str1中的位置
u16 MyStrstr(u8 *str1, u8 *str2, u16 str1_len, u16 str2_len)
{
	u16 len = str1_len;
	if(str1_len == 0 || str2_len == 0)
	{
		return 0xFFFF;
	}
	else
	{
		while(str1_len >= str2_len)
		{
			str1_len --;
			if (!memcmp(str1, str2, str2_len))
			{
				return len - str1_len - 1;
			}
			str1 ++;
		}
		return 0xFFFF;
	}
}

//获得整数的位数
u8 GetDatBit(u32 dat)
{
	u8 j = 1;
	u32 i;
	i = dat;
	while(i >= 10)
	{
		j ++;
		i /= 10;
	}
	return j;
}

//用个位数换算出一个整数 1 10 100 1000......
u32 GetADV(u8 len)
{
	u32 count = 1;
	if(len == 1)
	{
		return 1;
	}
	else
	{
		len --;
		while(len --)
		{
			count *= 10;
		}
	}
	return count;
}

//整数转换为字符串
void IntToString(u8 *DString,u32 Dint,u8 zero_num)
{
	u16 i = 0;
	u8 j = GetDatBit(Dint);
	for(i = 0; i < GetDatBit(Dint) + zero_num; i ++)
	{
		DString[i] = Dint / GetADV(j) % 10 + 0x30;
		j --;
	}
}

u32 StringToInt(u8 *String)
{
	u8 len;
	u8 i;
	u32 count=0;
	u32 dev;

	len = strlen((char *)String);
	dev = 1;
	for(i = 0; i < len; i ++)//len-1
	{
		if(String[i] != '.')
		{
			count += ((String[i] - 0x30) * GetADV(len) / dev);
			dev *= 10;
		}
		else
		{
			len --;
			count /= 10;
		}
	}
	if(String[i]!=0x00)
	{
		count += (String[i] - 0x30);
	}
	return count;
}

unsigned short find_str(unsigned char *s_str, unsigned char *p_str, unsigned short count, unsigned short *seek)
{
	unsigned short _count = 1;
    unsigned short len = 0;
    unsigned char *temp_str = NULL;
    unsigned char *temp_ptr = NULL;
    unsigned char *temp_char = NULL;

	(*seek) = 0;
    if(0 == s_str || 0 == p_str)
        return 0;
    for(temp_str = s_str; *temp_str != '\0'; temp_str++)
    {
        temp_char = temp_str;

        for(temp_ptr = p_str; *temp_ptr != '\0'; temp_ptr++)
        {
            if(*temp_ptr != *temp_char)
            {
                len = 0;
                break;
            }
            temp_char++;
            len++;
        }
        if(*temp_ptr == '\0')
        {
            if(_count == count)
                return len;
            else
            {
                _count++;
                len = 0;
            }
        }
        (*seek) ++;
    }
    return 0;
}

int search_str(unsigned char *source, unsigned char *target)
{
	unsigned short seek = 0;
    unsigned short len;
    len = find_str(source, target, 1, &seek);
    if(len == 0)
        return -1;
    else
        return len;
}

unsigned short get_str1(unsigned char *source, unsigned char *begin, unsigned short count1, unsigned char *end, unsigned short count2, unsigned char *out)
{
	unsigned short i;
    unsigned short len1;
    unsigned short len2;
    unsigned short index1 = 0;
    unsigned short index2 = 0;
    unsigned short length = 0;
    len1 = find_str(source, begin, count1, &index1);
    len2 = find_str(source, end, count2, &index2);
    length = index2 - index1 - len1;
    if((len1 != 0) && (len2 != 0))
    {
        for( i = 0; i < index2 - index1 - len1; i++)
            out[i] = source[index1 + len1 + i];
        out[i] = '\0';
    }
    else
    {
        out[0] = '\0';
    }
    return length;
}

unsigned short get_str2(unsigned char *source, unsigned char *begin, unsigned short count, unsigned short length, unsigned char *out)
{
	unsigned short i = 0;
    unsigned short len1 = 0;
    unsigned short index1 = 0;
    len1 = find_str(source, begin, count, &index1);
    if(len1 != 0)
    {
        for(i = 0; i < length; i++)
            out[i] = source[index1 + len1 + i];
        out[i] = '\0';
    }
    else
    {
        out[0] = '\0';
    }
    return length;
}

unsigned short get_str3(unsigned char *source, unsigned char *out, unsigned short length)
{
	unsigned short i = 0;
    for (i = 0 ; i < length ; i++)
    {
        out[i] = source[i];
    }
    out[i] = '\0';
    return length;
}

//32位CRC校验
u32 CRC32( const u8 *buf, u32 size)
{
     uint32_t i, crc;
     crc = 0xFFFFFFFF;
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
     return crc^0xFFFFFFFF;
}

/*****************************************************
函数：u16 CRC16(u8 *puchMsgg,u8 usDataLen)
功能：CRC校验用函数
参数：puchMsgg是要进行CRC校验的消息，usDataLen是消息中字节数
返回：计算出来的CRC校验码。
*****************************************************/
u16 CRC16(u8 *puchMsgg,u8 usDataLen)
{
    u8 uchCRCHi = 0xFF ; 											//高CRC字节初始化
    u8 uchCRCLo = 0xFF ; 											//低CRC 字节初始化
    u8 uIndex ; 													//CRC循环中的索引
    while (usDataLen--) 											//传输消息缓冲区
    {
		uIndex = uchCRCHi ^ *puchMsgg++; 							//计算CRC
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
    }
    return ((uchCRCHi << 8) | uchCRCLo);
}

//计算校验和
u8 CalCheckSum(u8 *buf, u16 len)
{
	u8 sum = 0;
	u16 i = 0;

	for(i = 0; i < len; i ++)
	{
		sum += *(buf + i);
	}

	return sum;
}

//产生一个系统1毫秒滴答时钟.
void SysTick1msAdder(void)
{
	SysTick1ms = (SysTick1ms + 1) & 0xFFFFFFFF;
}

//获取系统1毫秒滴答时钟
u32 GetSysTick1ms(void)
{
	return SysTick1ms;
}

//产生一个系统10毫秒滴答时钟.
void SysTick10msAdder(void)
{
	SysTick10ms = (SysTick10ms + 1) & 0xFFFFFFFF;
}

//获取系统10毫秒滴答时钟
u32 GetSysTick10ms(void)
{
	return SysTick10ms;
}

//产生一个系统100毫秒滴答时钟.
void SysTick100msAdder(void)
{
	SysTick100ms = (SysTick100ms + 1) & 0xFFFFFFFF;
}

//获取系统100毫秒滴答时钟
u32 GetSysTick100ms(void)
{
	return SysTick1ms;
}

void SetSysTick1s(time_t sec)
{
	SysTick1s = sec;
}

//获取系统1秒滴答时钟
time_t GetSysTick1s(void)
{
	return SysTick1s;
}

//从EEPROM中读取数据(带CRC16校验码)len包括CRC16校验码
u8 ReadDataFromEepromToHoldBuf(u8 *inbuf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 ReadCrcCode;
	u16 CalCrcCode = 0;

	for(i = s_add; i < s_add + len; i ++)
	{
		*(inbuf + i) = AT24CXX_ReadOneByte(i);
	}

	ReadCrcCode=(u16)(*(inbuf + s_add + len - 2));
	ReadCrcCode=ReadCrcCode<<8;
	ReadCrcCode=ReadCrcCode|(u16)(u16)(*(inbuf + s_add + len - 1));

	CalCrcCode = CRC16(inbuf + s_add,len - 2);

	if(ReadCrcCode == CalCrcCode)
	{
		return 1;
	}

	return 0;
}

//向EEPROM中写入数据(带CRC16校验码)len不包括CRC16校验码
void WriteDataFromHoldBufToEeprom(u8 *inbuf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 CalCrcCode = 0;

	CalCrcCode = CRC16(inbuf,len);
	*(inbuf + len + 0) = (u8)(CalCrcCode >> 8);
	*(inbuf + len + 1) = (u8)(CalCrcCode & 0x00FF);

	for(i = s_add ,j = 0; i < s_add + len + 2; i ++, j ++)
	{
		AT24CXX_WriteOneByte(i,*(inbuf + j));
	}
}

//将数字或者缓冲区当中的数据转换成字符串，并赋值给相应的指针
//type 0:转换数字id 1:转换缓冲区数据，add为缓冲区起始地址 2将字符串长度传到参数size中
u8 GetMemoryForString(u8 **str, u8 type, u32 id, u16 add, u16 size, u8 *hold_reg)
{
	u8 ret = 0;
	u8 len = 0;
	u8 new_len = 0;

	if(*str == NULL)
	{
		if(type == 0)
		{
			len = GetDatBit(id);
		}
		else if(type == 1)
		{
			len = *(hold_reg + add);
		}
		else if(type == 2)
		{
			len = size;
		}

		*str = (u8 *)mymalloc(sizeof(u8) * len + 1);
	}

	if(*str != NULL)
	{
		len = strlen((char *)*str);
		if(type == 0)
		{
			new_len = GetDatBit(id);
		}
		else if(type == 1)
		{
			new_len = *(hold_reg + add);
		}
		else if(type == 2)
		{
			new_len = size;

			add -= 1;
		}

		if(len == new_len)
		{
			memset(*str,0,new_len + 1);

			if(type == 0)
			{
				IntToString(*str,id,0);
			}
			else if(type == 1 || type == 2)
			{
				memcpy(*str,(hold_reg + add + 1),new_len);
			}
			ret = 1;
		}
		else
		{
			myfree(*str);
			*str = (u8 *)mymalloc(sizeof(u8) * new_len + 1);
			if(*str != NULL)
			{
				memset(*str,0,new_len + 1);

				if(type == 0)
				{
					IntToString(*str,id,0);
				}
				else if(type == 1 || type == 2)
				{
					memcpy(*str,(hold_reg + add + 1),new_len);
				}
				len = new_len;
				new_len = 0;
				ret = 1;
			}
		}
	}

	return ret;
}

//将字符串拷贝到指定地址
u8 CopyStrToPointer(u8 **pointer, u8 *str, u8 len)
{
	u8 ret = 0;

	if(*pointer == NULL)
	{
		*pointer = (u8 *)mymalloc(len + 1);
	}
	else if(*pointer != NULL)
	{
		myfree(*pointer);
		*pointer = (u8 *)mymalloc(sizeof(u8) * len + 1);
	}

	if(*pointer != NULL)
	{
		memset(*pointer,0,len + 1);

		memcpy(*pointer,str,len);

		ret = 1;
	}

	return ret;
}

//获取设备名称
u8 GetDeviceName(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&DeviceName, 1, 0, DEVICE_NAME_ADD, 0,HoldReg);

	return ret;
}

//获取设备ID
u8 GetDeviceID(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&DeviceID, 2, 0, DEVICE_ID_ADD, DEVICE_ID_LEN - 2, HoldReg);

	return ret;
}

//获取设备UUID
u8 GetDeviceUUID(void)
{
	u8 ret = 0;

	ret = GetMemoryForString(&DeviceUUID, 2, 0, UU_ID_ADD, UU_ID_LEN - 2, HoldReg);

	return ret;
}


//读取应用程序版本号
u8 ReadSoftWareVersion(void)
{
	u8 ret = 1;

	if(SoftWareVersion == NULL)
	{
		SoftWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
	}

	memset(SoftWareVersion,0,6);

	sprintf((char *)SoftWareVersion, "%02d.%02d", SOFT_WARE_VRESION / 100,SOFT_WARE_VRESION % 100);

	return ret;
}

//读取硬件版本号
u8 ReadHardWareVersion(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,HW_VER_ADD, HW_VER_LEN);

	if(ret)
	{
		if(HardWareVersion == NULL)
		{
			HardWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}

		memset(HardWareVersion,0,6);

		sprintf((char *)HardWareVersion, "%02d.%02d", HoldReg[HW_VER_ADD + 0],HoldReg[HW_VER_ADD + 1]);
	}
	else
	{
		if(HardWareVersion == NULL)
		{
			HardWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}

		memset(HardWareVersion,0,6);

		sprintf((char *)HardWareVersion, "null");
	}

	return ret;
}

//读取设备名称
u8 ReadDeviceName(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,DEVICE_NAME_ADD, DEVICE_NAME_LEN);

	if(ret)
	{
		GetDeviceName();
	}
	else
	{
		if(DeviceName == NULL)
		{
			DeviceName = (u8 *)mymalloc(sizeof(u8) * 5);
		}

		memset(DeviceName,0,5);

		sprintf((char *)DeviceName, "null");
	}

	return ret;
}

//读取设备ID
u8 ReadDeviceID(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,DEVICE_ID_ADD, DEVICE_ID_LEN);

	if(ret)
	{
		GetDeviceID();
	}
	else
	{
		if(DeviceID == NULL)
		{
			DeviceID = (u8 *)mymalloc(sizeof(u8) * 6);
		}

		memset(DeviceID,0,6);
	}

	return ret;
}

//读取设备UUID
u8 ReadDeviceUUID(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,UU_ID_ADD, UU_ID_LEN);

	if(ret)
	{
		GetDeviceUUID();
	}
//	else
//	{
//		if(DeviceUUID == NULL)
//		{
//			DeviceUUID = (u8 *)mymalloc(sizeof(u8) * 65);
//		}

//		memset(DeviceUUID,0,65);

//		sprintf((char *)DeviceUUID, "000000000000000000000000000000000000");
//	}

	return ret;
}

//读取数据上传间隔时间
u8 ReadUpLoadINVL(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,UPLOAD_INVL_ADD, UPLOAD_INVL_LEN);

	if(ret)
	{
		UpLoadINCL = (((u16)HoldReg[UPLOAD_INVL_ADD + 0]) << 8) + (u16)HoldReg[UPLOAD_INVL_ADD +1] & 0x00FF;

		if(UpLoadINCL > MAX_UPLOAD_INVL)
		{
			UpLoadINCL = 10;
		}
	}

	return ret;
}

//读取时间策略组数
u8 ReadTimeGroupNumber(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,TIME_GROUP_NUM_ADD, TIME_GROUP_NUM_LEN);

	if(ret)
	{
		if(HoldReg[TIME_GROUP_NUM_ADD] >= 2 && HoldReg[TIME_GROUP_NUM_ADD] <= MAX_GROUP_NUM)
		{
			TimeGroupNumber = HoldReg[TIME_GROUP_NUM_ADD];
		}
		else
		{
			TimeGroupNumber = 0;
		}
	}

	return ret;
}

//读取数据上传间隔时间
u8 ReadAllRelayState(void)
{
	u8 ret = 0;

	ret = ReadDataFromEepromToHoldBuf(HoldReg,RELAY_STATE_ADD, RELAY_STATE_LEN);

	if(ret)
	{
		AllRelayState = (((u16)HoldReg[RELAY_STATE_ADD + 0]) << 8) + (u16)HoldReg[RELAY_STATE_ADD +1] & 0x00FF;

		if(AllRelayState > 0x0FFF)
		{
			AllRelayState = 0x0000;
		}
	}

	return ret;
}

//将继电器输出状态写入EEPROM
u8 WriteAllRelayState(void)
{
	u8 ret = 0;
	
	HoldReg[RELAY_STATE_ADD + 0] = (u8)(AllRelayState >> 8);
	HoldReg[RELAY_STATE_ADD + 1] = (u8)(AllRelayState & 0x00FF);
	
	WriteDataFromHoldBufToEeprom(&HoldReg[RELAY_STATE_ADD],RELAY_STATE_ADD, RELAY_STATE_LEN - 2);
	
	return ret;
}

//设定OTA参数到EEPROM中
void WriteOTAInfo(u8 *hold_reg,u8 reset)
{
	u16 crc_code = 0;
	u16 i = 0;
	
	if(reset == 1)
	{
		HaveNewFirmWare   = 0;
		NewFirmWareBagNum = 0;
		NewFirmWareVer    = 101;
		LastBagByteNum    = 0;
	}
	
	if(NewFirmWareAdd != 0xAA && NewFirmWareAdd != 0x55)
	{
		NewFirmWareAdd = 0xAA;
	}
	
	*(hold_reg + FIRM_WARE_FLAG_S_ADD) 			= HaveNewFirmWare;
	*(hold_reg + FIRM_WARE_STORE_ADD_S_ADD) 	= NewFirmWareAdd;
	*(hold_reg + FIRM_WARE_VER_S_ADD + 0) 		= (u8)((NewFirmWareVer >> 8) & 0x00FF);
	*(hold_reg + FIRM_WARE_VER_S_ADD + 1) 		= (u8)(NewFirmWareVer & 0x00FF);
	*(hold_reg + FIRM_WARE_BAG_NUM_S_ADD + 0) 	= (u8)((NewFirmWareBagNum >> 8) & 0x00FF);
	*(hold_reg + FIRM_WARE_BAG_NUM_S_ADD + 1) 	= (u8)(NewFirmWareBagNum & 0x00FF);
	*(hold_reg + LAST_BAG_BYTE_NUM_S_ADD) 		= LastBagByteNum;

	crc_code = CRC16(hold_reg + OTA_INFO_ADD, OTA_INFO_LEN - 2);

	*(hold_reg + OTA_INFO_ADD + OTA_INFO_LEN - 2) = (u8)(crc_code >> 8);
	*(hold_reg + OTA_INFO_ADD + OTA_INFO_LEN - 1) = (u8)(crc_code & 0x00FF);

	for(i = OTA_INFO_ADD; i < OTA_INFO_ADD + OTA_INFO_LEN; i ++)
	{
		AT24CXX_WriteOneByte(i,*(hold_reg + i));
	}
}

//从EEPROM中读取OTA信息
u8 ReadOTAInfo(u8 *hold_reg)
{
	u8 ret = 0;
	
	ret = ReadDataFromEepromToHoldBuf(hold_reg,OTA_INFO_ADD,OTA_INFO_LEN);
	
	if(ret == 1)
	{
		HaveNewFirmWare 	= *(hold_reg + FIRM_WARE_FLAG_S_ADD);
		NewFirmWareAdd 		= *(hold_reg + FIRM_WARE_STORE_ADD_S_ADD);
		NewFirmWareVer 		= (((u16)(*(hold_reg + FIRM_WARE_VER_S_ADD + 0))) << 8) + \
								(u16)(*(hold_reg + FIRM_WARE_VER_S_ADD + 1));
		NewFirmWareBagNum 	= (((u16)(*(hold_reg + FIRM_WARE_BAG_NUM_S_ADD + 0))) << 8) + \
								(u16)(*(hold_reg + FIRM_WARE_BAG_NUM_S_ADD + 1));
		LastBagByteNum 		= *(hold_reg + LAST_BAG_BYTE_NUM_S_ADD);
	}
	else
	{
		WriteOTAInfo(HoldReg,1);		//复位OTA信息
	}
	
	return ret;
}

//读取时间策略数组
u8 ReadRegularTimeGroups(void)
{
	u8 ret = 0;
	u16 i = 0;
	u16 j = 0;
	u16 read_crc = 0;
	u16 cal_crc = 0;
	u8 time_group[256];
	u8 read_success_buf_flag[MAX_GROUP_NUM];

	ReadTimeGroupNumber();		//读取时间策略条数

	if(TimeGroupNumber != 0 && TimeGroupNumber % 2 == 0)
	{
		memset(time_group,0,256);
		memset(read_success_buf_flag,0,MAX_GROUP_NUM);

		for(i = 0; i < TimeGroupNumber; i ++)
		{
			for(j = i * 9; j < i * 9 + 9; j ++)
			{
				time_group[j] = AT24CXX_ReadOneByte(TIME_RULE_ADD + j);
			}

			cal_crc = CRC16(&time_group[j - 9],7);
			read_crc = (((u16)time_group[j - 2]) << 8) + (u16)time_group[j - 1];

			if(cal_crc == read_crc)
			{
				read_success_buf_flag[i] = 1;
			}
		}

		for(i = 0; i <= TimeGroupNumber / 2; i += 2)
		{
			if(read_success_buf_flag[i + 0] == 1 && read_success_buf_flag[i + 1] == 1)
			{
				RegularTimeStruct[i / 2].type 		= time_group[(i + 0) * 9 + 0];

				RegularTimeStruct[i / 2].s_year 	= time_group[(i + 0) * 9 + 1];
				RegularTimeStruct[i / 2].s_month 	= time_group[(i + 0) * 9 + 2];
				RegularTimeStruct[i / 2].s_date 	= time_group[(i + 0) * 9 + 3];
				RegularTimeStruct[i / 2].s_hour 	= time_group[(i + 0) * 9 + 4];
				RegularTimeStruct[i / 2].s_minute 	= time_group[(i + 0) * 9 + 5];

//				RegularTimeStruct[i / 2].percent 	= time_group[(i + 0) * 9 + 6];

				RegularTimeStruct[i / 2].e_year 	= time_group[(i + 1) * 9 + 1];
				RegularTimeStruct[i / 2].e_month 	= time_group[(i + 1) * 9 + 2];
				RegularTimeStruct[i / 2].e_date 	= time_group[(i + 1) * 9 + 3];
				RegularTimeStruct[i / 2].e_hour 	= time_group[(i + 1) * 9 + 4];
				RegularTimeStruct[i / 2].e_minute 	= time_group[(i + 1) * 9 + 5];

				RegularTimeStruct[i / 2].s_seconds = RegularTimeStruct[i / 2].s_hour * 3600 + RegularTimeStruct[i / 2].s_minute * 60;
				RegularTimeStruct[i / 2].e_seconds = RegularTimeStruct[i / 2].e_hour * 3600 + RegularTimeStruct[i / 2].e_minute * 60;
			}
		}

		for(i = 0; i <= TimeGroupNumber / 2; i += 2)
		{
			if(read_success_buf_flag[i + 0] != 1 || read_success_buf_flag[i + 1] != 1)
			{
				memcpy(&RegularTimeStruct[i / 2],&RegularTimeStruct[i + 2 / 2],sizeof(RegularTime_S));
				read_success_buf_flag[i + 2 + 0] = 0;
				read_success_buf_flag[i + 2 + 1] = 0;
			}
		}
	}

	return ret;
}


void ReadParametersFromEEPROM(void)
{
	ReadSoftWareVersion();
	ReadHardWareVersion();
	ReadDeviceName();
	ReadDeviceID();
	ReadDeviceUUID();
	ReadUpLoadINVL();
	ReadAllRelayState();
}

//将继电器状态和时间打包
u16 PackDataOfRelayInfo(u8 *outbuf)
{
	u8 len = 0;
	
	*(outbuf + 0) = (u8)(OutPutControlState >> 8);
	*(outbuf + 1) = (u8)(OutPutControlState & 0x00FF);
	*(outbuf + 2) = (u8)(AllRelayPowerState >> 8);
	*(outbuf + 3) = (u8)(AllRelayPowerState & 0x00FF);
	
	*(outbuf + 4) = calendar.hour;
	*(outbuf + 5) = calendar.min;
	*(outbuf + 6) = calendar.sec;
	
	len = 7;
	
	return len;
}

//将数据打包成网络格式的数据
u16 PackNetData(u8 dev_add,u8 fun_code,u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *outbuf)
{
	u16 len = 0;

	*(outbuf + 0) = 0x68;

	if(DeviceID != NULL)
	{
		memcpy(outbuf + 1,DeviceID,DEVICE_ID_LEN - 2);			//设备ID

		*(outbuf + 7) = 0x68;
		*(outbuf + 8) = dev_add;
		*(outbuf + 9) = fun_code;
		*(outbuf + 10) = cmd_id;
		*(outbuf + 11) = inbuf_len;

//		if(DeviceUUID != NULL)
//		{
//			memcpy(outbuf + 12,DeviceUUID,UU_ID_LEN - 2);		//UUID
//		}
//		else
//		{
//			memcpy(outbuf + 12,"000000000000000000000000000000000000",UU_ID_LEN - 2);	//默认UUID
//		}

		memcpy(outbuf + 12,inbuf,inbuf_len);	//具体数据内容

		*(outbuf + 12 + inbuf_len) = CalCheckSum(outbuf, 12 + inbuf_len);

		*(outbuf + 12 + inbuf_len + 1) = 0x16;

		*(outbuf + 12 + inbuf_len + 2) = 0xFE;
		*(outbuf + 12 + inbuf_len + 3) = 0xFD;
		*(outbuf + 12 + inbuf_len + 4) = 0xFC;
		*(outbuf + 12 + inbuf_len + 5) = 0xFB;
		*(outbuf + 12 + inbuf_len + 6) = 0xFA;
		*(outbuf + 12 + inbuf_len + 7) = 0xF9;

		len = 12 + inbuf_len + 7 + 1;
	}
	else
	{
		return 0;
	}

	return len;
}














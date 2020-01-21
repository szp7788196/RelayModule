#include "common.h"
#include "24cxx.h"
#include "relay.h"


u8 TimeStrategyNumber = 0;						//时间策略组数
u8 AppoinGroupNumber = 0;						//节日策略组数
pStrategyTime CurrentStrategy = NULL;			//被选中的策略组
pStrategyTime StrategyGroup[MAX_GROUP_NUM];		//总策略列表
NormalControl_S NormalControl;					//平日模式策略组
pAppointmentControl AppointmentControl;			//节日策略组
u8 ContrastTable[24];							//计量模块回路对照表



/****************************互斥量相关******************************/
SemaphoreHandle_t  xMutex_IIC1 			= NULL;	//IIC总线1的互斥量
SemaphoreHandle_t  xMutex_STRATEGY 		= NULL;	//AT指令的互斥量

QueueHandle_t xQueue_key 				= NULL;	//用于按键时间的消息队列
QueueHandle_t xQueue_RelayState 		= NULL;	//用于继电器状态变化的消息队列

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
u8 DeviceAreaID = 0xFF;				//设备逻辑区码
u8 DeviceBoxID = 0xFF;				//设备物理区码

/***************************运行参数相关*****************************/
u16 UpLoadINCL = 10;				//数据上传时间间隔0~65535秒
u8 GetTimeOK = 0;					//成功获取时间标志
u8 DeviceWorkMode = 0;				//运行模式，0：自动，1：手动
u16 RelayActionINCL = 100;			//数据上传时间间隔0~65535毫秒
u32 RS485BuadRate = 9600;			//通讯波特率

/***************************其他*****************************/
u8 RefreshStrategy = 1;						//刷新策略列表

u8 NeedToReset = 0;					//复位/重启标志
u16 OutPutControlBit = 0;			//开出位标志
u16 OutPutControlState = 0;			//开出位标志(具体哪几位)
u16 RelaysState = 0;				//各个继电器的状态
u16 AllRelayPowerState = 0;			//继电器输入端是否带电
u16 AllRelayState = 0;				//继电器的状态
u8 HaveNewActionCommand = 0;		//有新的动作指令

/*************************固件升级相关***************************/
FrameWareInfo_S FrameWareInfo;				//固件信息
FrameWareState_S FrameWareState;			//固件升级状态

/*************************天文时间相关***************************/
Location_S Location;
SunRiseSetTime_S SunRiseSetTime;

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
//CRC32
u32 CRC32(const u8 *buf, u32 size, u32 temp,u8 flag)
{
	uint32_t i, crc,crc_e;

	crc = temp;
	for (i = 0; i < size; i++)
	{
		crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
	}

	if(flag != 0)
	{
		crc_e = crc^0xFFFFFFFF;
	}
	else if(flag == 0)
	{
		crc_e = crc;
	}
	return crc_e;
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

u16 GetCRC16(u8 *data,u16 len)
{
	u16 ax,lsb;
	u8 temp1 = 0;
	u8 temp2 = 0;
	int i,j;

	ax = 0xFFFF;

	for(i = 0; i < len; i ++)
	{
		ax ^= data[i];

		for(j = 0; j < 8; j ++)
		{
			lsb = ax & 0x0001;
			ax = ax >> 1;

			if(lsb != 0)
				ax ^= 0xA001;
		}
	}

	temp1 = (u8)((ax >> 8) & 0x00FF);
	temp2 = (u8)(ax & 0x00FF);

	ax = ((((u16)temp2) << 8) & 0xFF00) + (u16)temp1;

	return ax;
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

//获取系统时间状态
u8 GetSysTimeState(void)
{
	u8 ret = 0;

	if(calendar.w_year >= 2019)
	{
		ret = 2;
	}

	return ret;
}

//闰年判断
u8 leap_year_judge(u16 year)
{
	u16 leap = 0;

	if(year % 400 == 0)
	{
		leap = 1;
	}
    else
    {
        if(year % 4 == 0 && year % 100 != 0)
		{
			leap = 1;
		}
        else
		{
			leap = 0;
		}
	}

	return leap;
}

//闰年判断 返回当前年月日 在2000年开始到今天的天数
u32 get_days_form_calendar(u16 year,u8 month,u8 date)
{
	u16 i = 0;
	u8 leap = 0;
	u32 days = 0;
	u8 x[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};

	for(i = 2000; i <= year; i ++)
	{
		leap = leap_year_judge(i);

		if(leap == 1)
		{
			days += 366;
		}
		else if(leap == 0)
		{
			days += 365;
		}
	}

	leap = leap_year_judge(year);

	if(leap == 1)
	{
		x[2] = 29;
	}
	else if(leap == 0)
	{
		x[2] = 28;
	}

	for(i = 1; i < month; i ++)
	{
		days += x[i];			//整月的天数
	}

	days += (u16)date;			//日的天数

	return days;
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

//在FLASH中的指定位置读取一个字节
u8 STMFLASH_ReadByte(u32 faddr)
{
	return *(vu8*)faddr;
}

//按字节读取FLASH指定地址
void STMFLASH_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	u16 i = 0;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadByte(ReadAddr);
		ReadAddr++;
	}
}

////从EEPROM中读取数据(带CRC16校验码)len包括CRC16校验码
//u8 ReadDataFromEepromToHoldBuf(u8 *inbuf,u16 s_add, u16 len)
//{
//	u16 i = 0;
//	u16 ReadCrcCode;
//	u16 CalCrcCode = 0;

//	for(i = s_add; i < s_add + len; i ++)
//	{
//		*(inbuf + i) = AT24CXX_ReadOneByte(i);
//	}

//	ReadCrcCode=(u16)(*(inbuf + s_add + len - 2));
//	ReadCrcCode=ReadCrcCode<<8;
//	ReadCrcCode=ReadCrcCode|(u16)(u16)(*(inbuf + s_add + len - 1));

//	CalCrcCode = CRC16(inbuf + s_add,len - 2);

//	if(ReadCrcCode == CalCrcCode)
//	{
//		return 1;
//	}

//	return 0;
//}

////向EEPROM中写入数据(带CRC16校验码)len不包括CRC16校验码
//void WriteDataFromHoldBufToEeprom(u8 *inbuf,u16 s_add, u16 len)
//{
//	u16 i = 0;
//	u16 j = 0;
//	u16 CalCrcCode = 0;

//	CalCrcCode = CRC16(inbuf,len);
//	*(inbuf + len + 0) = (u8)(CalCrcCode >> 8);
//	*(inbuf + len + 1) = (u8)(CalCrcCode & 0x00FF);

//	for(i = s_add ,j = 0; i < s_add + len + 2; i ++, j ++)
//	{
//		AT24CXX_WriteOneByte(i,*(inbuf + j));
//	}
//}

//从EEPROM中读取数据(带CRC16校验码)len包括CRC16校验码
u8 ReadDataFromEepromToMemory(u8 *buf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 ReadCrcCode;
	u16 CalCrcCode = 0;

	for(i = s_add,j = 0; i < s_add + len; i ++, j++)
	{
		*(buf + j) = AT24CXX_ReadOneByte(i);
	}

	ReadCrcCode = (u16)(*(buf + len - 2));
	ReadCrcCode = ReadCrcCode << 8;
	ReadCrcCode = ReadCrcCode | (u16)(*(buf + len - 1));

	CalCrcCode = GetCRC16(buf,len - 2);

	if(ReadCrcCode == CalCrcCode)
	{
		return 1;
	}

	return 0;
}

//向EEPROM中写入数据(带CRC16校验码)len不包括CRC16校验码
void WriteDataFromMemoryToEeprom(u8 *inbuf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 CalCrcCode = 0;

	CalCrcCode = GetCRC16(inbuf,len);

	for(i = s_add ,j = 0; i < s_add + len; i ++, j ++)			//写入原始数据
	{
		AT24CXX_WriteOneByte(i,*(inbuf + j));
	}

	AT24CXX_WriteOneByte(s_add + len + 0,(u8)(CalCrcCode >> 8));		//写入CRC
	AT24CXX_WriteOneByte(s_add + len + 1,(u8)(CalCrcCode & 0x00FF));
}

////将数字或者缓冲区当中的数据转换成字符串，并赋值给相应的指针
////type 0:转换数字id 1:转换缓冲区数据，add为缓冲区起始地址 2将字符串长度传到参数size中
//u8 GetMemoryForString(u8 **str, u8 type, u32 id, u16 add, u16 size, u8 *hold_reg)
//{
//	u8 ret = 0;
//	u8 len = 0;
//	u8 new_len = 0;

//	if(*str == NULL)
//	{
//		if(type == 0)
//		{
//			len = GetDatBit(id);
//		}
//		else if(type == 1)
//		{
//			len = *(hold_reg + add);
//		}
//		else if(type == 2)
//		{
//			len = size;
//		}

//		*str = (u8 *)mymalloc(sizeof(u8) * len + 1);
//	}

//	if(*str != NULL)
//	{
//		len = strlen((char *)*str);
//		if(type == 0)
//		{
//			new_len = GetDatBit(id);
//		}
//		else if(type == 1)
//		{
//			new_len = *(hold_reg + add);
//		}
//		else if(type == 2)
//		{
//			new_len = size;

//			add -= 1;
//		}

//		if(len == new_len)
//		{
//			memset(*str,0,new_len + 1);

//			if(type == 0)
//			{
//				IntToString(*str,id,0);
//			}
//			else if(type == 1 || type == 2)
//			{
//				memcpy(*str,(hold_reg + add + 1),new_len);
//			}
//			ret = 1;
//		}
//		else
//		{
//			myfree(*str);
//			*str = (u8 *)mymalloc(sizeof(u8) * new_len + 1);
//			if(*str != NULL)
//			{
//				memset(*str,0,new_len + 1);

//				if(type == 0)
//				{
//					IntToString(*str,id,0);
//				}
//				else if(type == 1 || type == 2)
//				{
//					memcpy(*str,(hold_reg + add + 1),new_len);
//				}
//				len = new_len;
//				new_len = 0;
//				ret = 1;
//			}
//		}
//	}

//	return ret;
//}

//将内存中的数据拷贝到指定指针所指的内存中
u8 GetMemoryForSpecifyPointer(u8 **str,u16 size, u8 *memory)
{
	u8 ret = 0;
	u8 len = 0;
	u8 new_len = 0;

	if(*str == NULL)
	{
		len = size;

		*str = (u8 *)mymalloc(sizeof(u8) * len + 1);
	}

	if(*str != NULL)
	{
		len = strlen((char *)*str);

		new_len = size;

		if(len == new_len)
		{
			memset(*str,0,new_len + 1);

			memcpy(*str,memory,new_len);

			ret = 1;
		}
		else
		{
			myfree(*str);
			*str = (u8 *)mymalloc(sizeof(u8) * new_len + 1);

			if(*str != NULL)
			{
				memset(*str,0,new_len + 1);

				memcpy(*str,memory,new_len);

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

//读取逻辑区号
u8 ReadDeviceAreaID(void)
{
	u8 ret = 0;

	u8 temp_buf[AREA_ID_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,AREA_ID_ADD, AREA_ID_LEN);

	if(ret)
	{
		if(temp_buf[0] >= 0xFE)
		{
			return 0;
		}

		DeviceAreaID = temp_buf[0];
	}
	else
	{
		DeviceAreaID = 0x01;
	}

	return ret;
}

//读取物理区号
u8 ReadDeviceBoxID(void)
{
	u8 ret = 0;

	u8 temp_buf[BOX_ID_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,BOX_ID_ADD, BOX_ID_LEN);

	if(ret)
	{
		if(temp_buf[0] >= 0xFE)
		{
			return 0;
		}

		DeviceBoxID = temp_buf[0];
	}
	else
	{
		DeviceBoxID = 0x01;
	}

	return ret;
}

//读取位置信息
u8 ReadPosition(void)
{
	u8 ret = 0;
	u8 i = 0;
	u8 buf[POSITION_LEN];
	u8 temp_buf[8];

	ret = ReadDataFromEepromToMemory(buf,POSITION_ADD, POSITION_LEN);

	if(ret)
	{
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
	}
	else
	{
		Location.longitude = 116.397128f;
		Location.latitude = 39.916527f;
	}

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

	u8 temp_buf[HW_VER_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,HW_VER_ADD, HW_VER_LEN);

	if(ret)
	{
		if(HardWareVersion == NULL)
		{
			HardWareVersion = (u8 *)mymalloc(sizeof(u8) * 6);
		}

		memset(HardWareVersion,0,6);

		sprintf((char *)HardWareVersion, "%02d.%02d", temp_buf[0],temp_buf[1]);
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

	u8 temp_buf[DEVICE_NAME_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,DEVICE_NAME_ADD, DEVICE_NAME_LEN);

	if(ret)
	{
		GetMemoryForSpecifyPointer(&DeviceName,DEVICE_NAME_LEN - 2, temp_buf + 1);
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

	u8 temp_buf[UU_ID_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,DEVICE_ID_ADD, DEVICE_ID_LEN);

	if(ret)
	{
		if(DeviceID == NULL)
		{
			DeviceID = (u8 *)mymalloc(sizeof(u8) * DEVICE_ID_LEN);
		}

		memcpy(DeviceID,temp_buf,DEVICE_ID_LEN - 2);
	}
	else
	{
		if(DeviceID == NULL)
		{
			DeviceID = (u8 *)mymalloc(sizeof(u8) * DEVICE_ID_LEN);
		}

		memset(DeviceID,0,DEVICE_ID_LEN);

		DeviceID[5] = 0x02;
	}

	return ret;
}

//读取设备UUID
u8 ReadDeviceUUID(void)
{
	u8 ret = 0;

	u8 temp_buf[UU_ID_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,UU_ID_ADD, UU_ID_LEN);

	if(ret)
	{
		GetMemoryForSpecifyPointer(&DeviceUUID,UU_ID_LEN - 2, temp_buf);
	}
	else
	{
		if(DeviceUUID == NULL)
		{
			DeviceUUID = (u8 *)mymalloc(sizeof(u8) * UU_ID_LEN);
		}

		memset(DeviceUUID,0,UU_ID_LEN);

		sprintf((char *)DeviceUUID, "00000000000000001");
	}

	return ret;
}

//读取数据上传间隔时间
u8 ReadUpLoadINVL(void)
{
	u8 ret = 0;

	u8 temp_buf[UPLOAD_INVL_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,UPLOAD_INVL_ADD, UPLOAD_INVL_LEN);

	if(ret)
	{
		UpLoadINCL = (((u16)temp_buf[0]) << 8) + (u16)temp_buf[1];

		if(UpLoadINCL > MAX_UPLOAD_INVL)
		{
			UpLoadINCL = 10;
		}
	}

	return ret;
}

//读取继电器动作间隔
u8 ReadRelayActionINCL(void)
{
	u8 ret = 0;

	u8 temp_buf[REALY_ACTION_INVL_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,REALY_ACTION_INVL_ADD, REALY_ACTION_INVL_LEN);

	if(ret)
	{
		RelayActionINCL = (((u16)temp_buf[0]) << 8) + (u16)temp_buf[1];

		if(RelayActionINCL > MAX_REALY_ACTION_INVL)
		{
			RelayActionINCL = 100;
		}
	}

	return ret;
}

//读取通讯波特率
u8 ReadRS485BuadRate(void)
{
	u8 ret = 0;

	u8 temp_buf[RS485_BUAD_RATE_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,RS485_BUAD_RATE_ADD, RS485_BUAD_RATE_LEN);

	if(ret)
	{
		RS485BuadRate = (((u32)temp_buf[0]) << 24) +
						(((u32)temp_buf[1]) << 16) +
						(((u32)temp_buf[2]) << 8) +
						(u32)temp_buf[3];

		if(RS485BuadRate > 115200 || RS485BuadRate < 1200)
		{
			RS485BuadRate = 9600;
		}
	}

	return ret;
}

//读取时间策略组数
u8 ReadTimeGroupNumber(void)
{
	u8 ret = 0;

	u8 temp_buf[STRATEGY_GROUP_NUM_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,STRATEGY_GROUP_NUM_ADD, STRATEGY_GROUP_NUM_LEN);

	if(ret)
	{
		if(temp_buf[0] <= MAX_STRATEGY_NUM)
		{
			TimeStrategyNumber = temp_buf[0];
		}
		else
		{
			TimeStrategyNumber = 0;
		}
	}

	return ret;
}

//读取节日策略组数
u8 ReadAppionGroupNumber(void)
{
	u8 ret = 0;

	u8 temp_buf[APPOIN_GROUP_NUM_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,APPOIN_GROUP_NUM_ADD, APPOIN_GROUP_NUM_LEN);

	if(ret)
	{
		if(temp_buf[0] <= MAX_GROUP_NUM)
		{
			AppoinGroupNumber = temp_buf[0];
		}
		else
		{
			AppoinGroupNumber = 0;
		}
	}

	return ret;
}

//读取继电器状态
u8 ReadAllRelayState(void)
{
	u8 ret = 0;

	u8 temp_buf[RELAY_STATE_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,RELAY_STATE_ADD, RELAY_STATE_LEN);

	if(ret)
	{
		AllRelayState = (((u16)temp_buf[0]) << 8) + (u16)temp_buf[1] & 0x00FF;

		if(AllRelayState > 0x0FFF)
		{
			AllRelayState = 0x0000;
		}
	}

	return ret;
}

//读取位置信息
u8 ReadContrastTable(void)
{
	u8 ret = 0;

	u8 temp_buf[CONTRAST_TABLE_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,CONTRAST_TABLE_ADD, CONTRAST_TABLE_LEN);

	if(ret)
	{
		memcpy(ContrastTable,temp_buf,CONTRAST_TABLE_LEN - 2);
	}
	else
	{
		memset(ContrastTable,0,CONTRAST_TABLE_LEN - 2);
	}

	return ret;
}

//将继电器输出状态写入EEPROM
u8 WriteAllRelayState(void)
{
	u8 ret = 0;

	u8 temp_buf[RELAY_STATE_LEN];

	temp_buf[0] = (u8)(AllRelayState >> 8);
	temp_buf[1] = (u8)(AllRelayState & 0x00FF);

	WriteDataFromMemoryToEeprom(temp_buf,RELAY_STATE_ADD,RELAY_STATE_LEN - 2);

	return ret;
}

//读取固件信息
u8 ReadFrameWareInfo(void)
{
	u8 ret = 0;

	u8 temp_buf[SOFT_WARE_INFO_LEN];

	ret = ReadDataFromEepromToMemory(temp_buf,SOFT_WARE_INFO_ADD, SOFT_WARE_INFO_LEN);

	if(ret)
	{
		FrameWareInfo.version = (((u16)temp_buf[0]) << 8) + (u16)temp_buf[1];

		FrameWareInfo.length = (((u32)temp_buf[2]) << 24) +
							   (((u32)temp_buf[3]) << 16) +
							   (((u32)temp_buf[4]) << 8) +
							   (((u32)temp_buf[5]) << 0);
	}
	else
	{
		FrameWareInfo.version = 101;

		FrameWareInfo.length = 0;
	}

	return ret;
}

//将固件升级状态写入到EEPROM
void WriteFrameWareStateToEeprom(void)
{
	u8 temp_buf[UPDATE_STATE_LEN];

	temp_buf[0]  = FrameWareState.state;
	temp_buf[1]  = (u8)(FrameWareState.total_bags >> 8);
	temp_buf[2]  = (u8)FrameWareState.total_bags;
	temp_buf[3]  = (u8)(FrameWareState.current_bag_cnt >> 8);
	temp_buf[4]  = (u8)FrameWareState.current_bag_cnt;
	temp_buf[5]  = (u8)(FrameWareState.bag_size >> 8);
	temp_buf[6]  = (u8)FrameWareState.bag_size;
	temp_buf[7]  = (u8)(FrameWareState.last_bag_size >> 8);
	temp_buf[8]  = (u8)FrameWareState.last_bag_size;
	temp_buf[9]  = (u8)(FrameWareState.total_size >> 24);
	temp_buf[10] = (u8)(FrameWareState.total_size >> 16);
	temp_buf[11] = (u8)(FrameWareState.total_size >> 8);
	temp_buf[12] = (u8)FrameWareState.total_size;

	WriteDataFromMemoryToEeprom(temp_buf,UPDATE_STATE_ADD,UPDATE_STATE_LEN - 2);
}

//读取固件设计状态
u8 ReadFrameWareState(void)
{
	u8 ret = 0;
	u16 page_num = 0;
	u16 i = 0;
	u8 remp_buf[UPDATE_STATE_LEN];

	ret = ReadDataFromEepromToMemory(remp_buf,UPDATE_STATE_ADD,UPDATE_STATE_LEN);

	if(ret == 1)
	{
		FrameWareState.state 			= remp_buf[0];
		FrameWareState.total_bags 		= ((((u16)remp_buf[1]) << 8) & 0xFF00) +
		                                  (((u16)remp_buf[2]) & 0x00FF);
		FrameWareState.current_bag_cnt 	= ((((u16)remp_buf[3]) << 8) & 0xFF00) +
		                                  (((u16)remp_buf[4]) & 0x00FF);
		FrameWareState.bag_size 		= ((((u16)remp_buf[5]) << 8) & 0xFF00) +
		                                  (((u16)remp_buf[6]) & 0x00FF);
		FrameWareState.last_bag_size 	= ((((u16)remp_buf[7]) << 8) & 0xFF00) +
		                                  (((u16)remp_buf[8]) & 0x00FF);

		FrameWareState.total_size 		= ((((u32)remp_buf[9]) << 24) & 0xFF000000) +
								          ((((u32)remp_buf[10]) << 16) & 0x00FF0000) +
								          ((((u32)remp_buf[11]) << 8) & 0x0000FF00) +
								          ((((u32)remp_buf[12]) << 0) & 0x000000FF);

		ret = 1;
	}
	else
	{
		RESET_STATE:
		FrameWareState.state 			= FIRMWARE_FREE;
		FrameWareState.total_bags 		= 0;
		FrameWareState.current_bag_cnt 	= 0;
		FrameWareState.bag_size 		= 0;
		FrameWareState.last_bag_size 	= 0;

		FrameWareState.total_size 		= 0;

		WriteFrameWareStateToEeprom();			//将默认值写入EEPROM
	}

	if(FrameWareState.state == FIRMWARE_DOWNLOADING ||
	   FrameWareState.state == FIRMWARE_DOWNLOAD_WAIT)
	{
		page_num = (FIRMWARE_MAX_FLASH_ADD - FIRMWARE_BUCKUP_FLASH_BASE_ADD) / 2048;	//得到备份区的扇区总数

		FLASH_Unlock();						//解锁FLASH

		for(i = 0; i < page_num; i ++)
		{
			FLASH_ErasePage(i * 2048 + FIRMWARE_BUCKUP_FLASH_BASE_ADD);	//擦除当前FLASH扇区
		}

		FLASH_Lock();						//上锁
	}

	if(FrameWareState.state == FIRMWARE_UPDATE_SUCCESS)
	{
//		UpdateSoftWareVer();
//		UpdateSoftWareReleaseDate();

		goto RESET_STATE;
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
	s16 minutes = 0;
	u8 time_group[MAX_STRATEGY_NUM * STRATEGY_CONTENT_LEN];
	u8 read_success_buf_flag[MAX_STRATEGY_NUM];

	for(i = 0; i < MAX_GROUP_NUM; i ++)
	{
		StrategyGroup[i] = (pStrategyTime)mymalloc(sizeof(StrategyTime_S));

		StrategyGroup[i]->group = i + 1;
		StrategyGroup[i]->type = 0xFF;
		StrategyGroup[i]->hour = 0;
		StrategyGroup[i]->minute = 0;
		StrategyGroup[i]->offset_min = 0;
		StrategyGroup[i]->control_bit = 0;
		StrategyGroup[i]->control_state = 0;
		StrategyGroup[i]->prev = NULL;
		StrategyGroup[i]->next = NULL;
	}

	memset(time_group,0,MAX_STRATEGY_NUM * STRATEGY_CONTENT_LEN);
	memset(read_success_buf_flag,0,MAX_STRATEGY_NUM);

	for(i = 0; i < TimeStrategyNumber; i ++)
	{
		for(j = i * STRATEGY_CONTENT_LEN; j < i * STRATEGY_CONTENT_LEN + STRATEGY_CONTENT_LEN; j ++)
		{
			time_group[j] = AT24CXX_ReadOneByte(STRATEGY_CONTENT_ADD + j);
		}

		cal_crc = CRC16(&time_group[j - STRATEGY_CONTENT_LEN],(STRATEGY_CONTENT_LEN - 2));
		read_crc = (((u16)time_group[j - 2]) << 8) + (u16)time_group[j - 1];

		if(cal_crc == read_crc)
		{
			read_success_buf_flag[i] = 1;
		}
	}

	for(i = 0; i < TimeStrategyNumber; i ++)
	{
		if(read_success_buf_flag[i] == 1)
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
	}

	return ret;
}

//读取平常策略组
u8 ReadNormalGroup(void)
{
	u8 ret = 0;

	u8 remp_buf[NORMAL_STRATEGY_GROUP_LEN];

	ret = ReadDataFromEepromToMemory(remp_buf,NORMAL_STRATEGY_GROUP_ADD, NORMAL_STRATEGY_GROUP_LEN);

	if(ret)
	{
		NormalControl.week_enable 		= remp_buf[0];
		NormalControl.cycle_min 		= (((u16)remp_buf[1]) << 8) + (u16)remp_buf[2];
		NormalControl.strategy_group 	= remp_buf[5];
	}
	else
	{
		NormalControl.week_enable 		= 0;
		NormalControl.cycle_min 		= 0;
		NormalControl.strategy_group 	= 0;
	}

	return ret;
}

//读取节日策略
u8 ReadAppointmentGroups(void)
{
	u8 ret = 0;
	u16 i = 0;
	u16 j = 0;
	u16 read_crc = 0;
	u16 cal_crc = 0;

	u8 time_group[MAX_GROUP_NUM * APPOIN_STRATEGY_GROUP_LEN];
	u8 read_success_buf_flag[MAX_GROUP_NUM];

	AppointmentControl = (pAppointmentControl)mymalloc(sizeof(AppointmentControl_S));

	AppointmentControl->prev 			= NULL;
	AppointmentControl->next 			= NULL;

	AppointmentControl->number 			= 0;
	AppointmentControl->s_year 			= 0;
	AppointmentControl->s_month 		= 0;
	AppointmentControl->s_date 			= 0;
	AppointmentControl->s_hour 			= 0;
	AppointmentControl->s_minute 		= 0;

	AppointmentControl->e_year 			= 0;
	AppointmentControl->e_month 		= 0;
	AppointmentControl->e_date 			= 0;
	AppointmentControl->e_hour 			= 0;
	AppointmentControl->e_minute 		= 0;

	AppointmentControl->week_enable 	= 0;

	AppointmentControl->cycle_min 		= 0;

	AppointmentControl->strategy_group 	= 0;

	memset(time_group,0,MAX_GROUP_NUM * APPOIN_STRATEGY_GROUP_LEN);
	memset(read_success_buf_flag,0,MAX_GROUP_NUM);

	for(i = 0; i < AppoinGroupNumber; i ++)
	{
		for(j = i * APPOIN_STRATEGY_GROUP_LEN; j < i * APPOIN_STRATEGY_GROUP_LEN + APPOIN_STRATEGY_GROUP_LEN; j ++)
		{
			time_group[j] = AT24CXX_ReadOneByte(APPOIN_STRATEGY_GROUP_ADD + j);
		}

		cal_crc = CRC16(&time_group[j - APPOIN_STRATEGY_GROUP_LEN],(APPOIN_STRATEGY_GROUP_LEN - 2));
		read_crc = (((u16)time_group[j - 2]) << 8) + (u16)time_group[j - 1];

		if(cal_crc == read_crc)
		{
			read_success_buf_flag[i] = 1;
		}
	}

	for(i = 0; i < AppoinGroupNumber; i ++)
	{
		if(read_success_buf_flag[i] == 1)
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
	ReadDeviceAreaID();
	ReadDeviceBoxID();
	ReadPosition();
	ReadUpLoadINVL();
	ReadRelayActionINCL();
	ReadRS485BuadRate();
	ReadAllRelayState();
	ReadFrameWareInfo();
	ReadFrameWareState();
	ReadContrastTable();
	ReadTimeGroupNumber();
	ReadAppionGroupNumber();
	ReadRegularTimeGroups();
	ReadNormalGroup();
	ReadAppointmentGroups();
}

//将继电器状态和时间打包
u16 PackDataOfRelayInfo(u8 *outbuf)
{
	u8 len = 0;
	u16 relays_power_stste = 0;

#ifndef FORWARD
	u8 i = 0;

	for(i = 0; i < CH_NUM; i ++)
	{
		if(AllRelayPowerState & (1 << (CH_NUM - 1 - i)))
		{
			relays_power_stste |= (1 << i);
		}
		else
		{
			relays_power_stste &= ~(1 << i);
		}
	}
#endif

	*(outbuf + 0) = (u8)(RelaysState >> 8);
	*(outbuf + 1) = (u8)(RelaysState & 0x00FF);
	*(outbuf + 2) = (u8)(relays_power_stste >> 8);
	*(outbuf + 3) = (u8)(relays_power_stste & 0x00FF);

	*(outbuf + 4) = ((((u8)(calendar.w_year - 2000)) / 10) << 4) | (((u8)(calendar.w_year - 2000)) % 10);
	*(outbuf + 5) = ((calendar.w_month / 10) << 4) | (calendar.w_month % 10);
	*(outbuf + 6) = ((calendar.w_date / 10) << 4) | (calendar.w_date % 10);
	*(outbuf + 7) = ((calendar.hour / 10) << 4) | (calendar.hour % 10);
	*(outbuf + 8) = ((calendar.min / 10) << 4) | (calendar.min % 10);
	*(outbuf + 9) = ((calendar.sec / 10) << 4) | (calendar.sec % 10);

	len = 10;

	return len;
}

//将数据打包成网络格式的数据
u16 PackNetData(u8 fun_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf,u8 id_type)
{
	u8 i = 0;
	u16 len = 0;

	*(outbuf + 0) = 0x68;

	if(DeviceID != NULL)
	{
		memcpy(outbuf + 1,DeviceID,DEVICE_ID_LEN - 2);			//设备ID

		*(outbuf + 7) = 0x68;
		*(outbuf + 8) = DeviceAreaID;
		*(outbuf + 9) = DeviceBoxID;


		if(DeviceUUID != NULL)
		{
			memcpy(outbuf + 10,DeviceUUID,UU_ID_LEN - 2);		//UUID
		}
		else
		{
			memcpy(outbuf + 10,"00000000000000001",UU_ID_LEN - 2);	//默认UUID
		}

		if(id_type == 0)
		{
			for(i = 0; i < 17; i ++)
			{
				*(outbuf + 10 + i) = *(outbuf + 10 + i) - 0x30;
			}
		}

		*(outbuf + 27) = fun_code;
		*(outbuf + 28) = (u8)((inbuf_len >> 8) & 0x00FF);
		*(outbuf + 29) = (u8)(inbuf_len & 0x00FF);

		memcpy(outbuf + 30,inbuf,inbuf_len);	//具体数据内容

		*(outbuf + 30 + inbuf_len + 0) = CalCheckSum(outbuf, 30 + inbuf_len);

		*(outbuf + 30 + inbuf_len + 1) = 0x16;

		*(outbuf + 30 + inbuf_len + 2) = 0xFE;
		*(outbuf + 30 + inbuf_len + 3) = 0xFD;
		*(outbuf + 30 + inbuf_len + 4) = 0xFC;
		*(outbuf + 30 + inbuf_len + 5) = 0xFB;
		*(outbuf + 30 + inbuf_len + 6) = 0xFA;
		*(outbuf + 30 + inbuf_len + 7) = 0xF9;

		len = 30 + inbuf_len + 7 + 1;
	}
	else
	{
		return 0;
	}

	return len;
}

u8 RegularTimeGroupAdd(pStrategyTime group_time)
{
	u8 ret = 1;
	u8 i = 0;
	pStrategyTime main_time = NULL;
	pStrategyTime tmp_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	if(group_time->group >= 1 && group_time->group <= MAX_GROUP_NUM)
	{
		for(i = 0; i < MAX_GROUP_NUM; i ++)
		{
			if(group_time->group == StrategyGroup[i]->group)
			{
				main_time = StrategyGroup[i];

				break;
			}
		}
	}

	if(main_time != NULL)
	{
		for(tmp_time = main_time; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			if(tmp_time->next == NULL)
			{
				tmp_time->next = group_time;
				tmp_time->next->prev = tmp_time;

				break;
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}

	return ret;
}

u8 RegularTimeGroupSub(u16 number)
{
	u8 ret = 0;
	u16 i = 0;
	pStrategyTime tmp_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	for(i = 0; i < MAX_GROUP_NUM; i ++)
	{
		if(StrategyGroup[i] != NULL && StrategyGroup[i]->next != NULL)
		{
			for(tmp_time = StrategyGroup[i]->next; tmp_time != NULL; tmp_time = tmp_time->next)
			{
				if(tmp_time->number == number)
				{
					if(tmp_time->next != NULL)
					{
						tmp_time->prev->next = tmp_time->next;
						tmp_time->next->prev = tmp_time->prev;
					}
					else
					{
						tmp_time->prev->next = NULL;
					}

					myfree(tmp_time);

					ret = 1;
				}
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}

	return ret;
}

void RemoveAllStrategy(void)
{
	u16 i = 0;

	for(i = 0; i < TimeStrategyNumber; i ++)
	{
		RegularTimeGroupSub(i);

		AT24CXX_WriteLenByte(STRATEGY_CONTENT_ADD + STRATEGY_CONTENT_LEN * i + (STRATEGY_CONTENT_LEN - 2),0x0000,2);
	}
}

u8 AppointmentGroupAdd(pAppointmentControl group_time)
{
	u8 ret = 1;
	pAppointmentControl main_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	if(AppointmentControl != NULL)
	{
		for(main_time = AppointmentControl; main_time != NULL; main_time = main_time->next)
		{
			if(main_time->next == NULL)
			{
				main_time->next = group_time;
				main_time->next->prev = main_time;

				break;
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}

	return ret;
}

u8 AppointmentGroupSub(u16 number)
{
	u8 ret = 0;
	pAppointmentControl tmp_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	if(AppointmentControl != NULL || AppointmentControl->next != NULL)
	{
		for(tmp_time = AppointmentControl->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			if(tmp_time->number == number)
			{
				if(tmp_time->next != NULL)
				{
					tmp_time->prev->next = tmp_time->next;
					tmp_time->next->prev = tmp_time->prev;
				}
				else
				{
					tmp_time->prev->next = NULL;
				}

				myfree(tmp_time);

				ret = 1;
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}

	return ret;
}

void RemoveAllAppointmentStrategy(void)
{
	u16 i = 0;

	for(i = 0; i < AppoinGroupNumber; i ++)
	{
		AppointmentGroupSub(i);

		AT24CXX_WriteLenByte(APPOIN_STRATEGY_GROUP_ADD + APPOIN_STRATEGY_GROUP_LEN * i + (APPOIN_STRATEGY_GROUP_LEN - 2),0x0000,2);
	}
}

//获取当前的策略组
u8 GetCurrentStrategy(void)
{
	u8 ret = 0;
	u8 i = 0;

	u32 gate_day_s = 0;
	u32 gate_day_e = 0;
	u32 gate_day_n = 0;

	u32 gate_minute_s = 0;
	u32 gate_minute_e = 0;
	u32 gate_minute_n = 0;

	pAppointmentControl tmp_time = NULL;

	if(NormalControl.strategy_group >= 1 && NormalControl.strategy_group <= MAX_GROUP_NUM)		//组号合法
	{
		for(i = 0; i < MAX_GROUP_NUM; i ++)
		{
			if(NormalControl.strategy_group == StrategyGroup[i]->group)
			{
				CurrentStrategy = StrategyGroup[i];												//指定策略组

				break;
			}
		}

		if((NormalControl.week_enable & 0x80) == 0x00)		//星期无效
		{
			ret = 1;
		}
		else												//星期有效
		{
			if((NormalControl.week_enable & (1 << calendar.week)) != 0x00)
			{
				ret = 1;
			}
		}
	}

	if(AppointmentControl != NULL && AppointmentControl->next != NULL)
	{
		for(tmp_time = AppointmentControl->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			gate_day_s = get_days_form_calendar(tmp_time->s_year + 2000,tmp_time->s_month,tmp_time->s_date);
			gate_minute_s = gate_day_s * 1440 + tmp_time->s_hour * 60 + tmp_time->s_minute;

			gate_day_e = get_days_form_calendar(tmp_time->e_year + 2000,tmp_time->e_month,tmp_time->e_date);
			gate_minute_e = gate_day_e * 1440 + tmp_time->e_hour * 60 + tmp_time->e_minute;

			gate_day_n = get_days_form_calendar(calendar.w_year,calendar.w_month,calendar.w_date);
			gate_minute_n = gate_day_n * 1440 + calendar.hour * 60 + calendar.min;

			if(gate_minute_s <= gate_minute_n && gate_minute_n <= gate_minute_e)					//在节日期间内
			{
				if(tmp_time->strategy_group >= 1 && tmp_time->strategy_group <= MAX_GROUP_NUM)		//组号合法
				{
					CurrentStrategy = StrategyGroup[tmp_time->strategy_group - 1];					//指定策略组

					if((tmp_time->week_enable & 0x80) == 0x00)		//星期无效
					{
						ret = 1;

						break;
					}
					else											//星期有效
					{
						if((tmp_time->week_enable & (1 << calendar.week)) != 0x00)
						{
							ret = 1;

							break;
						}
						else
						{
							ret = 0;

							break;
						}
					}
				}
			}
		}
	}

	return ret;
}

//刷新所有策略的日出日落时间
void RefreshStrategySunRiseSetTime(void)
{
	u8 i = 0;
	s16 minutes = 0;
	pStrategyTime tmp_time = NULL;
	pStrategyTime main_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	for(i = 0; i < MAX_GROUP_NUM; i ++)
	{
		main_time = StrategyGroup[i];

		if(main_time != NULL && main_time->next != NULL)
		{
			for(tmp_time = main_time->next; tmp_time != NULL; tmp_time = tmp_time->next)
			{
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
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}
}











































#ifndef __COMMON_H
#define __COMMON_H

#include "stm32f10x.h"
#include "string.h"
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "rtc.h"

/*---------------------------------------------------------------------------*/
/* Type Definition Macros                                                    */
/*---------------------------------------------------------------------------*/
#ifndef __WORDSIZE
  /* Assume 32 */
  #define __WORDSIZE 32
#endif

    typedef unsigned char   uint8;
    typedef char            int8;
    typedef unsigned short  uint16;
    typedef short           int16;
    typedef unsigned int    uint32;
    typedef int             int32;

#ifdef WIN32
    typedef int socklen_t;
#endif

    typedef unsigned long long int  uint64;
    typedef long long int           int64;


#define SOFT_WARE_VRESION			101			//软件版本号

#define DEBUG_LOG								//是否打印调试信息

#define DEBUG_LOG								//是否打印调试信息

#define MAX_FW_VER					9999
#define MAX_FW_BAG_NUM				896
#define MAX_FW_LAST_BAG_NUM			134


#define MAX_GROUP_NUM				50		//(256 - 11 - 6 - 36) / 11
#define HOLD_REG_LEN				512
#define TIME_BUF_LEN				256

#define MAX_UPLOAD_INVL				65500
#define MAX_REALY_ACTION_INVL		65500

#define TYPE_WEEKDAY				0x01
#define TYPE_WEEKEND				0x02
#define TYPE_HOLIDAY_START			0x04
#define TYPE_HOLIDAY_END			0x14

#define MODE_AUTO					0
#define MODE_MANUAL					1

#define APP_SW_VER_ADD				4			//应用软件版本存储地址
#define APP_SW_VER_LEN				4

#define HW_VER_ADD					8			//硬件版本存储地址
#define HW_VER_LEN					4

#define DEVICE_NAME_ADD				12			//设备名称存储地址
#define DEVICE_NAME_LEN				35

#define DEVICE_ID_ADD				47			//设备ID存储地址
#define DEVICE_ID_LEN				8

#define UU_ID_ADD					55			//UUID存储地址
#define UU_ID_LEN					19

#define AREA_ID_ADD					121			//逻辑区码存储地址
#define AREA_ID_LEN					3			//逻辑区长度

#define BOX_ID_ADD					124			//物理区码存储地址
#define BOX_ID_LEN					3			//物理区长度

#define UPLOAD_INVL_ADD				256			//数据上传周期存储地址
#define UPLOAD_INVL_LEN				4

#define REALY_ACTION_INVL_ADD		266			//继电器动作间隔时间存储地址
#define REALY_ACTION_INVL_LEN		4

#define RS485_BUAD_RATE_ADD			270			//继电器动作间隔时间存储地址
#define RS485_BUAD_RATE_LEN			6

#define OTA_INFO_ADD				301			//OTA信息存储地址
#define OTA_INFO_LEN				9

#define FIRM_WARE_FLAG_S_ADD		301			//新固件标识存储地址
#define FIRM_WARE_STORE_ADD_S_ADD	302			//新固件Flash地址存储地址
#define FIRM_WARE_VER_S_ADD			303			//新固件版本号存储地址
#define FIRM_WARE_BAG_NUM_S_ADD		305			//新固件总包数存储地址
#define LAST_BAG_BYTE_NUM_S_ADD		307			//新固件末包字节数存储地址

#define TIME_GROUP_NUM_ADD			361			//策略组数存储地址
#define TIME_GROUP_NUM_LEN			3

#define RELAY_STATE_ADD				364			//继电器状态存储地址
#define RELAY_STATE_LEN				4

#define TIME_RULE_ADD				512			//时间策略存储地址
#define TIME_RULE_LEN				12


#define RegularTime_S struct RegularTime
typedef struct RegularTime *pRegularTime;
struct RegularTime
{
	u8 number;
	u8 type;			//策略类别Bit0:1 工作日 Bit1:1 周末 Bit2:1节日

	u8 year;
	u8 month;
	u8 date;
	u8 hour;
	u8 minute;

	u16 control_bit;	//位指定字节
	u16 control_state;	//状态指定字节

	pRegularTime prev;
	pRegularTime next;
};

#define HolodayRange_S struct HolodayRange
typedef struct HolodayRange *pHolodayRange;
struct HolodayRange
{
	u8 year_s;
	u8 month_s;
	u8 date_s;
	
	u8 year_e;
	u8 month_e;
	u8 date_e;
};

static const uint32_t crc32tab[] =
{
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
	0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
	0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
	0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
	0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
	0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
	0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
	0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
	0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
	0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
	0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
	0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
	0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
	0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
	0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
	0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
	0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
	0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
	0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
	0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
	0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
	0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
	0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
	0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
	0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
	0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
	0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
	0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
	0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
	0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
	0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
	0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
	0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
	0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
	0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
	0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
	0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
	0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
	0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
	0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
	0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
	0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
	0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
	0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
	0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
	0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
	0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
	0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
	0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
	0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
	0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
	0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

static u8 auchCRCHi[] =
{
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40
    } ;
    /* CRC低位字节值表*/
static u8 auchCRCLo[] =
{
    0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
    0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
    0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
    0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
    0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
    0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
    0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
    0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
    0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
    0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
    0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
    0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
    0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
    0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
    0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
    0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
    0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
    0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
    0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
    0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
    0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
    0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
    0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
    0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
    0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
    0x43,0x83,0x41,0x81,0x80,0x40
};

extern SemaphoreHandle_t  xMutex_IIC1;			//IIC1的互斥量
extern SemaphoreHandle_t  xMutex_STRATEGY;		//AT指令的互斥量

extern QueueHandle_t xQueue_key;				//用于按键时间的消息队列


extern u8 HoldReg[HOLD_REG_LEN];
extern u8 RegularTimeGroups[TIME_BUF_LEN];
extern u8 TimeGroupNumber;
extern pRegularTime RegularTimeWeekDay;			//工作日策略
extern pRegularTime RegularTimeWeekEnd;			//周末策略
extern pRegularTime RegularTimeHoliday;			//节假日策略
extern HolodayRange_S HolodayRange;				//节假日起始日期

/***************************固件升级相关*****************************/
extern u8 NeedUpDateFirmWare;			//有新固件需要加载
extern u8 HaveNewFirmWare;				//0xAA有新固件 others无新固件
extern u8 NewFirmWareAdd;				//0xAA新固件地址0x0800C000 0x55新固件地址0x08026000
extern u16 NewFirmWareBagNum;			//固件包的数量（一个固件包含多个小包）
extern u16 NewFirmWareVer;				//固件包的版本
extern u8 LastBagByteNum;				//最后一包的字节数

/***************************系统心跳相关*****************************/
extern u32 SysTick1ms;					//1ms滴答时钟
extern u32 SysTick10ms;					//10ms滴答时钟
extern u32 SysTick100ms;				//10ms滴答时钟
extern time_t SysTick1s;				//1s滴答时钟


/***********************MCU厂商唯一序列号*****************************/
extern u8 *UniqueChipID;

/***************************版本相关*********************************/
extern u8 *BootLoaderVersion;			//BootLoader版本号
extern u8 *SoftWareVersion;				//应用程序版本号
extern u8 *HardWareVersion;				//硬件版本号

/***************************设备相关*********************************/
extern u8 *DeviceName;					//设备名称
extern u8 *DeviceID;					//设备ID
extern u8 *DeviceUUID;					//设备UUID
extern u8 DeviceAreaID;					//设备逻辑区码
extern u8 DeviceBoxID;					//设备物理区码

/***************************运行参数相关*****************************/
extern u16 UpLoadINCL;					//数据上传时间间隔0~65535秒
extern u8 GetTimeOK;					//成功获取时间标志
extern u8 DeviceWorkMode;				//运行模式，0：自动，1：手动
extern u16 RelayActionINCL;				//数据上传时间间隔0~65535毫秒
extern u32 RS485BuadRate;				//通讯波特率

extern u8 NeedToReset;					//复位/重启标志

/*|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|*/
/*|                                                        OutPutControlBit每个bit对应一个继电器的开闭状态                                                                |*/
/*|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|*/
/*|    bit0     |    bit1     |    bit2     |    bit3     |    bit4     |    bit5     |    bit6     |    bit7     |    bit8     |    bit9     |    bit10    |    bit11    |*/
/*|-------------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|*/
/*|  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |  1   |  0   |*/
/*|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|*/
/*| R1合 | R1分 | R2合 | R2分 | R3合 | R3分 | R4合 | R4分 | R5合 | R5分 | R6合 | R6分 | R7合 | R7分 | R8合 | R8分 | R9合 | R9分 | R10合| R10分| R11合| R11分| R12合| R21分|*/
/*|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|*/
extern u16 OutPutControlBit;			//开出位标志
extern u16 OutPutControlState;			//开出位标志(具体哪几位)
extern u16 RelaysState;					//各个继电器的状态
extern u16 AllRelayPowerState;			//继电器输入端是否带电
extern u16 AllRelayState;				//继电器的状态
extern u8 HaveNewActionCommand;			//有新的动作指令


u16 MyStrstr(u8 *str1, u8 *str2, u16 str1_len, u16 str2_len);
u8 GetDatBit(u32 dat);
u32 GetADV(u8 len);
void IntToString(u8 *DString,u32 Dint,u8 zero_num);
u32 StringToInt(u8 *String);
unsigned short find_str(unsigned char *s_str, unsigned char *p_str, unsigned short count, unsigned short *seek);
int search_str(unsigned char *source, unsigned char *target);
unsigned short get_str1(unsigned char *source, unsigned char *begin, unsigned short count1, unsigned char *end, unsigned short count2, unsigned char *out);
unsigned short get_str2(unsigned char *source, unsigned char *begin, unsigned short count, unsigned short length, unsigned char *out);
unsigned short get_str3(unsigned char *source, unsigned char *out, unsigned short length);
u32 CRC32( const u8 *buf, u32 size);
u16 CRC16(u8 *puchMsgg,u8 usDataLen);
u8 CalCheckSum(u8 *buf, u16 len);

void SysTick1msAdder(void);
u32 GetSysTick1ms(void);
void SysTick10msAdder(void);
u32 GetSysTick10ms(void);
void SysTick100msAdder(void);
u32 GetSysTick100ms(void);
void SetSysTick1s(time_t sec);
time_t GetSysTick1s(void);


u8 ReadDataFromEepromToHoldBuf(u8 *inbuf,u16 s_add, u16 len);
void WriteDataFromHoldBufToEeprom(u8 *inbuf,u16 s_add, u16 len);
u8 GetMemoryForString(u8 **str, u8 type, u32 id, u16 add, u16 size, u8 *hold_reg);
u8 CopyStrToPointer(u8 **pointer, u8 *str, u8 len);

u8 GetDeviceName(void);
u8 GetDeviceID(void);
u8 GetDeviceUUID(void);
u8 ReadDeviceAreaID(void);
u8 ReadDeviceBoxID(void);
u8 ReadUpLoadINVL(void);
u8 ReadRelayActionINCL(void);
u8 ReadRS485BuadRate(void);


u8 ReadSoftWareVersion(void);
u8 ReadHardWareVersion(void);
u8 ReadDeviceName(void);
u8 ReadDeviceID(void);
u8 ReadDeviceUUID(void);
u8 ReadTimeGroupNumber(void);
u8 ReadAllRelayState(void);
u8 WriteAllRelayState(void);
void WriteOTAInfo(u8 *hold_reg,u8 reset);
u8 ReadOTAInfo(u8 *hold_reg);
u8 ReadRegularTimeGroups(void);
void ReadParametersFromEEPROM(void);

u16 PackDataOfRelayInfo(u8 *outbuf);
u16 PackNetData(u8 fun_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf,u8 id_type);

u8 RegularTimeGroupAdd(u8 type,pRegularTime group_time);
u8 RegularTimeGroupSub(u8 number);
void RemoveAllStrategy(void);






























#endif

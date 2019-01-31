#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
  
 

#define KEY1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)//读取按键0

#define KEY_NULL	0
#define KEY1_PRES	1		//KEY0  


void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode);  	//按键扫描函数					    
#endif

#ifndef __TPIC6C595_H
#define __TPIC6C595_H

#include "sys.h"


#define HC595_G_L		GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define HC595_G_H		GPIO_SetBits(GPIOA,GPIO_Pin_4)

#define HC595_CLR_L		GPIO_ResetBits(GPIOA,GPIO_Pin_6)
#define HC595_CLR_H		GPIO_SetBits(GPIOA,GPIO_Pin_6)

#define HC595_LRCK_L	GPIO_ResetBits(GPIOA,GPIO_Pin_1)
#define HC595_LRCK_H	GPIO_SetBits(GPIOA,GPIO_Pin_1)

#define HC595_SRCK_L	GPIO_ResetBits(GPIOA,GPIO_Pin_5)
#define HC595_SRCK_H	GPIO_SetBits(GPIOA,GPIO_Pin_5)

#define HC595_SDATA_L	GPIO_ResetBits(GPIOA,GPIO_Pin_7)
#define HC595_SDATA_H	GPIO_SetBits(GPIOA,GPIO_Pin_7)



void TPIC6C595_Init(void);
void TPIC6C595WriteOneByte(u32 byte);
void TPIC6C595OutPut(void);









































#endif

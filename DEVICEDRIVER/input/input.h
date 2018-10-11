#ifndef __INPUT_H
#define __INPUT_H


#include "sys.h"


#define INPUT1   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)
#define INPUT2   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)
#define INPUT3   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)
#define INPUT4   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)
#define INPUT5   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)
#define INPUT6   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)
#define INPUT7   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)
#define INPUT8   GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6)
#define INPUT9   GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)
#define INPUT10  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)
#define INPUT11  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)
#define INPUT12  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)



void INPUT_Init(void);
u16 GetAllInputState(void);
u16 GetAllOutPutState(void);


































#endif

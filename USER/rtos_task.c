#include "rtos_task.h"
#include "common.h"
#include "task_led.h"
#include "task_hci.h"
#include "task_sensor.h"
#include "task_main.h"
#include "task_key.h"

/*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************/
void AppObjCreate(void)
{
	//创建互斥量
	xMutex_IIC1 = xSemaphoreCreateMutex();
	if(xMutex_IIC1 == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
	
	xMutex_STRATEGY = xSemaphoreCreateMutex();
	if(xMutex_STRATEGY == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
	
	//创建消息队列
	xQueue_key = xQueueCreate(5, sizeof(u8));
    if( xQueue_key == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
	
	//创建消息队列
	xQueue_RelayState = xQueueCreate(20, sizeof(u32));
    if( xQueue_RelayState == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

/*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************/
void AppTaskCreate(void)
{
	xTaskCreate(vTaskLED,
				"vTaskLED",
				64,
				NULL,
				3,
				&xHandleTaskLED);		//指示灯任务
	
	xTaskCreate(vTaskSENSOR,
				"vTaskSENSOR",
				128,
				NULL,
				4,
				&xHandleTaskSENSOR);	//传感器采集任务
	
	xTaskCreate(vTaskKEY,
				"vTaskKEY",
				64,
				NULL,
				5,
				&xHandleTaskKEY);		//人机交互任务
	
	xTaskCreate(vTaskHCI,
				"vTaskHCI",
				1024,
				NULL,
				6,
				&xHandleTaskHCI);		//人机交互任务
	
	xTaskCreate(vTaskMAIN,
				"vTaskMAIN",
				512,
				NULL,
				7,
				&xHandleTaskMAIN);		//主任务
}


























































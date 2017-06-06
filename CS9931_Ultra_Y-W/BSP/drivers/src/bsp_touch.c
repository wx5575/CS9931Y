/*
*********************************************************************************************************
*
*	模块名称 : 电阻式触摸板驱动模块
*	文件名称 : bsp_touch.c
*	版    本 : V1.4
*	说    明 : 驱动TS2046芯片 和 RA8875内置触摸
*	修改记录 :
*		版本号  日期        作者    说明
*       v1.0    2012-07-06 armfly  ST固件库V3.5.0版本。
*		v1.1    2012-10-22 armfly  增加4点校准
*		v1.2    2012-11-07 armfly  解决4点校准的XY交换分支的bug
*		v1.3    2012-12-17 armfly  触摸校准函数增加入口参数:等待时间
*		V1.4    2013-07-26 armfly  更改 TOUCH_DataFilter() 滤波算法
*		V1.5    2013-07-32 armfly  修改TOUCH_WaitRelease(),计数器需要清零
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f4xx.h"
#include <stdio.h>

#include "bsp_touch.h"
#include "LCD_RA8875.h"

#include <stdbool.h>
#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>
#include "mouse.h"

	
struct rtgui_touch_device
{
    struct rt_device parent;

    rt_timer_t poll_timer;
    rt_uint16_t x, y;

    rt_bool_t calibrating;
    rt_touch_calibration_func_t calibration_func;

    rt_uint16_t min_x, max_x;
    rt_uint16_t min_y, max_y;
};
static struct rtgui_touch_device *touch = RT_NULL;

rt_inline void EXTI_Enable(rt_uint32_t enable);



#define X_WIDTH 800
#define Y_WIDTH 480

static void rtgui_touch_calculate()
{
	u32 t;
	
    if (touch != RT_NULL)
    {
        unsigned int touch_hw_tmp_x[10];
        unsigned int touch_hw_tmp_y[10];
        unsigned int i;

        for(i=0; i<10; i++)
        {
			if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_3) != 0)
			{
				break;
		
			}
			touch_hw_tmp_x[i] = RA8875_TouchReadX();
			touch_hw_tmp_y[i] = RA8875_TouchReadY();
			for(t=0;t<16800;t++);
        }
		if(i!=10)return;

        {
            unsigned int temp_x = 0;
            unsigned int temp_y = 0;
            unsigned int max_x = 0;
            unsigned int min_x = 0xffff;
            unsigned int max_y = 0;
            unsigned int min_y = 0xffff;
			
			/* 去掉一个最大值，去掉一个最小值，取平均值。 */
            for(i=0; i<10; i++)
            {
                temp_x += touch_hw_tmp_x[i];
                temp_y += touch_hw_tmp_y[i];
                if(touch_hw_tmp_x[i] > max_x) max_x = touch_hw_tmp_x[i];
                if(touch_hw_tmp_x[i] < min_x) min_x = touch_hw_tmp_x[i];
                if(touch_hw_tmp_y[i] > max_y) max_y = touch_hw_tmp_y[i];
                if(touch_hw_tmp_y[i] < min_y) min_y = touch_hw_tmp_y[i];
            }
            touch->x = (temp_x-max_x-min_x) / 8;
            touch->y = (temp_y-max_y-min_y) / 8;
        }

        /* if it's not in calibration status  */
        if (touch->calibrating != RT_TRUE)	// 对AD值做简单处理，转换结果为像素值。
        {
            if (touch->max_x > touch->min_x)
            {
                touch->x = (touch->x - touch->min_x) * X_WIDTH/(touch->max_x - touch->min_x);
            }
            else
            {
                touch->x = (touch->min_x - touch->x) * X_WIDTH/(touch->min_x - touch->max_x);
            }

            if (touch->max_y > touch->min_y)
            {
                touch->y = (touch->y - touch->min_y) * Y_WIDTH /(touch->max_y - touch->min_y);
            }
            else
            {
                touch->y = (touch->min_y - touch->y) * Y_WIDTH /(touch->min_y - touch->max_y);
            }
        }
    }
}

static unsigned int flag = 0;
void touch_timeout(void* parameter)
{
    struct rtgui_event_mouse emouse;

    if (GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_3) != 0)
    {
        EXTI_Enable(1);
			
        emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
        emouse.button = (RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_UP);

        /* use old value */
        emouse.x = touch->x;
        emouse.y = touch->y;

        /* stop timer */
        rt_timer_stop(touch->poll_timer);
#ifdef	RT_TOUCH_DEBUG
        rt_kprintf("touch up: (%d, %d)\n", emouse.x, emouse.y);
#endif
        flag = 0;

        if ((touch->calibrating == RT_TRUE) && (touch->calibration_func != RT_NULL))
        {
            /* callback function */
            touch->calibration_func(emouse.x, emouse.y);
#ifdef	RT_TOUCH_DEBUG
			rt_kprintf("calibration call.\n");
#endif
        }
    }

    else
    {
        if(flag == 0)
        {
            // calculation 
            rtgui_touch_calculate();

            // send mouse event 
            emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
            emouse.parent.sender = RT_NULL;

            emouse.x = touch->x;
            emouse.y = touch->y;

            // init mouse button 
            emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_DOWN);
#ifdef	RT_TOUCH_DEBUG
            rt_kprintf("touch down: (%d, %d)\n", emouse.x, emouse.y);
#endif
            flag = 1;
        }
        else
        {
            // send mouse event 
            emouse.parent.type = RTGUI_EVENT_MOUSE_MOTION;
            emouse.parent.sender = RT_NULL;

            // calculation 
            rtgui_touch_calculate();

            emouse.x = touch->x;
            emouse.y = touch->y;

            // init mouse button 
            emouse.button = 0;
#ifdef	RT_TOUCH_DEBUG
            rt_kprintf("touch motion: (%d, %d)\n", emouse.x, emouse.y);
#endif
	

        }
    }

    /* send event to server */
   	if (touch->calibrating != RT_TRUE)
	{
        rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));
		//rt_kprintf("TOUCH.\n"); 
	}
	else rt_kprintf("error when send event to server.\n") ;
}
static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

rt_inline void EXTI_Enable(rt_uint32_t enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Configure  EXTI  */
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升

    if (enable)
    {
        /* enable */
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }
    else
    {
        /* disable */
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    }

    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line3);
}

static void EXTI_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	/* 使能SYSCFG时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
	
	
	/* 配置 PC7 为浮空输入模式，用于触笔中断 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
	/* 连接 EXTI Line13 到 PC8 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOI, EXTI_PinSource3);

    /* Configure  EXTI  */
    EXTI_Enable(1);
}
/* RT-Thread Device Interface */
static rt_err_t rtgui_touch_init (rt_device_t dev)
{


    NVIC_Configuration();
    EXTI_Configuration();

	RA8875_TouchInit();
	
	
    return RT_EOK;
}
static rt_err_t rtgui_touch_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case RT_TOUCH_CALIBRATION:
        touch->calibrating = RT_TRUE;
        touch->calibration_func = (rt_touch_calibration_func_t)args;
        break;

    case RT_TOUCH_NORMAL:
        touch->calibrating = RT_FALSE;
        break;

    case RT_TOUCH_CALIBRATION_DATA:
    {
        struct calibration_data* data;

        data = (struct calibration_data*) args;

        //update
        touch->min_x = data->min_x;
        touch->max_x = data->max_x;
        touch->min_y = data->min_y;
        touch->max_y = data->max_y;

        //save setup
        
    }
    break;
    }

    return RT_EOK;
}

void EXTI3_IRQHandler(void)
{
    /* disable interrupt */
    EXTI_Enable(0);
	/* start timer */
	rt_timer_start(touch->poll_timer);
	EXTI_ClearITPendingBit(EXTI_Line3);
	rt_timer_start(touch->poll_timer);
}

void rtgui_touch_hw_init(void)
{

    touch = (struct rtgui_touch_device*)rt_malloc (sizeof(struct rtgui_touch_device));
    if (touch == RT_NULL) return; /* no memory yet */

    /* clear device structure */
    rt_memset(&(touch->parent), 0, sizeof(struct rt_device));
    touch->calibrating = false;
    touch->min_x = 0x3dc;
    touch->max_x = 0x21;
    touch->min_y = 0x398;
    touch->max_y = 0x4b;

    /* init device structure */
    touch->parent.type = RT_Device_Class_Unknown;
    touch->parent.init = rtgui_touch_init;
    touch->parent.control = rtgui_touch_control;
    touch->parent.user_data = RT_NULL;

    /* create 1/8 second timer */
    touch->poll_timer = rt_timer_create("touch", touch_timeout, RT_NULL,
                                        RT_TICK_PER_SECOND/50, RT_TIMER_FLAG_PERIODIC);

    /* register touch device to RT-Thread */
    rt_device_register(&(touch->parent), "touch", RT_DEVICE_FLAG_RDWR);
	rt_device_open(&(touch->parent), RT_DEVICE_FLAG_RDWR);
	if (touch->poll_timer != RT_NULL)	rt_timer_start(touch->poll_timer);
	else  rt_kprintf("touch create error.\n");

}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

/*
 * Copyright(c) 2013,
 * All rights reserved
 * 文件名称：buzzer.c
 * 摘  要  ：主程序
 * 当前版本：V1.0，孙世成编写
 * 修改记录：
 * V1.0, 2014.07.26, 此版本配合赵工设计CS99xxZ(7寸屏)系列主板设计，目前仅适用CS99xxZ(7寸屏)系列
 *
 */

#include "buzzer.h"

#define		BUZZER_SETB()			(GPIO_SetBits(GPIOG,GPIO_Pin_6))
#define		BUZZER_CLR()			(GPIO_ResetBits(GPIOG,GPIO_Pin_6))

/*
 * 函数名：buzzer_GPIO_Config
 * 描述  ：引脚配置
 * 输入  ：无
 * 输出  ：无
 */
static void buzzer_gpio_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
			
	/* 第1步：打开GPIOA GPIOC GPIOD GPIOF GPIOG的时钟
	   注意：这个地方可以一次性全打开
	*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}


/*
 * 函数名：buzzer_init
 * 描述  ：buzzer初始化
 * 输入  ：无
 * 输出  ：无
 */
void buzzer_init(void)
{
	buzzer_gpio_config();
	
}

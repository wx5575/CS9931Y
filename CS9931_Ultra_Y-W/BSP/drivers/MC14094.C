/*
 * Copyright(c) 2013,
 * All rights reserved
 * 文件名称：MC14094.c
 * 摘  要  ：主程序
 * 当前版本：V2.0，孙世成编写
 * 修改记录：
 * V1.0, 2013.09.05, 此版本配合赵工设计CS88xx(简易型)及CS99xx系列主板设计，目前仅适用CS88xx(简易型)及CS99xx驱动系列
 * V2.0, 2014.07.26, 此版本在CS88xx(简易型)及CS99xx驱动基础上配合赵工设计CS99xxZ(7寸屏)系列主板修改，目前仅适用CS99xxZ(7寸屏)系列
 *
 */

#include "MC14094.h"



static void MC14094K_Write(void);
static void MC14094_Write(void);


#define MC14094_SETCLK()				(GPIO_SetBits(GPIOH,GPIO_Pin_4))
#define MC14094_CLRCLK()				(GPIO_ResetBits(GPIOH,GPIO_Pin_4))
#define MC14094_SETDATA()				(GPIO_SetBits(GPIOF,GPIO_Pin_10))
#define MC14094_CLRDATA()				(GPIO_ResetBits(GPIOF,GPIO_Pin_10))
#define MC14094_SETST()					(GPIO_SetBits(GPIOA,GPIO_Pin_0))
#define MC14094_CLRST()					(GPIO_ResetBits(GPIOA,GPIO_Pin_0))

#define MC14094K_SETCLK()				(GPIO_SetBits(GPIOG,GPIO_Pin_12))
#define MC14094K_CLRCLK()				(GPIO_ResetBits(GPIOG,GPIO_Pin_12))
#define MC14094K_SETDATA()				(GPIO_SetBits(GPIOG,GPIO_Pin_11))
#define MC14094K_CLRDATA()				(GPIO_ResetBits(GPIOG,GPIO_Pin_11))
#define MC14094K_SETST()				(GPIO_SetBits(GPIOG,GPIO_Pin_10))
#define MC14094K_CLRST()				(GPIO_ResetBits(GPIOG,GPIO_Pin_10))

#define MC14094_Dly					10
u8		MC14094_BUFFERK	= 0x00;
u16		MC14094_BUFFER	= 0x0000;

/*
 * 函数名：MC14094_GPIO_Config
 * 描述  ：引脚配置
 * 输入  ：无
 * 输出  ：无
 */
static void MC14094_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
			
	/* 第1步：打开GPIOA GPIOC GPIOD GPIOF GPIOG的时钟
	   注意：这个地方可以一次性全打开
	*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOH, ENABLE);
	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	
}

static void MC14094_Delay(u32 t)
{
	while(t--);
}

/*
 * 函数名：MC14094_Init
 * 描述  ：MC14094初始化
 * 输入  ：无
 * 输出  ：无
 */
void MC14094_Init(void)
{
	MC14094_GPIO_Config();
	
	MC14094_BUFFERK = 0xa5;
	MC14094_BUFFER = 0xa5a5;
	MC14094K_Write();
	MC14094_Write();
}
	
/*
 * 函数名：MC14094K_Write
 * 描述  ：写
 * 输入  ：无
 * 输出  ：无
 */
static void MC14094K_Write(void)
{
	unsigned char i,pos=0x80;
	MC14094K_CLRST();
	for(i=0;i<8;i++)
	{	
		MC14094K_CLRCLK();
		if(MC14094_BUFFERK & pos)
		{
			MC14094K_SETDATA();
		}
		else
		{
			MC14094K_CLRDATA();
		}
		MC14094_Delay(MC14094_Dly);
		MC14094K_SETCLK();		
		MC14094_Delay(MC14094_Dly);
		pos=pos>>1;
	}
	MC14094K_SETST();
}

/*
 * 函数名：MC14094_Write
 * 描述  ：写
 * 输入  ：无
 * 输出  ：无
 */
static void MC14094_Write(void)
{
	unsigned char i;
	unsigned int pos=0x8000;
	MC14094_CLRST();
	for(i=0;i<16;i++)
	{	
		MC14094_CLRCLK();
		if(MC14094_BUFFER & pos)
		{
			MC14094_SETDATA();
		}
		else
		{
			MC14094_CLRDATA();
		}
		MC14094_Delay(MC14094_Dly);
		MC14094_SETCLK();		
		MC14094_Delay(MC14094_Dly);
		pos=pos>>1;
	}
	MC14094_SETST();
}

/*
 * 函数名：MC14094_CMD
 * 描述  ：Write Command
 * 输入  ：None
 * 输出  ：None
 */
void MC14094_CMD(u8 id,u8 bits,u8 status)
{
	switch(id)
	{
		case MC14094:
			status != 0? (MC14094_BUFFER |= (0x0001<<bits)) : (MC14094_BUFFER &= ~(0x0001<<bits));
		break;
		case MC14094K:
			status != 0? (MC14094_BUFFERK |= (0x01<<bits)) : (MC14094_BUFFERK &= ~(0x01<<bits));
		break;
	}
}

/*
 * 函数名：MC14094_UPDATA
 * 描述  ：Write Updata
 * 输入  ：None
 * 输出  ：None
 */
void MC14094_Updata(u8 id)
{
	switch(id)
	{
		case MC14094:
			MC14094_Write();
		break;
		case MC14094K:
			MC14094K_Write();
		break;
	}
}

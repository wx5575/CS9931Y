/*
*********************************************************************************************************
*	                                  
*	模块名称 : 按键驱动模块    
*	文件名称 : bsp_button.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

/* 按键滤波时间50ms, 单位10ms
 只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
*/
#define BUTTON_FILTER_TIME 	1
#define BUTTON_LONG_TIME 	  100		/* 持续1秒，认为长按事件 */

#define	BUTTON_BACKCOLOR		0xffff

extern struct rt_mailbox key_mb;


/*
	每个按键对应1个全局的结构体变量。
	其成员变量是实现滤波和多种按键状态所必须的
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint32_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t Count;			/* 滤波器计数器 */
	uint8_t FilterTime;		/* 滤波时间(最大255,表示2550ms) */
	uint16_t LongCount;		/* 长按计数器 */
	uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
	uint8_t  State;			/* 按键当前状态（按下还是弹起） */
	uint8_t KeyCodeUp;		/* 按键弹起的键值代码, 0表示不检测按键弹起 */
	uint8_t KeyCodeDown;	/* 按键按下的键值代码, 0表示不检测按键按下 */
	uint8_t KeyCodeLong;	/* 按键长按的键值代码, 0表示不检测长按 */
	uint8_t RepeatSpeed;	/* 连续按键周期 */
	uint8_t RepeatCount;	/* 连续按键计数器 */
}BUTTON_T;

/* 定义键值代码
	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2)	编译器可帮我们避免键值重复。
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 表示按键事件 */

	KEY_DOWN = 0x20,
	KEY_UP = 0x40,
	KEY_LONG = 0x80,
	
	KEY_DISPLAY = 1,			/* SET键按下 */
	
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	
	KEY_LOCK,
	KEY_OFFSET,
	KEY_ENTER,
	KEY_EXIT,
	
	KEY_NUM0,
	KEY_NUM1,
	KEY_NUM2,
	KEY_NUM3,
	KEY_NUM4,
	KEY_NUM5,
	KEY_NUM6,
	KEY_NUM7,
	KEY_NUM8,
	KEY_NUM9,
	
	KEY_POS,
	KEY_SHIFT,
	
	
	KEY_L,
	KEY_U,
	KEY_D,
	KEY_R,
	
	CODE_LEFT,
	CODE_RIGHT,
}KEY_ENUM;

enum{
	LED_FAIL=0x100,
	LED_PASS=0x200,
	FMQ=0x400,
	LED_KEY1=0x800,
	LED_KEY2=0x1000,
	
	LED_TEST=0x10000,
};
/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	20
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t Read;					/* 缓冲区读指针 */
	uint8_t Write;					/* 缓冲区写指针 */
}KEY_FIFO_T;


/* 供外部调用的函数声明 */
void bsp_InitButton(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
void bsp_KeyPro(void);
// void CODE_LEFT_IRQ(void);
// void CODE_RIGHT_IRQ(void);
// void SoftSendEXITKey(void);
// void bsp_KeyMode(u8 key, u8 m);
void ui_key_updata(uint8_t key_disable);
void buzzer(u8 timer);

void bsp_display(u32 cmd,u8 status);

#endif


